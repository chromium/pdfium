// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_font.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/cfx_fontcache.h"
#include "core/fxge/cfx_fontmapper.h"
#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/cfx_glyphcache.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/fx_font.h"
#include "core/fxge/scoped_font_transform.h"
#include "third_party/base/check.h"
#include "third_party/base/cxx17_backports.h"
#include "third_party/base/numerics/safe_conversions.h"
#include "third_party/base/span.h"

#define EM_ADJUST(em, a) (em == 0 ? (a) : (a)*1000 / em)

namespace {

constexpr int kThousandthMinInt = std::numeric_limits<int>::min() / 1000;
constexpr int kThousandthMaxInt = std::numeric_limits<int>::max() / 1000;

struct OUTLINE_PARAMS {
  UnownedPtr<CFX_Path> m_pPath;
  FT_Pos m_CurX;
  FT_Pos m_CurY;
  float m_CoordUnit;
};

FX_RECT FXRectFromFTPos(FT_Pos left, FT_Pos top, FT_Pos right, FT_Pos bottom) {
  return FX_RECT(pdfium::base::checked_cast<int32_t>(left),
                 pdfium::base::checked_cast<int32_t>(top),
                 pdfium::base::checked_cast<int32_t>(right),
                 pdfium::base::checked_cast<int32_t>(bottom));
}

FX_RECT ScaledFXRectFromFTPos(FT_Pos left,
                              FT_Pos top,
                              FT_Pos right,
                              FT_Pos bottom,
                              int x_scale,
                              int y_scale) {
  if (x_scale == 0 || y_scale == 0)
    return FXRectFromFTPos(left, top, right, bottom);

  return FXRectFromFTPos(left * 1000 / x_scale, top * 1000 / y_scale,
                         right * 1000 / x_scale, bottom * 1000 / y_scale);
}

#ifdef PDF_ENABLE_XFA
unsigned long FTStreamRead(FXFT_StreamRec* stream,
                           unsigned long offset,
                           unsigned char* buffer,
                           unsigned long count) {
  if (count == 0)
    return 0;

  IFX_SeekableReadStream* pFile =
      static_cast<IFX_SeekableReadStream*>(stream->descriptor.pointer);
  return pFile && pFile->ReadBlockAtOffset(buffer, offset, count) ? count : 0;
}

void FTStreamClose(FXFT_StreamRec* stream) {}
#endif  // PDF_ENABLE_XFA

void Outline_CheckEmptyContour(OUTLINE_PARAMS* param) {
  size_t size;
  {
    pdfium::span<const CFX_Path::Point> points = param->m_pPath->GetPoints();
    size = points.size();

    if (size >= 2 &&
        points[size - 2].IsTypeAndOpen(CFX_Path::Point::Type::kMove) &&
        points[size - 2].m_Point == points[size - 1].m_Point) {
      size -= 2;
    }
    if (size >= 4 &&
        points[size - 4].IsTypeAndOpen(CFX_Path::Point::Type::kMove) &&
        points[size - 3].IsTypeAndOpen(CFX_Path::Point::Type::kBezier) &&
        points[size - 3].m_Point == points[size - 4].m_Point &&
        points[size - 2].m_Point == points[size - 4].m_Point &&
        points[size - 1].m_Point == points[size - 4].m_Point) {
      size -= 4;
    }
  }
  // Only safe after |points| has been destroyed.
  param->m_pPath->GetPoints().resize(size);
}

int Outline_MoveTo(const FT_Vector* to, void* user) {
  OUTLINE_PARAMS* param = static_cast<OUTLINE_PARAMS*>(user);

  Outline_CheckEmptyContour(param);

  param->m_pPath->ClosePath();
  param->m_pPath->AppendPoint(
      CFX_PointF(to->x / param->m_CoordUnit, to->y / param->m_CoordUnit),
      CFX_Path::Point::Type::kMove);

  param->m_CurX = to->x;
  param->m_CurY = to->y;
  return 0;
}

int Outline_LineTo(const FT_Vector* to, void* user) {
  OUTLINE_PARAMS* param = static_cast<OUTLINE_PARAMS*>(user);

  param->m_pPath->AppendPoint(
      CFX_PointF(to->x / param->m_CoordUnit, to->y / param->m_CoordUnit),
      CFX_Path::Point::Type::kLine);

  param->m_CurX = to->x;
  param->m_CurY = to->y;
  return 0;
}

int Outline_ConicTo(const FT_Vector* control, const FT_Vector* to, void* user) {
  OUTLINE_PARAMS* param = static_cast<OUTLINE_PARAMS*>(user);

  param->m_pPath->AppendPoint(
      CFX_PointF((param->m_CurX + (control->x - param->m_CurX) * 2 / 3) /
                     param->m_CoordUnit,
                 (param->m_CurY + (control->y - param->m_CurY) * 2 / 3) /
                     param->m_CoordUnit),
      CFX_Path::Point::Type::kBezier);

  param->m_pPath->AppendPoint(
      CFX_PointF((control->x + (to->x - control->x) / 3) / param->m_CoordUnit,
                 (control->y + (to->y - control->y) / 3) / param->m_CoordUnit),
      CFX_Path::Point::Type::kBezier);

  param->m_pPath->AppendPoint(
      CFX_PointF(to->x / param->m_CoordUnit, to->y / param->m_CoordUnit),
      CFX_Path::Point::Type::kBezier);

  param->m_CurX = to->x;
  param->m_CurY = to->y;
  return 0;
}

int Outline_CubicTo(const FT_Vector* control1,
                    const FT_Vector* control2,
                    const FT_Vector* to,
                    void* user) {
  OUTLINE_PARAMS* param = static_cast<OUTLINE_PARAMS*>(user);

  param->m_pPath->AppendPoint(CFX_PointF(control1->x / param->m_CoordUnit,
                                         control1->y / param->m_CoordUnit),
                              CFX_Path::Point::Type::kBezier);

  param->m_pPath->AppendPoint(CFX_PointF(control2->x / param->m_CoordUnit,
                                         control2->y / param->m_CoordUnit),
                              CFX_Path::Point::Type::kBezier);

  param->m_pPath->AppendPoint(
      CFX_PointF(to->x / param->m_CoordUnit, to->y / param->m_CoordUnit),
      CFX_Path::Point::Type::kBezier);

  param->m_CurX = to->x;
  param->m_CurY = to->y;
  return 0;
}

bool ShouldAppendStyle(const ByteString& style) {
  return !style.IsEmpty() && style != "Regular";
}

constexpr int8_t kAngleSkew[] = {
    -0,  -2,  -3,  -5,  -7,  -9,  -11, -12, -14, -16, -18, -19, -21, -23, -25,
    -27, -29, -31, -32, -34, -36, -38, -40, -42, -45, -47, -49, -51, -53, -55,
};

constexpr uint8_t kWeightPow[] = {
    0,   6,   12,  14,  16,  18,  22,  24,  28,  30,  32,  34,  36,  38,  40,
    42,  44,  46,  48,  50,  52,  54,  56,  58,  60,  62,  64,  66,  68,  70,
    70,  72,  72,  74,  74,  74,  76,  76,  76,  78,  78,  78,  80,  80,  80,
    82,  82,  82,  84,  84,  84,  84,  86,  86,  86,  88,  88,  88,  88,  90,
    90,  90,  90,  92,  92,  92,  92,  94,  94,  94,  94,  96,  96,  96,  96,
    96,  98,  98,  98,  98,  100, 100, 100, 100, 100, 102, 102, 102, 102, 102,
    104, 104, 104, 104, 104, 106, 106, 106, 106, 106,
};

constexpr uint8_t kWeightPow11[] = {
    0,  4,  7,  8,  9,  10, 12, 13, 15, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 39, 39, 40, 40, 41,
    41, 41, 42, 42, 42, 43, 43, 43, 44, 44, 44, 45, 45, 45, 46, 46, 46,
    46, 43, 47, 47, 48, 48, 48, 48, 45, 50, 50, 50, 46, 51, 51, 51, 52,
    52, 52, 52, 53, 53, 53, 53, 53, 54, 54, 54, 54, 55, 55, 55, 55, 55,
    56, 56, 56, 56, 56, 57, 57, 57, 57, 57, 58, 58, 58, 58, 58,
};

constexpr uint8_t kWeightPowShiftJis[] = {
    0,   0,   2,   4,   6,   8,   10,  14,  16,  20,  22,  26,  28,  32,  34,
    38,  42,  44,  48,  52,  56,  60,  64,  66,  70,  74,  78,  82,  86,  90,
    96,  96,  96,  96,  98,  98,  98,  100, 100, 100, 100, 102, 102, 102, 102,
    104, 104, 104, 104, 104, 106, 106, 106, 106, 106, 108, 108, 108, 108, 108,
    110, 110, 110, 110, 110, 112, 112, 112, 112, 112, 112, 114, 114, 114, 114,
    114, 114, 114, 116, 116, 116, 116, 116, 116, 116, 118, 118, 118, 118, 118,
    118, 118, 120, 120, 120, 120, 120, 120, 120, 120,
};

constexpr size_t kWeightPowArraySize = 100;
static_assert(kWeightPowArraySize == pdfium::size(kWeightPow), "Wrong size");
static_assert(kWeightPowArraySize == pdfium::size(kWeightPow11), "Wrong size");
static_assert(kWeightPowArraySize == pdfium::size(kWeightPowShiftJis),
              "Wrong size");

}  // namespace

