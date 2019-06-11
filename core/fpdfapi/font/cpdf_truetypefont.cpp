// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_truetypefont.h"

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxge/fx_font.h"

namespace {

const uint8_t kPrefix[4] = {0x00, 0xf0, 0xf1, 0xf2};

}  // namespace

CPDF_TrueTypeFont::CPDF_TrueTypeFont(CPDF_Document* pDocument,
                                     CPDF_Dictionary* pFontDict)
    : CPDF_SimpleFont(pDocument, pFontDict) {}

CPDF_TrueTypeFont::~CPDF_TrueTypeFont() = default;

bool CPDF_TrueTypeFont::IsTrueTypeFont() const {
  return true;
}

const CPDF_TrueTypeFont* CPDF_TrueTypeFont::AsTrueTypeFont() const {
  return this;
}

CPDF_TrueTypeFont* CPDF_TrueTypeFont::AsTrueTypeFont() {
  return this;
}

bool CPDF_TrueTypeFont::Load() {
  return LoadCommon();
}

void CPDF_TrueTypeFont::LoadGlyphMap() {
  FXFT_FaceRec* face = m_Font.GetFaceRec();
  if (!face)
    return;

  int baseEncoding = m_BaseEncoding;
  if (m_pFontFile && face->num_charmaps > 0 &&
      (baseEncoding == PDFFONT_ENCODING_MACROMAN ||
       baseEncoding == PDFFONT_ENCODING_WINANSI) &&
      FontStyleIsSymbolic(m_Flags)) {
    bool bSupportWin = false;
    bool bSupportMac = false;
    for (int i = 0; i < FXFT_Get_Face_CharmapCount(face); i++) {
      int platform_id =
          FXFT_Get_Charmap_PlatformID(FXFT_Get_Face_Charmaps(face)[i]);
      if (platform_id == 0 || platform_id == 3) {
        bSupportWin = true;
      } else if (platform_id == 0 || platform_id == 1) {
        bSupportMac = true;
      }
    }
    if (baseEncoding == PDFFONT_ENCODING_WINANSI && !bSupportWin) {
      baseEncoding =
          bSupportMac ? PDFFONT_ENCODING_MACROMAN : PDFFONT_ENCODING_BUILTIN;
    } else if (baseEncoding == PDFFONT_ENCODING_MACROMAN && !bSupportMac) {
      baseEncoding =
          bSupportWin ? PDFFONT_ENCODING_WINANSI : PDFFONT_ENCODING_BUILTIN;
    }
  }
  if (((baseEncoding == PDFFONT_ENCODING_MACROMAN ||
        baseEncoding == PDFFONT_ENCODING_WINANSI) &&
       m_CharNames.empty()) ||
      FontStyleIsNonSymbolic(m_Flags)) {
    if (!FXFT_Has_Glyph_Names(face) &&
        (!face->num_charmaps || !face->charmaps)) {
      int nStartChar = m_pFontDict->GetIntegerFor("FirstChar");
      if (nStartChar < 0 || nStartChar > 255)
        return;

      int charcode = 0;
      for (; charcode < nStartChar; charcode++)
        m_GlyphIndex[charcode] = 0;
      uint16_t nGlyph = charcode - nStartChar + 3;
      for (; charcode < 256; charcode++, nGlyph++)
        m_GlyphIndex[charcode] = nGlyph;
      return;
    }
    bool bMSUnicode = FT_UseTTCharmap(face, 3, 1);
    bool bMacRoman = false;
    bool bMSSymbol = false;
    if (!bMSUnicode) {
      if (FontStyleIsNonSymbolic(m_Flags)) {
        bMacRoman = FT_UseTTCharmap(face, 1, 0);
        bMSSymbol = !bMacRoman && FT_UseTTCharmap(face, 3, 0);
      } else {
        bMSSymbol = FT_UseTTCharmap(face, 3, 0);
        bMacRoman = !bMSSymbol && FT_UseTTCharmap(face, 1, 0);
      }
    }
    bool bToUnicode = m_pFontDict->KeyExist("ToUnicode");
    for (uint32_t charcode = 0; charcode < 256; charcode++) {
      const char* name = GetAdobeCharName(baseEncoding, m_CharNames, charcode);
      if (!name) {
        m_GlyphIndex[charcode] =
            m_pFontFile ? FT_Get_Char_Index(face, charcode) : -1;
        continue;
      }
      m_Encoding.SetUnicode(charcode, PDF_UnicodeFromAdobeName(name));
      if (bMSSymbol) {
        for (size_t j = 0; j < FX_ArraySize(kPrefix); j++) {
          uint16_t unicode = kPrefix[j] * 256 + charcode;
          m_GlyphIndex[charcode] = FT_Get_Char_Index(face, unicode);
          if (m_GlyphIndex[charcode])
            break;
        }
      } else if (m_Encoding.UnicodeFromCharCode(charcode)) {
        if (bMSUnicode) {
          m_GlyphIndex[charcode] =
              FT_Get_Char_Index(face, m_Encoding.UnicodeFromCharCode(charcode));
        } else if (bMacRoman) {
          uint32_t maccode =
              FT_CharCodeFromUnicode(FT_ENCODING_APPLE_ROMAN,
                                     m_Encoding.UnicodeFromCharCode(charcode));
          if (!maccode) {
            m_GlyphIndex[charcode] = FXFT_Get_Name_Index(face, name);
          } else {
            m_GlyphIndex[charcode] = FT_Get_Char_Index(face, maccode);
          }
        }
      }
      if ((m_GlyphIndex[charcode] != 0 && m_GlyphIndex[charcode] != 0xffff) ||
          !name) {
        continue;
      }
      if (strcmp(name, ".notdef") == 0) {
        m_GlyphIndex[charcode] = FT_Get_Char_Index(face, 32);
        continue;
      }
      m_GlyphIndex[charcode] = FXFT_Get_Name_Index(face, name);
      if (m_GlyphIndex[charcode] != 0 || !bToUnicode)
        continue;

      WideString wsUnicode = UnicodeFromCharCode(charcode);
      if (!wsUnicode.IsEmpty()) {
        m_GlyphIndex[charcode] = FT_Get_Char_Index(face, wsUnicode[0]);
        m_Encoding.SetUnicode(charcode, wsUnicode[0]);
      }
    }
    return;
  }
  if (FT_UseTTCharmap(face, 3, 0)) {
    bool bFound = false;
    for (int charcode = 0; charcode < 256; charcode++) {
      for (size_t j = 0; j < FX_ArraySize(kPrefix); j++) {
        uint16_t unicode = kPrefix[j] * 256 + charcode;
        m_GlyphIndex[charcode] = FT_Get_Char_Index(face, unicode);
        if (m_GlyphIndex[charcode]) {
          bFound = true;
          break;
        }
      }
    }
    if (bFound) {
      if (baseEncoding != PDFFONT_ENCODING_BUILTIN) {
        for (uint32_t charcode = 0; charcode < 256; charcode++) {
          const char* name =
              GetAdobeCharName(baseEncoding, m_CharNames, charcode);
          if (name)
            m_Encoding.SetUnicode(charcode, PDF_UnicodeFromAdobeName(name));
        }
      } else if (FT_UseTTCharmap(face, 1, 0)) {
        for (int charcode = 0; charcode < 256; charcode++) {
          m_Encoding.SetUnicode(
              charcode,
              FT_UnicodeFromCharCode(FT_ENCODING_APPLE_ROMAN, charcode));
        }
      }
      return;
    }
  }
  if (FT_UseTTCharmap(face, 1, 0)) {
    bool bFound = false;
    for (int charcode = 0; charcode < 256; charcode++) {
      m_GlyphIndex[charcode] = FT_Get_Char_Index(face, charcode);
      m_Encoding.SetUnicode(
          charcode, FT_UnicodeFromCharCode(FT_ENCODING_APPLE_ROMAN, charcode));
      if (m_GlyphIndex[charcode]) {
        bFound = true;
      }
    }
    if (m_pFontFile || bFound)
      return;
  }
  if (FXFT_Select_Charmap(face, FT_ENCODING_UNICODE) == 0) {
    bool bFound = false;
    const uint16_t* pUnicodes = PDF_UnicodesForPredefinedCharSet(baseEncoding);
    for (uint32_t charcode = 0; charcode < 256; charcode++) {
      if (m_pFontFile) {
        m_Encoding.SetUnicode(charcode, charcode);
      } else {
        const char* name = GetAdobeCharName(0, m_CharNames, charcode);
        if (name)
          m_Encoding.SetUnicode(charcode, PDF_UnicodeFromAdobeName(name));
        else if (pUnicodes)
          m_Encoding.SetUnicode(charcode, pUnicodes[charcode]);
      }
      m_GlyphIndex[charcode] =
          FT_Get_Char_Index(face, m_Encoding.UnicodeFromCharCode(charcode));
      if (m_GlyphIndex[charcode])
        bFound = true;
    }
    if (bFound)
      return;
  }
  for (int charcode = 0; charcode < 256; charcode++)
    m_GlyphIndex[charcode] = charcode;
}
