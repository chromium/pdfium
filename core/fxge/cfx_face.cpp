// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_face.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/cfx_glyphbitmap.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"
#include "core/fxge/fx_fontencoding.h"
#include "core/fxge/scoped_font_transform.h"
#include "third_party/base/check.h"
#include "third_party/base/numerics/safe_conversions.h"
#include "third_party/base/numerics/safe_math.h"

namespace {

constexpr int kMaxGlyphDimension = 2048;

}  // namespace

// static
RetainPtr<CFX_Face> CFX_Face::New(FT_Library library,
                                  RetainPtr<Retainable> pDesc,
                                  pdfium::span<const FT_Byte> data,
                                  FT_Long face_index) {
  FXFT_FaceRec* pRec = nullptr;
  if (FT_New_Memory_Face(library, data.data(),
                         pdfium::base::checked_cast<FT_Long>(data.size()),
                         face_index, &pRec) != 0) {
    return nullptr;
  }
  // Private ctor.
  return pdfium::WrapRetain(new CFX_Face(pRec, std::move(pDesc)));
}

// static
RetainPtr<CFX_Face> CFX_Face::Open(FT_Library library,
                                   const FT_Open_Args* args,
                                   FT_Long face_index) {
  FXFT_FaceRec* pRec = nullptr;
  if (FT_Open_Face(library, args, face_index, &pRec) != 0)
    return nullptr;

  // Private ctor.
  return pdfium::WrapRetain(new CFX_Face(pRec, nullptr));
}

bool CFX_Face::HasGlyphNames() const {
  return !!(GetRec()->face_flags & FT_FACE_FLAG_GLYPH_NAMES);
}

bool CFX_Face::IsTtOt() const {
  return !!(GetRec()->face_flags & FT_FACE_FLAG_SFNT);
}

bool CFX_Face::IsTricky() const {
  return !!(GetRec()->face_flags & FT_FACE_FLAG_TRICKY);
}

bool CFX_Face::IsFixedWidth() const {
  return !!(GetRec()->face_flags & FT_FACE_FLAG_FIXED_WIDTH);
}

#if defined(PDF_ENABLE_XFA)
bool CFX_Face::IsScalable() const {
  return !!(GetRec()->face_flags & FT_FACE_FLAG_SCALABLE);
}

void CFX_Face::ClearExternalStream() {
  GetRec()->face_flags &= ~FT_FACE_FLAG_EXTERNAL_STREAM;
}
#endif

bool CFX_Face::IsItalic() const {
  return !!(GetRec()->style_flags & FT_STYLE_FLAG_ITALIC);
}

bool CFX_Face::IsBold() const {
  return !!(GetRec()->style_flags & FT_STYLE_FLAG_BOLD);
}

ByteString CFX_Face::GetFamilyName() const {
  return ByteString(GetRec()->family_name);
}

ByteString CFX_Face::GetStyleName() const {
  return ByteString(GetRec()->style_name);
}

FX_RECT CFX_Face::GetBBox() const {
  return FX_RECT(pdfium::base::checked_cast<int32_t>(GetRec()->bbox.xMin),
                 pdfium::base::checked_cast<int32_t>(GetRec()->bbox.yMin),
                 pdfium::base::checked_cast<int32_t>(GetRec()->bbox.xMax),
                 pdfium::base::checked_cast<int32_t>(GetRec()->bbox.yMax));
}

uint16_t CFX_Face::GetUnitsPerEm() const {
  return pdfium::base::checked_cast<uint16_t>(GetRec()->units_per_EM);
}

int16_t CFX_Face::GetAscender() const {
  return pdfium::base::checked_cast<int16_t>(GetRec()->ascender);
}

int16_t CFX_Face::GetDescender() const {
  return pdfium::base::checked_cast<int16_t>(GetRec()->descender);
}

#if BUILDFLAG(IS_ANDROID)
int16_t CFX_Face::GetHeight() const {
  return pdfium::base::checked_cast<int16_t>(GetRec()->height);
}
#endif

pdfium::span<uint8_t> CFX_Face::GetData() const {
  return {GetRec()->stream->base, GetRec()->stream->size};
}