const CFX_Font::CharsetFontMap CFX_Font::kDefaultTTFMap[] = {
    {static_cast<int>(FX_Charset::kANSI), kDefaultAnsiFontName},
    {static_cast<int>(FX_Charset::kChineseSimplified), "SimSun"},
    {static_cast<int>(FX_Charset::kChineseTraditional), "MingLiU"},
    {static_cast<int>(FX_Charset::kShiftJIS), "MS Gothic"},
    {static_cast<int>(FX_Charset::kHangul), "Batang"},
    {static_cast<int>(FX_Charset::kMSWin_Cyrillic), "Arial"},
#if BUILDFLAG(IS_WIN)
    {static_cast<int>(FX_Charset::kMSWin_EasternEuropean), "Tahoma"},
#else
    {static_cast<int>(FX_Charset::kMSWin_EasternEuropean), "Arial"},
#endif
    {static_cast<int>(FX_Charset::kMSWin_Arabic), "Arial"},
    {-1, nullptr}};

// static
const char CFX_Font::kUntitledFontName[] = "Untitled";

// static
const char CFX_Font::kDefaultAnsiFontName[] = "Helvetica";

// static
const char CFX_Font::kUniversalDefaultFontName[] = "Arial Unicode MS";

// static
ByteString CFX_Font::GetDefaultFontNameByCharset(FX_Charset nCharset) {
  for (size_t i = 0; i < pdfium::size(kDefaultTTFMap) - 1; ++i) {
    if (static_cast<int>(nCharset) == kDefaultTTFMap[i].charset)
      return kDefaultTTFMap[i].fontname;
  }
  return kUniversalDefaultFontName;
}

