// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_font.h"

#include <stdint.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <utility>

#include "build/build_config.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/span.h"
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

namespace {

const CFX_Font::CharsetFontMap kDefaultTTFMap[] = {
    {static_cast<int>(FX_Charset::kANSI), CFX_Font::kDefaultAnsiFontName},
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
    // TODO(crbug.com/348468114): Remove sentinel value when
    // FPDF_GetDefaultTTFMap() gets removed.
    {-1, nullptr}};

FX_RECT FXRectFromFTPos(FT_Pos left, FT_Pos top, FT_Pos right, FT_Pos bottom) {
  return FX_RECT(pdfium::checked_cast<int32_t>(left),
                 pdfium::checked_cast<int32_t>(top),
                 pdfium::checked_cast<int32_t>(right),
                 pdfium::checked_cast<int32_t>(bottom));
}

FX_RECT ScaledFXRectFromFTPos(FT_Pos left,
                              FT_Pos top,
                              FT_Pos right,
                              FT_Pos bottom,
                              int x_scale,
                              int y_scale) {
  if (x_scale == 0 || y_scale == 0) {
    return FXRectFromFTPos(left, top, right, bottom);
  }

  return FXRectFromFTPos(left * 1000 / x_scale, top * 1000 / y_scale,
                         right * 1000 / x_scale, bottom * 1000 / y_scale);
}

#ifdef PDF_ENABLE_XFA
unsigned long FTStreamRead(FXFT_StreamRec* stream,
                           unsigned long offset,
                           unsigned char* buffer,
                           unsigned long count) {
  if (count == 0) {
    return 0;
  }

  IFX_SeekableReadStream* pFile =
      static_cast<IFX_SeekableReadStream*>(stream->descriptor.pointer);

  // SAFETY: caller ensures `buffer` points to at least `count` bytes.
  return pFile && pFile->ReadBlockAtOffset(
                      UNSAFE_BUFFERS(pdfium::span(buffer, count)), offset)
             ? count
             : 0;
}

void FTStreamClose(FXFT_StreamRec* stream) {}
#endif  // PDF_ENABLE_XFA

bool ShouldAppendStyle(const ByteString& style) {
  return !style.IsEmpty() && style != "Regular";
}

}  // namespace

// static
const char CFX_Font::kUntitledFontName[] = "Untitled";

// static
const char CFX_Font::kDefaultAnsiFontName[] = "Helvetica";

// static
const char CFX_Font::kUniversalDefaultFontName[] = "Arial Unicode MS";

// static
pdfium::span<const CFX_Font::CharsetFontMap> CFX_Font::GetDefaultTTFMapSpan() {
  auto map_with_sentinel_value = pdfium::span(kDefaultTTFMap);
  return map_with_sentinel_value.first(map_with_sentinel_value.size() - 1);
}

// static
ByteString CFX_Font::GetDefaultFontNameByCharset(FX_Charset nCharset) {
  for (const auto& entry : GetDefaultTTFMapSpan()) {
    if (static_cast<int>(nCharset) == entry.charset) {
      return entry.fontname;
    }
  }
  return kUniversalDefaultFontName;
}

// static
FX_Charset CFX_Font::GetCharSetFromUnicode(uint16_t word) {
  // to avoid CJK Font to show ASCII
  if (word < 0x7F) {
    return FX_Charset::kANSI;
  }

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

  if (word >= 0x0E00 && word <= 0x0E7F) {
    return FX_Charset::kThai;
  }

  if ((word >= 0x0370 && word <= 0x03FF) ||
      (word >= 0x1F00 && word <= 0x1FFF)) {
    return FX_Charset::kMSWin_Greek;
  }

  if ((word >= 0x0600 && word <= 0x06FF) ||
      (word >= 0xFB50 && word <= 0xFEFC)) {
    return FX_Charset::kMSWin_Arabic;
  }

  if (word >= 0x0590 && word <= 0x05FF) {
    return FX_Charset::kMSWin_Hebrew;
  }

  if (word >= 0x0400 && word <= 0x04FF) {
    return FX_Charset::kMSWin_Cyrillic;
  }

  if (word >= 0x0100 && word <= 0x024F) {
    return FX_Charset::kMSWin_EasternEuropean;
  }

  if (word >= 0x1E00 && word <= 0x1EFF) {
    return FX_Charset::kMSWin_Vietnamese;
  }

  return FX_Charset::kANSI;
}

CFX_Font::CFX_Font() = default;

int CFX_Font::GetSubstFontItalicAngle() const {
  CFX_SubstFont* subst_font = GetSubstFont();
  return subst_font ? subst_font->italic_angle_ : 0;
}

