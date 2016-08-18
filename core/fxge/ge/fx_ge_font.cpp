// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/include/fx_font.h"

#include "core/fpdfapi/fpdf_font/include/cpdf_font.h"
#include "core/fxge/ge/fx_text_int.h"
#include "core/fxge/include/cfx_fontmgr.h"
#include "core/fxge/include/cfx_gemodule.h"
#include "core/fxge/include/fx_freetype.h"

#define EM_ADJUST(em, a) (em == 0 ? (a) : (a)*1000 / em)

namespace {

#ifdef PDF_ENABLE_XFA

unsigned long FTStreamRead(FXFT_Stream stream,
                           unsigned long offset,
                           unsigned char* buffer,
                           unsigned long count) {
  if (count == 0)
    return 0;

  IFX_FileRead* pFile = static_cast<IFX_FileRead*>(stream->descriptor.pointer);
  return pFile->ReadBlock(buffer, offset, count) ? count : 0;
}

void FTStreamClose(FXFT_Stream stream) {}

FX_BOOL LoadFileImp(FXFT_Library library,
                    FXFT_Face* Face,
                    IFX_FileRead* pFile,
                    int32_t faceIndex,
                    std::unique_ptr<FXFT_StreamRec>* stream) {
  std::unique_ptr<FXFT_StreamRec> stream1(new FXFT_StreamRec());
  stream1->base = nullptr;
  stream1->size = static_cast<unsigned long>(pFile->GetSize());
  stream1->pos = 0;
  stream1->descriptor.pointer = pFile;
  stream1->close = FTStreamClose;
  stream1->read = FTStreamRead;
  FXFT_Open_Args args;
  args.flags = FT_OPEN_STREAM;
  args.stream = stream1.get();
  if (FXFT_Open_Face(library, &args, faceIndex, Face))
    return FALSE;
  if (stream)
    *stream = std::move(stream1);
  return TRUE;
}
#endif  // PDF_ENABLE_XFA

FXFT_Face FT_LoadFont(const uint8_t* pData, int size) {
  return CFX_GEModule::Get()->GetFontMgr()->GetFixedFace(pData, size, 0);
}

}  // namespace

CFX_Font::CFX_Font()
#ifdef PDF_ENABLE_XFA
    : m_bLogic(FALSE),
      m_pOwnedStream(nullptr),
      m_Face(nullptr),
#else
    : m_Face(nullptr),
#endif  // PDF_ENABLE_XFA
      m_pFontData(nullptr),
      m_pGsubData(nullptr),
      m_dwSize(0),
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
      m_pPlatformFont(nullptr),
#endif
      m_bEmbedded(FALSE),
      m_bVertical(FALSE) {
}

#ifdef PDF_ENABLE_XFA
FX_BOOL CFX_Font::LoadClone(const CFX_Font* pFont) {
  if (!pFont)
    return FALSE;

  m_bLogic = TRUE;
  if (pFont->m_pSubstFont) {
    m_pSubstFont.reset(new CFX_SubstFont);
    m_pSubstFont->m_Charset = pFont->m_pSubstFont->m_Charset;
    m_pSubstFont->m_SubstFlags = pFont->m_pSubstFont->m_SubstFlags;
    m_pSubstFont->m_Weight = pFont->m_pSubstFont->m_Weight;
    m_pSubstFont->m_Family = pFont->m_pSubstFont->m_Family;
    m_pSubstFont->m_ItalicAngle = pFont->m_pSubstFont->m_ItalicAngle;
  }
  if (pFont->m_OtfFontData.GetSize()) {
    m_OtfFontData.AttachData(pFont->m_OtfFontData.GetBuffer(),
                             pFont->m_OtfFontData.GetSize());
  }
  m_Face = pFont->m_Face;
  m_bEmbedded = pFont->m_bEmbedded;
  m_bVertical = pFont->m_bVertical;
  m_dwSize = pFont->m_dwSize;
  m_pFontData = pFont->m_pFontData;
  m_pGsubData = pFont->m_pGsubData;
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  m_pPlatformFont = pFont->m_pPlatformFont;
#endif
  m_pOwnedStream = pFont->m_pOwnedStream;
  return TRUE;
}
#endif  // PDF_ENABLE_XFA