// static
FX_Charset CFX_Font::GetCharSetFromUnicode(uint16_t word) {
  // to avoid CJK Font to show ASCII
  if (word < 0x7F)
    return FX_Charset::kANSI;

  // find new charset
  if ((word >= 0x4E00 && word <= 0x9FA5) ||
      (word >= 0xE7C7 && word <= 0xE7F3) ||
      (word >= 0x3000 && word <= 0x303F) ||
      (word >= 0x2000 && word <= 0x206F)) {
    return FX_Charset::kChineseSimplified;
  }

  if (((word >= 0x3040) && (word <= 0x309F)) ||
      ((word >= 0x30A0) && (word <= 0x30FF)) ||
      ((word >= 0x31F0) && (word <= 0x31FF)) ||
      ((word >= 0xFF00) && (word <= 0xFFEF))) {
    return FX_Charset::kShiftJIS;
  }

  if (((word >= 0xAC00) && (word <= 0xD7AF)) ||
      ((word >= 0x1100) && (word <= 0x11FF)) ||
      ((word >= 0x3130) && (word <= 0x318F))) {
    return FX_Charset::kHangul;
  }

  if (word >= 0x0E00 && word <= 0x0E7F)
    return FX_Charset::kThai;

  if ((word >= 0x0370 && word <= 0x03FF) || (word >= 0x1F00 && word <= 0x1FFF))
    return FX_Charset::kMSWin_Greek;

  if ((word >= 0x0600 && word <= 0x06FF) || (word >= 0xFB50 && word <= 0xFEFC))
    return FX_Charset::kMSWin_Arabic;

  if (word >= 0x0590 && word <= 0x05FF)
    return FX_Charset::kMSWin_Hebrew;

  if (word >= 0x0400 && word <= 0x04FF)
    return FX_Charset::kMSWin_Cyrillic;

  if (word >= 0x0100 && word <= 0x024F)
    return FX_Charset::kMSWin_EasternEuropean;

  if (word >= 0x1E00 && word <= 0x1EFF)
    return FX_Charset::kMSWin_Vietnamese;

  return FX_Charset::kANSI;
}

