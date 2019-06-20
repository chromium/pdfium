// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/android/cfpf_skiafont.h"

#include <algorithm>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/android/cfpf_skiafontmgr.h"
#include "core/fxge/android/cfpf_skiapathfont.h"
#include "core/fxge/fx_freetype.h"

#define FPF_EM_ADJUST(em, a) (em == 0 ? (a) : (a)*1000 / em)

CFPF_SkiaFont::CFPF_SkiaFont(CFPF_SkiaFontMgr* pFontMgr,
                             const CFPF_SkiaPathFont* pFont,
                             uint32_t dwStyle,
                             uint8_t uCharset)
    : m_pFontMgr(pFontMgr),
      m_pFont(pFont),
      m_Face(m_pFontMgr->GetFontFace(m_pFont->path(), m_pFont->face_index())),
      m_dwStyle(dwStyle),
      m_uCharset(uCharset) {}

CFPF_SkiaFont::~CFPF_SkiaFont() = default;

ByteString CFPF_SkiaFont::GetFamilyName() {
  if (!m_Face)
    return ByteString();
  return ByteString(FXFT_Get_Face_Family_Name(GetFaceRec()));
}

ByteString CFPF_SkiaFont::GetPsName() {
  if (!m_Face)
    return ByteString();
  return FT_Get_Postscript_Name(GetFaceRec());
}

int32_t CFPF_SkiaFont::GetGlyphIndex(wchar_t wUnicode) {
  if (!m_Face)
    return wUnicode;
  if (FXFT_Select_Charmap(GetFaceRec(), FT_ENCODING_UNICODE))
    return 0;
  return FT_Get_Char_Index(GetFaceRec(), wUnicode);
}

int32_t CFPF_SkiaFont::GetGlyphWidth(int32_t iGlyphIndex) {
  if (!m_Face)
    return 0;
  if (FT_Load_Glyph(GetFaceRec(), iGlyphIndex,
                    FT_LOAD_NO_SCALE | FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH)) {
    return 0;
  }
  return FPF_EM_ADJUST(FXFT_Get_Face_UnitsPerEM(GetFaceRec()),
                       FXFT_Get_Glyph_HoriAdvance(GetFaceRec()));
}

int32_t CFPF_SkiaFont::GetAscent() const {
  if (!m_Face)
    return 0;
  return FPF_EM_ADJUST(FXFT_Get_Face_UnitsPerEM(GetFaceRec()),
                       FXFT_Get_Face_Ascender(GetFaceRec()));
}

int32_t CFPF_SkiaFont::GetDescent() const {
  if (!m_Face)
    return 0;
  return FPF_EM_ADJUST(FXFT_Get_Face_UnitsPerEM(GetFaceRec()),
                       FXFT_Get_Face_Descender(GetFaceRec()));
}