CFX_Font::~CFX_Font() {
#ifdef PDF_ENABLE_XFA
  if (m_bLogic) {
    m_OtfFontData.DetachBuffer();
    return;
  }
#endif  // PDF_ENABLE_XFA
  if (m_Face) {
#ifndef PDF_ENABLE_XFA
    if (FXFT_Get_Face_External_Stream(m_Face)) {
      FXFT_Clear_Face_External_Stream(m_Face);
    }
#endif  // PDF_ENABLE_XFA
    if (m_bEmbedded) {
      DeleteFace();
    } else {
      CFX_GEModule::Get()->GetFontMgr()->ReleaseFace(m_Face);
    }
  }
#ifdef PDF_ENABLE_XFA
  FX_Free(m_pOwnedStream);
#endif  // PDF_ENABLE_XFA
  FX_Free(m_pGsubData);
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_ && !defined _SKIA_SUPPORT_
  ReleasePlatformResource();
#endif
}

void CFX_Font::DeleteFace() {
  FXFT_Done_Face(m_Face);
  m_Face = nullptr;
}

void CFX_Font::LoadSubst(const CFX_ByteString& face_name,
                         FX_BOOL bTrueType,
                         uint32_t flags,
                         int weight,
                         int italic_angle,
                         int CharsetCP,
                         FX_BOOL bVertical) {
  m_bEmbedded = FALSE;
  m_bVertical = bVertical;
  m_pSubstFont.reset(new CFX_SubstFont);
  m_Face = CFX_GEModule::Get()->GetFontMgr()->FindSubstFont(
      face_name, bTrueType, flags, weight, italic_angle, CharsetCP,
      m_pSubstFont.get());
  if (m_Face) {
    m_pFontData = FXFT_Get_Face_Stream_Base(m_Face);
    m_dwSize = FXFT_Get_Face_Stream_Size(m_Face);
  }
}

#ifdef PDF_ENABLE_XFA
FX_BOOL CFX_Font::LoadFile(IFX_FileRead* pFile,
                           int nFaceIndex,
                           int* pFaceCount) {
  m_bEmbedded = FALSE;

  CFX_FontMgr* pFontMgr = CFX_GEModule::Get()->GetFontMgr();
  pFontMgr->InitFTLibrary();
  FXFT_Library library = pFontMgr->GetFTLibrary();

  std::unique_ptr<FXFT_StreamRec> stream;
  if (!LoadFileImp(library, &m_Face, pFile, nFaceIndex, &stream))
    return FALSE;

  if (pFaceCount)
    *pFaceCount = (int)m_Face->num_faces;
  m_pOwnedStream = stream.release();
  FXFT_Set_Pixel_Sizes(m_Face, 0, 64);
  return TRUE;
}
#endif  // PDF_ENABLE_XFA