CFX_Font::CFX_Font() = default;

int CFX_Font::GetSubstFontItalicAngle() const {
  CFX_SubstFont* subst_font = GetSubstFont();
  return subst_font ? subst_font->m_ItalicAngle : 0;
}

#ifdef PDF_ENABLE_XFA
bool CFX_Font::LoadFile(RetainPtr<IFX_SeekableReadStream> pFile,
                        int nFaceIndex) {
  m_bEmbedded = false;
  m_ObjectTag = 0;

  auto pStreamRec = std::make_unique<FXFT_StreamRec>();
  pStreamRec->base = nullptr;
  pStreamRec->size = static_cast<unsigned long>(pFile->GetSize());
  pStreamRec->pos = 0;
  pStreamRec->descriptor.pointer = static_cast<void*>(pFile.Get());
  pStreamRec->close = FTStreamClose;
  pStreamRec->read = FTStreamRead;

  FT_Open_Args args;
  args.flags = FT_OPEN_STREAM;
  args.stream = pStreamRec.get();

  m_Face = CFX_Face::Open(CFX_GEModule::Get()->GetFontMgr()->GetFTLibrary(),
                          &args, nFaceIndex);
  if (!m_Face)
    return false;

  m_pOwnedFile = std::move(pFile);
  m_pOwnedStreamRec = std::move(pStreamRec);
  FT_Set_Pixel_Sizes(m_Face->GetRec(), 0, 64);
  return true;
}

#if !BUILDFLAG(IS_WIN)
void CFX_Font::SetFace(RetainPtr<CFX_Face> face) {
  ClearGlyphCache();
  m_ObjectTag = 0;
  m_Face = face;
}

void CFX_Font::SetSubstFont(std::unique_ptr<CFX_SubstFont> subst) {
  m_pSubstFont = std::move(subst);
}
#endif  // !BUILDFLAG(IS_WIN)
#endif  // PDF_ENABLE_XFA

CFX_Font::~CFX_Font() {
  m_FontData = {};  // m_FontData can't outive m_Face.
  m_Face.Reset();

#if BUILDFLAG(IS_APPLE)
  ReleasePlatformResource();
#endif
}

void CFX_Font::LoadSubst(const ByteString& face_name,
                         bool bTrueType,
                         uint32_t flags,
                         int weight,
                         int italic_angle,
                         FX_CodePage code_page,
                         bool bVertical) {
  m_bEmbedded = false;
  m_bVertical = bVertical;
  m_ObjectTag = 0;
  m_pSubstFont = std::make_unique<CFX_SubstFont>();
  m_Face = CFX_GEModule::Get()->GetFontMgr()->GetBuiltinMapper()->FindSubstFont(
      face_name, bTrueType, flags, weight, italic_angle, code_page,
      m_pSubstFont.get());
  if (m_Face) {
    m_FontData = {FXFT_Get_Face_Stream_Base(m_Face->GetRec()),
                  FXFT_Get_Face_Stream_Size(m_Face->GetRec())};
  }
}