bool CFPF_SkiaFont::GetGlyphBBox(int32_t iGlyphIndex, FX_RECT& rtBBox) {
  if (!m_Face)
    return false;
  if (FXFT_Is_Face_Tricky(GetFaceRec())) {
    if (FT_Set_Char_Size(GetFaceRec(), 0, 1000 * 64, 72, 72))
      return false;
    if (FT_Load_Glyph(GetFaceRec(), iGlyphIndex,
                      FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH)) {
      FT_Set_Pixel_Sizes(GetFaceRec(), 0, 64);
      return false;
    }
    FT_Glyph glyph;
    if (FT_Get_Glyph(GetFaceRec()->glyph, &glyph)) {
      FT_Set_Pixel_Sizes(GetFaceRec(), 0, 64);
      return false;
    }
    FT_BBox cbox;
    FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_PIXELS, &cbox);
    int32_t x_ppem = GetFaceRec()->size->metrics.x_ppem;
    int32_t y_ppem = GetFaceRec()->size->metrics.y_ppem;
    rtBBox.left = FPF_EM_ADJUST(x_ppem, cbox.xMin);
    rtBBox.right = FPF_EM_ADJUST(x_ppem, cbox.xMax);
    rtBBox.top = FPF_EM_ADJUST(y_ppem, cbox.yMax);
    rtBBox.bottom = FPF_EM_ADJUST(y_ppem, cbox.yMin);
    rtBBox.top = std::min(rtBBox.top, GetAscent());
    rtBBox.bottom = std::max(rtBBox.bottom, GetDescent());
    FT_Done_Glyph(glyph);
    return FT_Set_Pixel_Sizes(GetFaceRec(), 0, 64) == 0;
  }
  if (FT_Load_Glyph(GetFaceRec(), iGlyphIndex,
                    FT_LOAD_NO_SCALE | FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH)) {
    return false;
  }
  rtBBox.left = FPF_EM_ADJUST(FXFT_Get_Face_UnitsPerEM(GetFaceRec()),
                              FXFT_Get_Glyph_HoriBearingX(GetFaceRec()));
  rtBBox.bottom = FPF_EM_ADJUST(FXFT_Get_Face_UnitsPerEM(GetFaceRec()),
                                FXFT_Get_Glyph_HoriBearingY(GetFaceRec()));
  rtBBox.right = FPF_EM_ADJUST(FXFT_Get_Face_UnitsPerEM(GetFaceRec()),
                               FXFT_Get_Glyph_HoriBearingX(GetFaceRec()) +
                                   FXFT_Get_Glyph_Width(GetFaceRec()));
  rtBBox.top = FPF_EM_ADJUST(FXFT_Get_Face_UnitsPerEM(GetFaceRec()),
                             FXFT_Get_Glyph_HoriBearingY(GetFaceRec()) -
                                 FXFT_Get_Glyph_Height(GetFaceRec()));
  return true;
}

bool CFPF_SkiaFont::GetBBox(FX_RECT& rtBBox) {
  if (!m_Face) {
    return false;
  }
  rtBBox.left = FPF_EM_ADJUST(FXFT_Get_Face_UnitsPerEM(GetFaceRec()),
                              FXFT_Get_Face_xMin(GetFaceRec()));
  rtBBox.top = FPF_EM_ADJUST(FXFT_Get_Face_UnitsPerEM(GetFaceRec()),
                             FXFT_Get_Face_yMin(GetFaceRec()));
  rtBBox.right = FPF_EM_ADJUST(FXFT_Get_Face_UnitsPerEM(GetFaceRec()),
                               FXFT_Get_Face_xMax(GetFaceRec()));
  rtBBox.bottom = FPF_EM_ADJUST(FXFT_Get_Face_UnitsPerEM(GetFaceRec()),
                                FXFT_Get_Face_yMax(GetFaceRec()));
  return true;
}

int32_t CFPF_SkiaFont::GetHeight() const {
  if (!m_Face)
    return 0;
  return FPF_EM_ADJUST(FXFT_Get_Face_UnitsPerEM(GetFaceRec()),
                       FXFT_Get_Face_Height(GetFaceRec()));
}

int32_t CFPF_SkiaFont::GetItalicAngle() const {
  if (!m_Face)
    return 0;

  auto* info = static_cast<TT_Postscript*>(
      FT_Get_Sfnt_Table(GetFaceRec(), ft_sfnt_post));
  return info ? info->italicAngle : 0;
}

uint32_t CFPF_SkiaFont::GetFontData(uint32_t dwTable,
                                    pdfium::span<uint8_t> pBuffer) {
  if (!m_Face)
    return 0;

  FT_ULong ulSize = pdfium::base::checked_cast<FT_ULong>(pBuffer.size());
  if (FT_Load_Sfnt_Table(GetFaceRec(), dwTable, 0, pBuffer.data(), &ulSize))
    return 0;
  return pdfium::base::checked_cast<uint32_t>(ulSize);
}