#ifdef PDF_ENABLE_XFA
bool CFX_Font::LoadFile(RetainPtr<IFX_SeekableReadStream> pFile,
                        int nFaceIndex) {
  object_tag_ = 0;

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

  face_ = CFX_Face::Open(CFX_GEModule::Get()->GetFontMgr()->GetFTLibrary(),
                         &args, nFaceIndex);
  if (!face_) {
    return false;
  }

  owned_file_ = std::move(pFile);
  owned_stream_rec_ = std::move(pStreamRec);
  face_->SetPixelSize(0, 64);
  return true;
}

#if !BUILDFLAG(IS_WIN)
void CFX_Font::SetFace(RetainPtr<CFX_Face> face) {
  ClearGlyphCache();
  object_tag_ = 0;
  face_ = face;
}

void CFX_Font::SetSubstFont(std::unique_ptr<CFX_SubstFont> subst) {
  subst_font_ = std::move(subst);
}
#endif  // !BUILDFLAG(IS_WIN)
#endif  // PDF_ENABLE_XFA

CFX_Font::~CFX_Font() {
  font_data_ = {};  // font_data_ can't outive face_.
  face_.Reset();

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
  vertical_ = bVertical;
  object_tag_ = 0;
  subst_font_ = std::make_unique<CFX_SubstFont>();
  face_ = CFX_GEModule::Get()->GetFontMgr()->GetBuiltinMapper()->FindSubstFont(
      face_name, bTrueType, flags, weight, italic_angle, code_page,
      subst_font_.get());
  if (face_) {
    font_data_ = face_->GetData();
  }
}

int CFX_Font::GetGlyphWidth(uint32_t glyph_index) const {
  return GetGlyphWidth(glyph_index, 0, 0);
}

int CFX_Font::GetGlyphWidth(uint32_t glyph_index,
                            int dest_width,
                            int weight) const {
  return GetOrCreateGlyphCache()->GetGlyphWidth(this, glyph_index, dest_width,
                                                weight);
}

int CFX_Font::GetGlyphWidthImpl(uint32_t glyph_index,
                                int dest_width,
                                int weight) const {
  if (!face_) {
    return 0;
  }

  return face_->GetGlyphWidth(glyph_index, dest_width, weight,
                              subst_font_.get());
}

bool CFX_Font::LoadEmbedded(pdfium::span<const uint8_t> src_span,
                            bool force_vertical,
                            uint64_t object_tag) {
  vertical_ = force_vertical;
  object_tag_ = object_tag;
  font_data_allocation_ = DataVector<uint8_t>(src_span.begin(), src_span.end());
  face_ = CFX_GEModule::Get()->GetFontMgr()->NewFixedFace(
      nullptr, font_data_allocation_, 0);
  font_data_ = font_data_allocation_;
  return !!face_;
}

bool CFX_Font::IsTTFont() const {
  return face_ && face_->IsTtOt();
}

int CFX_Font::GetAscent() const {
  if (!face_) {
    return 0;
  }
  return NormalizeFontMetric(face_->GetAscender(), face_->GetUnitsPerEm());
}

int CFX_Font::GetDescent() const {
  if (!face_) {
    return 0;
  }
  return NormalizeFontMetric(face_->GetDescender(), face_->GetUnitsPerEm());
}

std::optional<FX_RECT> CFX_Font::GetGlyphBBox(uint32_t glyph_index) {
  if (!face_) {
    return std::nullopt;
  }

  if (face_->IsTricky()) {
    int error = FT_Set_Char_Size(face_->GetRec(), 0, 1000 * 64, 72, 72);
    if (error) {
      return std::nullopt;
    }

    error = FT_Load_Glyph(face_->GetRec(), glyph_index,
                          FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
    if (error) {
      return std::nullopt;
    }

    FT_Glyph glyph;
    error = FT_Get_Glyph(face_->GetRec()->glyph, &glyph);
    if (error) {
      return std::nullopt;
    }

    FT_BBox cbox;
    FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_PIXELS, &cbox);
    int pixel_size_x = face_->GetRec()->size->metrics.x_ppem;
    int pixel_size_y = face_->GetRec()->size->metrics.y_ppem;
    FX_RECT result = ScaledFXRectFromFTPos(
        cbox.xMin, cbox.yMax, cbox.xMax, cbox.yMin, pixel_size_x, pixel_size_y);
    result.top = std::min(result.top, static_cast<int>(face_->GetAscender()));
    result.bottom =
        std::max(result.bottom, static_cast<int>(face_->GetDescender()));
    FT_Done_Glyph(glyph);
    if (!face_->SetPixelSize(0, 64)) {
      return std::nullopt;
    }
    return result;
  }
  static constexpr int kFlag =
      FT_LOAD_NO_SCALE | FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH;
  if (FT_Load_Glyph(face_->GetRec(), glyph_index, kFlag) != 0) {
    return std::nullopt;
  }
  int em = face_->GetUnitsPerEm();
  return ScaledFXRectFromFTPos(FXFT_Get_Glyph_HoriBearingX(face_->GetRec()),
                               FXFT_Get_Glyph_HoriBearingY(face_->GetRec()) -
                                   FXFT_Get_Glyph_Height(face_->GetRec()),
                               FXFT_Get_Glyph_HoriBearingX(face_->GetRec()) +
                                   FXFT_Get_Glyph_Width(face_->GetRec()),
                               FXFT_Get_Glyph_HoriBearingY(face_->GetRec()), em,
                               em);
}