std::unique_ptr<CFX_GlyphBitmap> CFX_Face::RenderGlyph(const CFX_Font* pFont,
                                                       uint32_t glyph_index,
                                                       bool bFontStyle,
                                                       const CFX_Matrix& matrix,
                                                       int dest_width,
                                                       int anti_alias) {
  FT_Matrix ft_matrix;
  ft_matrix.xx = matrix.a / 64 * 65536;
  ft_matrix.xy = matrix.c / 64 * 65536;
  ft_matrix.yx = matrix.b / 64 * 65536;
  ft_matrix.yy = matrix.d / 64 * 65536;
  bool bUseCJKSubFont = false;
  const CFX_SubstFont* pSubstFont = pFont->GetSubstFont();
  if (pSubstFont) {
    bUseCJKSubFont = pSubstFont->m_bSubstCJK && bFontStyle;
    int angle;
    if (bUseCJKSubFont) {
      angle = pSubstFont->m_bItalicCJK ? -15 : 0;
    } else {
      angle = pSubstFont->m_ItalicAngle;
    }
    if (angle) {
      int skew = CFX_Font::GetSkewFromAngle(angle);
      if (pFont->IsVertical()) {
        ft_matrix.yx += ft_matrix.yy * skew / 100;
      } else {
        ft_matrix.xy -= ft_matrix.xx * skew / 100;
      }
    }
    if (pSubstFont->IsBuiltInGenericFont()) {
      pFont->AdjustMMParams(glyph_index, dest_width,
                            pFont->GetSubstFont()->m_Weight);
    }
  }

  ScopedFontTransform scoped_transform(pdfium::WrapRetain(this), &ft_matrix);
  int load_flags = FT_LOAD_NO_BITMAP | FT_LOAD_PEDANTIC;
  if (!IsTtOt()) {
    load_flags |= FT_LOAD_NO_HINTING;
  }
  FXFT_FaceRec* rec = GetRec();
  int error = FT_Load_Glyph(rec, glyph_index, load_flags);
  if (error) {
    // if an error is returned, try to reload glyphs without hinting.
    if (load_flags & FT_LOAD_NO_HINTING) {
      return nullptr;
    }

    load_flags |= FT_LOAD_NO_HINTING;
    load_flags &= ~FT_LOAD_PEDANTIC;
    error = FT_Load_Glyph(rec, glyph_index, load_flags);
    if (error) {
      return nullptr;
    }
  }

  auto* glyph = rec->glyph;
  int weight;
  if (bUseCJKSubFont) {
    weight = pSubstFont->m_WeightCJK;
  } else {
    weight = pSubstFont ? pSubstFont->m_Weight : 0;
  }
  if (pSubstFont && !pSubstFont->IsBuiltInGenericFont() && weight > 400) {
    uint32_t index = (weight - 400) / 10;
    pdfium::base::CheckedNumeric<signed long> level =
        CFX_Font::GetWeightLevel(pSubstFont->m_Charset, index);
    if (level.ValueOrDefault(-1) < 0) {
      return nullptr;
    }

    level = level *
            (abs(static_cast<int>(ft_matrix.xx)) +
             abs(static_cast<int>(ft_matrix.xy))) /
            36655;
    FT_Outline_Embolden(&glyph->outline, level.ValueOrDefault(0));
  }
  FT_Library_SetLcdFilter(CFX_GEModule::Get()->GetFontMgr()->GetFTLibrary(),
                          FT_LCD_FILTER_DEFAULT);
  error = FT_Render_Glyph(glyph, static_cast<FT_Render_Mode>(anti_alias));
  if (error) {
    return nullptr;
  }

  const FT_Bitmap& bitmap = glyph->bitmap;
  if (bitmap.width > kMaxGlyphDimension || bitmap.rows > kMaxGlyphDimension) {
    return nullptr;
  }
  int dib_width = bitmap.width;
  auto pGlyphBitmap =
      std::make_unique<CFX_GlyphBitmap>(glyph->bitmap_left, glyph->bitmap_top);
  pGlyphBitmap->GetBitmap()->Create(dib_width, bitmap.rows,
                                    anti_alias == FT_RENDER_MODE_MONO
                                        ? FXDIB_Format::k1bppMask
                                        : FXDIB_Format::k8bppMask);
  int dest_pitch = pGlyphBitmap->GetBitmap()->GetPitch();
  uint8_t* pDestBuf = pGlyphBitmap->GetBitmap()->GetWritableBuffer().data();
  const uint8_t* pSrcBuf = bitmap.buffer;
  if (anti_alias != FT_RENDER_MODE_MONO &&
      bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
    unsigned int bytes = anti_alias == FT_RENDER_MODE_LCD ? 3 : 1;
    for (unsigned int i = 0; i < bitmap.rows; i++) {
      for (unsigned int n = 0; n < bitmap.width; n++) {
        uint8_t data =
            (pSrcBuf[i * bitmap.pitch + n / 8] & (0x80 >> (n % 8))) ? 255 : 0;
        for (unsigned int b = 0; b < bytes; b++) {
          pDestBuf[i * dest_pitch + n * bytes + b] = data;
        }
      }
    }
  } else {
    FXSYS_memset(pDestBuf, 0, dest_pitch * bitmap.rows);
    int rowbytes = std::min(abs(bitmap.pitch), dest_pitch);
    for (unsigned int row = 0; row < bitmap.rows; row++) {
      FXSYS_memcpy(pDestBuf + row * dest_pitch, pSrcBuf + row * bitmap.pitch,
                   rowbytes);
    }
  }
  return pGlyphBitmap;
}

bool CFX_Face::SelectCharMap(fxge::FontEncoding encoding) {
  FT_Error error =
      FT_Select_Charmap(GetRec(), static_cast<FT_Encoding>(encoding));
  return !error;
}

CFX_Face::CFX_Face(FXFT_FaceRec* rec, RetainPtr<Retainable> pDesc)
    : m_pRec(rec), m_pDesc(std::move(pDesc)) {
  DCHECK(m_pRec);
}

CFX_Face::~CFX_Face() = default;