int CFX_Font::GetGlyphWidth(uint32_t glyph_index) {
  if (!m_Face)
    return 0;
  if (m_pSubstFont && m_pSubstFont->m_bFlagMM)
    AdjustMMParams(glyph_index, 0, 0);
  int err =
      FT_Load_Glyph(m_Face->GetRec(), glyph_index,
                    FT_LOAD_NO_SCALE | FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
  if (err)
    return 0;

  FT_Pos horiAdvance = FXFT_Get_Glyph_HoriAdvance(m_Face->GetRec());
  if (horiAdvance < kThousandthMinInt || horiAdvance > kThousandthMaxInt)
    return 0;

  return static_cast<int>(
      EM_ADJUST(FXFT_Get_Face_UnitsPerEM(m_Face->GetRec()), horiAdvance));
}

bool CFX_Font::LoadEmbedded(pdfium::span<const uint8_t> src_span,
                            bool force_vertical,
                            uint64_t object_tag) {
  m_bVertical = force_vertical;
  m_ObjectTag = object_tag;
  m_FontDataAllocation = std::vector<uint8_t, FxAllocAllocator<uint8_t>>(
      src_span.begin(), src_span.end());
  m_Face = CFX_GEModule::Get()->GetFontMgr()->NewFixedFace(
      nullptr, m_FontDataAllocation, 0);
  m_bEmbedded = true;
  m_FontData = m_FontDataAllocation;
  return !!m_Face;
}

bool CFX_Font::IsTTFont() const {
  return m_Face && FXFT_Is_Face_TT_OT(m_Face->GetRec()) == FT_FACE_FLAG_SFNT;
}

int CFX_Font::GetAscent() const {
  if (!m_Face)
    return 0;

  int ascender = FXFT_Get_Face_Ascender(m_Face->GetRec());
  if (ascender < kThousandthMinInt || ascender > kThousandthMaxInt)
    return 0;

  return EM_ADJUST(FXFT_Get_Face_UnitsPerEM(m_Face->GetRec()), ascender);
}

int CFX_Font::GetDescent() const {
  if (!m_Face)
    return 0;

  int descender = FXFT_Get_Face_Descender(m_Face->GetRec());
  if (descender < kThousandthMinInt || descender > kThousandthMaxInt)
    return 0;

  return EM_ADJUST(FXFT_Get_Face_UnitsPerEM(m_Face->GetRec()), descender);
}

absl::optional<FX_RECT> CFX_Font::GetGlyphBBox(uint32_t glyph_index) {
  if (!m_Face)
    return absl::nullopt;

  if (FXFT_Is_Face_Tricky(m_Face->GetRec())) {
    int error = FT_Set_Char_Size(m_Face->GetRec(), 0, 1000 * 64, 72, 72);
    if (error)
      return absl::nullopt;

    error = FT_Load_Glyph(m_Face->GetRec(), glyph_index,
                          FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
    if (error)
      return absl::nullopt;

    FT_Glyph glyph;
    error = FT_Get_Glyph(m_Face->GetRec()->glyph, &glyph);
    if (error)
      return absl::nullopt;

    FT_BBox cbox;
    FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_PIXELS, &cbox);
    int pixel_size_x = m_Face->GetRec()->size->metrics.x_ppem;
    int pixel_size_y = m_Face->GetRec()->size->metrics.y_ppem;
    FX_RECT result = ScaledFXRectFromFTPos(
        cbox.xMin, cbox.yMax, cbox.xMax, cbox.yMin, pixel_size_x, pixel_size_y);
    result.top =
        std::min(result.top, pdfium::base::checked_cast<int32_t>(
                                 FXFT_Get_Face_Ascender(m_Face->GetRec())));
    result.bottom =
        std::max(result.bottom, pdfium::base::checked_cast<int32_t>(
                                    FXFT_Get_Face_Descender(m_Face->GetRec())));
    FT_Done_Glyph(glyph);
    if (FT_Set_Pixel_Sizes(m_Face->GetRec(), 0, 64) != 0)
      return absl::nullopt;
    return result;
  }
  constexpr int kFlag = FT_LOAD_NO_SCALE | FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH;
  if (FT_Load_Glyph(m_Face->GetRec(), glyph_index, kFlag) != 0)
    return absl::nullopt;
  int em = FXFT_Get_Face_UnitsPerEM(m_Face->GetRec());
  return ScaledFXRectFromFTPos(FXFT_Get_Glyph_HoriBearingX(m_Face->GetRec()),
                               FXFT_Get_Glyph_HoriBearingY(m_Face->GetRec()) -
                                   FXFT_Get_Glyph_Height(m_Face->GetRec()),
                               FXFT_Get_Glyph_HoriBearingX(m_Face->GetRec()) +
                                   FXFT_Get_Glyph_Width(m_Face->GetRec()),
                               FXFT_Get_Glyph_HoriBearingY(m_Face->GetRec()),
                               em, em);
}

bool CFX_Font::IsItalic() const {
  if (!m_Face)
    return false;
  if (FXFT_Is_Face_Italic(m_Face->GetRec()) == FT_STYLE_FLAG_ITALIC)
    return true;

  ByteString str(FXFT_Get_Face_Style_Name(m_Face->GetRec()));
  str.MakeLower();
  return str.Contains("italic");
}

bool CFX_Font::IsBold() const {
  return m_Face && FXFT_Is_Face_Bold(m_Face->GetRec()) == FT_STYLE_FLAG_BOLD;
}

bool CFX_Font::IsFixedWidth() const {
  return m_Face && FXFT_Is_Face_fixedwidth(m_Face->GetRec()) != 0;
}

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
bool CFX_Font::IsSubstFontBold() const {
  CFX_SubstFont* subst_font = GetSubstFont();
  return subst_font && subst_font->GetOriginalWeight() >= FXFONT_FW_BOLD;
}
#endif

ByteString CFX_Font::GetPsName() const {
  if (!m_Face)
    return ByteString();

  ByteString psName = FT_Get_Postscript_Name(m_Face->GetRec());
  if (psName.IsEmpty())
    psName = kUntitledFontName;
  return psName;
}

ByteString CFX_Font::GetFamilyName() const {
  if (!m_Face && !m_pSubstFont)
    return ByteString();
  if (m_Face)
    return ByteString(FXFT_Get_Face_Family_Name(m_Face->GetRec()));
  return m_pSubstFont->m_Family;
}

ByteString CFX_Font::GetFamilyNameOrUntitled() const {
  ByteString facename = GetFamilyName();
  return facename.IsEmpty() ? kUntitledFontName : facename;
}

ByteString CFX_Font::GetFaceName() const {
  if (!m_Face && !m_pSubstFont)
    return ByteString();
  if (m_Face) {
    ByteString style = ByteString(FXFT_Get_Face_Style_Name(m_Face->GetRec()));
    ByteString facename = GetFamilyNameOrUntitled();
    if (ShouldAppendStyle(style))
      facename += " " + style;
    return facename;
  }
  return m_pSubstFont->m_Family;
}

ByteString CFX_Font::GetBaseFontName() const {
  ByteString psname = GetPsName();
  if (!psname.IsEmpty() && psname != kUntitledFontName)
    return psname;
  if (m_Face) {
    ByteString style = ByteString(FXFT_Get_Face_Style_Name(m_Face->GetRec()));
    ByteString facename = GetFamilyNameOrUntitled();
    if (IsTTFont())
      facename.Remove(' ');
    if (ShouldAppendStyle(style))
      facename += (IsTTFont() ? "," : " ") + style;
    return facename;
  }
  if (m_pSubstFont)
    return m_pSubstFont->m_Family;
  return ByteString();
}

absl::optional<FX_RECT> CFX_Font::GetRawBBox() const {
  if (!m_Face)
    return absl::nullopt;

  return FXRectFromFTPos(FXFT_Get_Face_xMin(m_Face->GetRec()),
                         FXFT_Get_Face_yMin(m_Face->GetRec()),
                         FXFT_Get_Face_xMax(m_Face->GetRec()),
                         FXFT_Get_Face_yMax(m_Face->GetRec()));
}

absl::optional<FX_RECT> CFX_Font::GetBBox() const {
  absl::optional<FX_RECT> result = GetRawBBox();
  if (!result.has_value())
    return result;

  int em = FXFT_Get_Face_UnitsPerEM(m_Face->GetRec());
  if (em != 0) {
    FX_RECT& bbox = result.value();
    bbox.left = (bbox.left * 1000) / em;
    bbox.top = (bbox.top * 1000) / em;
    bbox.right = (bbox.right * 1000) / em;
    bbox.bottom = (bbox.bottom * 1000) / em;
  }
  return result;
}

RetainPtr<CFX_GlyphCache> CFX_Font::GetOrCreateGlyphCache() const {
  if (!m_GlyphCache)
    m_GlyphCache = CFX_GEModule::Get()->GetFontCache()->GetGlyphCache(this);
  return m_GlyphCache;
}

void CFX_Font::ClearGlyphCache() {
  m_GlyphCache = nullptr;
}

void CFX_Font::AdjustMMParams(int glyph_index,
                              int dest_width,
                              int weight) const {
  DCHECK(dest_width >= 0);
  FXFT_MM_VarPtr pMasters = nullptr;
  FT_Get_MM_Var(m_Face->GetRec(), &pMasters);
  if (!pMasters)
    return;

  FT_Pos coords[2];
  if (weight == 0)
    coords[0] = FXFT_Get_MM_Axis_Def(FXFT_Get_MM_Axis(pMasters, 0)) / 65536;
  else
    coords[0] = weight;

  if (dest_width == 0) {
    coords[1] = FXFT_Get_MM_Axis_Def(FXFT_Get_MM_Axis(pMasters, 1)) / 65536;
  } else {
    FT_Long min_param =
        FXFT_Get_MM_Axis_Min(FXFT_Get_MM_Axis(pMasters, 1)) / 65536;
    FT_Long max_param =
        FXFT_Get_MM_Axis_Max(FXFT_Get_MM_Axis(pMasters, 1)) / 65536;
    coords[1] = min_param;
    FT_Set_MM_Design_Coordinates(m_Face->GetRec(), 2, coords);
    FT_Load_Glyph(m_Face->GetRec(), glyph_index,
                  FT_LOAD_NO_SCALE | FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
    FT_Pos min_width = FXFT_Get_Glyph_HoriAdvance(m_Face->GetRec()) * 1000 /
                       FXFT_Get_Face_UnitsPerEM(m_Face->GetRec());
    coords[1] = max_param;
    FT_Set_MM_Design_Coordinates(m_Face->GetRec(), 2, coords);
    FT_Load_Glyph(m_Face->GetRec(), glyph_index,
                  FT_LOAD_NO_SCALE | FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
    FT_Pos max_width = FXFT_Get_Glyph_HoriAdvance(m_Face->GetRec()) * 1000 /
                       FXFT_Get_Face_UnitsPerEM(m_Face->GetRec());
    if (max_width == min_width) {
      FXFT_Free(m_Face->GetRec(), pMasters);
      return;
    }
    FT_Pos param = min_param + (max_param - min_param) *
                                   (dest_width - min_width) /
                                   (max_width - min_width);
    coords[1] = param;
  }
  FXFT_Free(m_Face->GetRec(), pMasters);
  FT_Set_MM_Design_Coordinates(m_Face->GetRec(), 2, coords);
}

std::unique_ptr<CFX_Path> CFX_Font::LoadGlyphPathImpl(uint32_t glyph_index,
                                                      int dest_width) const {
  if (!m_Face)
    return nullptr;

  FT_Set_Pixel_Sizes(m_Face->GetRec(), 0, 64);
  FT_Matrix ft_matrix = {65536, 0, 0, 65536};
  if (m_pSubstFont) {
    if (m_pSubstFont->m_ItalicAngle) {
      int skew = GetSkewFromAngle(m_pSubstFont->m_ItalicAngle);
      if (m_bVertical)
        ft_matrix.yx += ft_matrix.yy * skew / 100;
      else
        ft_matrix.xy -= ft_matrix.xx * skew / 100;
    }
    if (m_pSubstFont->m_bFlagMM)
      AdjustMMParams(glyph_index, dest_width, m_pSubstFont->m_Weight);
  }
  ScopedFontTransform scoped_transform(m_Face, &ft_matrix);
  int load_flags = FT_LOAD_NO_BITMAP;
  if (!(m_Face->GetRec()->face_flags & FT_FACE_FLAG_SFNT) ||
      !FT_IS_TRICKY(m_Face->GetRec()))
    load_flags |= FT_LOAD_NO_HINTING;
  if (FT_Load_Glyph(m_Face->GetRec(), glyph_index, load_flags))
    return nullptr;
  if (m_pSubstFont && !m_pSubstFont->m_bFlagMM &&
      m_pSubstFont->m_Weight > 400) {
    uint32_t index = std::min<uint32_t>((m_pSubstFont->m_Weight - 400) / 10,
                                        kWeightPowArraySize - 1);
    int level;
    if (m_pSubstFont->m_Charset == FX_Charset::kShiftJIS)
      level = kWeightPowShiftJis[index] * 65536 / 36655;
    else
      level = kWeightPow[index];
    FT_Outline_Embolden(FXFT_Get_Glyph_Outline(m_Face->GetRec()), level);
  }

  FT_Outline_Funcs funcs;
  funcs.move_to = Outline_MoveTo;
  funcs.line_to = Outline_LineTo;
  funcs.conic_to = Outline_ConicTo;
  funcs.cubic_to = Outline_CubicTo;
  funcs.shift = 0;
  funcs.delta = 0;

  auto pPath = std::make_unique<CFX_Path>();
  OUTLINE_PARAMS params;
  params.m_pPath = pPath.get();
  params.m_CurX = params.m_CurY = 0;
  params.m_CoordUnit = 64 * 64.0;

  FT_Outline_Decompose(FXFT_Get_Glyph_Outline(m_Face->GetRec()), &funcs,
                       &params);
  if (pPath->GetPoints().empty())
    return nullptr;

  Outline_CheckEmptyContour(&params);
  pPath->ClosePath();
  return pPath;
}

const CFX_GlyphBitmap* CFX_Font::LoadGlyphBitmap(
    uint32_t glyph_index,
    bool bFontStyle,
    const CFX_Matrix& matrix,
    int dest_width,
    int anti_alias,
    CFX_TextRenderOptions* text_options) const {
  return GetOrCreateGlyphCache()->LoadGlyphBitmap(this, glyph_index, bFontStyle,
                                                  matrix, dest_width,
                                                  anti_alias, text_options);
}

const CFX_Path* CFX_Font::LoadGlyphPath(uint32_t glyph_index,
                                        int dest_width) const {
  return GetOrCreateGlyphCache()->LoadGlyphPath(this, glyph_index, dest_width);
}

// static
int CFX_Font::GetWeightLevel(FX_Charset charset, size_t index) {
  if (index >= kWeightPowArraySize)
    return -1;

  if (charset == FX_Charset::kShiftJIS)
    return kWeightPowShiftJis[index];
  return kWeightPow11[index];
}

// static
int CFX_Font::GetSkewFromAngle(int angle) {
  // |angle| is non-positive so |-angle| is used as the index. Need to make sure
  // |angle| != INT_MIN since -INT_MIN is undefined.
  if (angle > 0 || angle == std::numeric_limits<int>::min() ||
      static_cast<size_t>(-angle) >= pdfium::size(kAngleSkew)) {
    return -58;
  }
  return kAngleSkew[-angle];
}

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
CFX_TypeFace* CFX_Font::GetDeviceCache() const {
  return GetOrCreateGlyphCache()->GetDeviceCache(this);
}
#endif