bool CFX_Font::IsItalic() const {
  if (!face_) {
    return false;
  }
  if (face_->IsItalic()) {
    return true;
  }

  ByteString str = face_->GetStyleName();
  str.MakeLower();
  return str.Contains("italic");
}

bool CFX_Font::IsBold() const {
  return face_ && face_->IsBold();
}

bool CFX_Font::IsFixedWidth() const {
  return face_ && face_->IsFixedWidth();
}

#if defined(PDF_USE_SKIA)
bool CFX_Font::IsSubstFontBold() const {
  CFX_SubstFont* subst_font = GetSubstFont();
  return subst_font &&
         subst_font->GetOriginalWeight() >= pdfium::kFontWeightBold;
}
#endif

ByteString CFX_Font::GetPsName() const {
  if (!face_) {
    return ByteString();
  }

  ByteString psName = FT_Get_Postscript_Name(face_->GetRec());
  if (psName.IsEmpty()) {
    psName = kUntitledFontName;
  }
  return psName;
}

ByteString CFX_Font::GetFamilyName() const {
  if (!face_ && !subst_font_) {
    return ByteString();
  }
  if (face_) {
    return face_->GetFamilyName();
  }
  return subst_font_->family_;
}

ByteString CFX_Font::GetFamilyNameOrUntitled() const {
  ByteString facename = GetFamilyName();
  return facename.IsEmpty() ? kUntitledFontName : facename;
}

ByteString CFX_Font::GetBaseFontName() const {
  ByteString psname = GetPsName();
  if (!psname.IsEmpty() && psname != kUntitledFontName) {
    return psname;
  }
  if (face_) {
    ByteString style = face_->GetStyleName();
    ByteString facename = GetFamilyNameOrUntitled();
    if (IsTTFont()) {
      facename.Remove(' ');
    }
    if (ShouldAppendStyle(style)) {
      facename += (IsTTFont() ? "," : " ") + style;
    }
    return facename;
  }
  if (subst_font_) {
    return subst_font_->family_;
  }
  return ByteString();
}

std::optional<FX_RECT> CFX_Font::GetRawBBox() const {
  if (!face_) {
    return std::nullopt;
  }
  return face_->GetBBox();
}

std::optional<FX_RECT> CFX_Font::GetBBox() const {
  std::optional<FX_RECT> result = GetRawBBox();
  if (!result.has_value()) {
    return result;
  }

  int em = face_->GetUnitsPerEm();
  if (em != 0) {
    FX_RECT& bbox = result.value();
    bbox.left = pdfium::saturated_cast<int32_t>((bbox.left * 1000.0f) / em);
    bbox.top = pdfium::saturated_cast<int32_t>((bbox.top * 1000.0f) / em);
    bbox.right = pdfium::saturated_cast<int32_t>((bbox.right * 1000.0f) / em);
    bbox.bottom = pdfium::saturated_cast<int32_t>((bbox.bottom * 1000.0f) / em);
  }
  return result;
}

RetainPtr<CFX_GlyphCache> CFX_Font::GetOrCreateGlyphCache() const {
  if (!glyph_cache_) {
    glyph_cache_ = CFX_GEModule::Get()->GetFontCache()->GetGlyphCache(this);
  }
  return glyph_cache_;
}

void CFX_Font::ClearGlyphCache() {
  glyph_cache_ = nullptr;
}

std::unique_ptr<CFX_Path> CFX_Font::LoadGlyphPathImpl(uint32_t glyph_index,
                                                      int dest_width) const {
  if (!face_) {
    return nullptr;
  }

  return face_->LoadGlyphPath(glyph_index, dest_width, vertical_,
                              subst_font_.get());
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

#if defined(PDF_USE_SKIA)
CFX_TypeFace* CFX_Font::GetDeviceCache() const {
  return GetOrCreateGlyphCache()->GetDeviceCache(this);
}
#endif