int CFX_Font::GetGlyphWidth(uint32_t glyph_index) {
  if (!m_Face) {
    return 0;
  }
  if (m_pSubstFont && (m_pSubstFont->m_SubstFlags & FXFONT_SUBST_MM)) {
    AdjustMMParams(glyph_index, 0, 0);
  }
  int err = FXFT_Load_Glyph(
      m_Face, glyph_index,
      FXFT_LOAD_NO_SCALE | FXFT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
  if (err) {
    return 0;
  }
  int width = EM_ADJUST(FXFT_Get_Face_UnitsPerEM(m_Face),
                        FXFT_Get_Glyph_HoriAdvance(m_Face));
  return width;
}

FX_BOOL CFX_Font::LoadEmbedded(const uint8_t* data, uint32_t size) {
  std::vector<uint8_t> temp(data, data + size);
  m_pFontDataAllocation.swap(temp);
  m_Face = FT_LoadFont(m_pFontDataAllocation.data(), size);
  m_pFontData = m_pFontDataAllocation.data();
  m_bEmbedded = TRUE;
  m_dwSize = size;
  return !!m_Face;
}

FX_BOOL CFX_Font::IsTTFont() const {
  if (!m_Face)
    return FALSE;
  return FXFT_Is_Face_TT_OT(m_Face) == FXFT_FACE_FLAG_SFNT;
}

int CFX_Font::GetAscent() const {
  if (!m_Face)
    return 0;
  return EM_ADJUST(FXFT_Get_Face_UnitsPerEM(m_Face),
                   FXFT_Get_Face_Ascender(m_Face));
}

int CFX_Font::GetDescent() const {
  if (!m_Face)
    return 0;
  return EM_ADJUST(FXFT_Get_Face_UnitsPerEM(m_Face),
                   FXFT_Get_Face_Descender(m_Face));
}

FX_BOOL CFX_Font::GetGlyphBBox(uint32_t glyph_index, FX_RECT& bbox) {
  if (!m_Face)
    return FALSE;

  if (FXFT_Is_Face_Tricky(m_Face)) {
    int error = FXFT_Set_Char_Size(m_Face, 0, 1000 * 64, 72, 72);
    if (error) {
      return FALSE;
    }
    error = FXFT_Load_Glyph(m_Face, glyph_index,
                            FXFT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
    if (error) {
      return FALSE;
    }
    FXFT_BBox cbox;
    FT_Glyph glyph;
    error = FXFT_Get_Glyph(((FXFT_Face)m_Face)->glyph, &glyph);
    if (error) {
      return FALSE;
    }
    FXFT_Glyph_Get_CBox(glyph, FXFT_GLYPH_BBOX_PIXELS, &cbox);
    int pixel_size_x = ((FXFT_Face)m_Face)->size->metrics.x_ppem,
        pixel_size_y = ((FXFT_Face)m_Face)->size->metrics.y_ppem;
    if (pixel_size_x == 0 || pixel_size_y == 0) {
      bbox.left = cbox.xMin;
      bbox.right = cbox.xMax;
      bbox.top = cbox.yMax;
      bbox.bottom = cbox.yMin;
    } else {
      bbox.left = cbox.xMin * 1000 / pixel_size_x;
      bbox.right = cbox.xMax * 1000 / pixel_size_x;
      bbox.top = cbox.yMax * 1000 / pixel_size_y;
      bbox.bottom = cbox.yMin * 1000 / pixel_size_y;
    }
    if (bbox.top > FXFT_Get_Face_Ascender(m_Face)) {
      bbox.top = FXFT_Get_Face_Ascender(m_Face);
    }
    if (bbox.bottom < FXFT_Get_Face_Descender(m_Face)) {
      bbox.bottom = FXFT_Get_Face_Descender(m_Face);
    }
    FT_Done_Glyph(glyph);
    return FXFT_Set_Pixel_Sizes(m_Face, 0, 64) == 0;
  }
  if (FXFT_Load_Glyph(
          m_Face, glyph_index,
          FXFT_LOAD_NO_SCALE | FXFT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH)) {
    return FALSE;
  }
  int em = FXFT_Get_Face_UnitsPerEM(m_Face);
  if (em == 0) {
    bbox.left = FXFT_Get_Glyph_HoriBearingX(m_Face);
    bbox.bottom = FXFT_Get_Glyph_HoriBearingY(m_Face);
    bbox.top = bbox.bottom - FXFT_Get_Glyph_Height(m_Face);
    bbox.right = bbox.left + FXFT_Get_Glyph_Width(m_Face);
  } else {
    bbox.left = FXFT_Get_Glyph_HoriBearingX(m_Face) * 1000 / em;
    bbox.top =
        (FXFT_Get_Glyph_HoriBearingY(m_Face) - FXFT_Get_Glyph_Height(m_Face)) *
        1000 / em;
    bbox.right =
        (FXFT_Get_Glyph_HoriBearingX(m_Face) + FXFT_Get_Glyph_Width(m_Face)) *
        1000 / em;
    bbox.bottom = (FXFT_Get_Glyph_HoriBearingY(m_Face)) * 1000 / em;
  }
  return TRUE;
}

FX_BOOL CFX_Font::IsItalic() const {
  if (!m_Face)
    return FALSE;

  FX_BOOL ret = FXFT_Is_Face_Italic(m_Face) == FXFT_STYLE_FLAG_ITALIC;
  if (!ret) {
    CFX_ByteString str(FXFT_Get_Face_Style_Name(m_Face));
    str.MakeLower();
    if (str.Find("italic") != -1) {
      ret = TRUE;
    }
  }
  return ret;
}

FX_BOOL CFX_Font::IsBold() const {
  if (!m_Face)
    return FALSE;
  return FXFT_Is_Face_Bold(m_Face) == FXFT_STYLE_FLAG_BOLD;
}

FX_BOOL CFX_Font::IsFixedWidth() const {
  if (!m_Face)
    return FALSE;
  return FXFT_Is_Face_fixedwidth(m_Face);
}

CFX_ByteString CFX_Font::GetPsName() const {
  if (!m_Face)
    return CFX_ByteString();

  CFX_ByteString psName = FXFT_Get_Postscript_Name(m_Face);
  if (psName.IsEmpty())
    psName = "Untitled";
  return psName;
}

CFX_ByteString CFX_Font::GetFamilyName() const {
  if (!m_Face && !m_pSubstFont) {
    return CFX_ByteString();
  }
  if (m_Face) {
    return CFX_ByteString(FXFT_Get_Face_Family_Name(m_Face));
  }
  return m_pSubstFont->m_Family;
}
CFX_ByteString CFX_Font::GetFaceName() const {
  if (!m_Face && !m_pSubstFont) {
    return CFX_ByteString();
  }
  if (m_Face) {
    CFX_ByteString facename;
    CFX_ByteString style = CFX_ByteString(FXFT_Get_Face_Style_Name(m_Face));
    facename = GetFamilyName();
    if (facename.IsEmpty()) {
      facename = "Untitled";
    }
    if (!style.IsEmpty() && style != "Regular") {
      facename += " " + style;
    }
    return facename;
  }
  return m_pSubstFont->m_Family;
}
FX_BOOL CFX_Font::GetBBox(FX_RECT& bbox) {
  if (!m_Face) {
    return FALSE;
  }
  int em = FXFT_Get_Face_UnitsPerEM(m_Face);
  if (em == 0) {
    bbox.left = FXFT_Get_Face_xMin(m_Face);
    bbox.bottom = FXFT_Get_Face_yMax(m_Face);
    bbox.top = FXFT_Get_Face_yMin(m_Face);
    bbox.right = FXFT_Get_Face_xMax(m_Face);
  } else {
    bbox.left = FXFT_Get_Face_xMin(m_Face) * 1000 / em;
    bbox.top = FXFT_Get_Face_yMin(m_Face) * 1000 / em;
    bbox.right = FXFT_Get_Face_xMax(m_Face) * 1000 / em;
    bbox.bottom = FXFT_Get_Face_yMax(m_Face) * 1000 / em;
  }
  return TRUE;
}

int CFX_Font::GetHeight() const {
  if (!m_Face)
    return 0;

  return EM_ADJUST(FXFT_Get_Face_UnitsPerEM(m_Face),
                   FXFT_Get_Face_Height(m_Face));
}

int CFX_Font::GetMaxAdvanceWidth() const {
  if (!m_Face)
    return 0;

  return EM_ADJUST(FXFT_Get_Face_UnitsPerEM(m_Face),
                   FXFT_Get_Face_MaxAdvanceWidth(m_Face));
}

int CFX_Font::GetULPos() const {
  if (!m_Face)
    return 0;

  return EM_ADJUST(FXFT_Get_Face_UnitsPerEM(m_Face),
                   FXFT_Get_Face_UnderLinePosition(m_Face));
}

int CFX_Font::GetULthickness() const {
  if (!m_Face)
    return 0;

  return EM_ADJUST(FXFT_Get_Face_UnitsPerEM(m_Face),
                   FXFT_Get_Face_UnderLineThickness(m_Face));
}
