// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_truetypefont.h"

#include <algorithm>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxge/fx_font.h"
#include "third_party/base/cxx17_backports.h"

namespace {

const uint8_t kPrefix[4] = {0x00, 0xf0, 0xf1, 0xf2};

bool IsWinAnsiOrMacRomanEncoding(int encoding) {
  return encoding == PDFFONT_ENCODING_WINANSI ||
         encoding == PDFFONT_ENCODING_MACROMAN;
}

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

  const int base_encoding = DetermineEncoding();
  if ((IsWinAnsiOrMacRomanEncoding(base_encoding) && m_CharNames.empty()) ||
      FontStyleIsNonSymbolic(m_Flags)) {
    if (!FXFT_Has_Glyph_Names(face) &&
        (!face->num_charmaps || !face->charmaps)) {
      SetGlyphIndicesFromFirstChar();
      return;
    }
    bool bMSUnicode = UseTTCharmapMSUnicode(face);
    bool bMacRoman = false;
    bool bMSSymbol = false;
    if (!bMSUnicode) {
      if (FontStyleIsNonSymbolic(m_Flags)) {
        bMacRoman = UseTTCharmapMacRoman(face);
        bMSSymbol = !bMacRoman && UseTTCharmapMSSymbol(face);
      } else {
        bMSSymbol = UseTTCharmapMSSymbol(face);
        bMacRoman = !bMSSymbol && UseTTCharmapMacRoman(face);
      }
    }
    bool bToUnicode = m_pFontDict->KeyExist("ToUnicode");
    for (uint32_t charcode = 0; charcode < 256; charcode++) {
      const char* name = GetAdobeCharName(base_encoding, m_CharNames, charcode);
      if (!name) {
        m_GlyphIndex[charcode] =
            m_pFontFile ? FT_Get_Char_Index(face, charcode) : -1;
        continue;
      }
      m_Encoding.SetUnicode(charcode, PDF_UnicodeFromAdobeName(name));
      if (bMSSymbol) {
        for (size_t j = 0; j < pdfium::size(kPrefix); j++) {
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
            m_GlyphIndex[charcode] = FT_Get_Name_Index(face, name);
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
      m_GlyphIndex[charcode] = FT_Get_Name_Index(face, name);
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
  if (UseTTCharmapMSSymbol(face)) {
    bool bFound = false;
    for (int charcode = 0; charcode < 256; charcode++) {
      for (size_t j = 0; j < pdfium::size(kPrefix); j++) {
        uint16_t unicode = kPrefix[j] * 256 + charcode;
        m_GlyphIndex[charcode] = FT_Get_Char_Index(face, unicode);
        if (m_GlyphIndex[charcode]) {
          bFound = true;
          break;
        }
      }
    }
    if (bFound) {
      if (base_encoding != PDFFONT_ENCODING_BUILTIN) {
        for (uint32_t charcode = 0; charcode < 256; charcode++) {
          const char* name =
              GetAdobeCharName(base_encoding, m_CharNames, charcode);
          if (name)
            m_Encoding.SetUnicode(charcode, PDF_UnicodeFromAdobeName(name));
        }
      } else if (UseTTCharmapMacRoman(face)) {
        for (int charcode = 0; charcode < 256; charcode++) {
          m_Encoding.SetUnicode(
              charcode,
              FT_UnicodeFromCharCode(FT_ENCODING_APPLE_ROMAN, charcode));
        }
      }
      return;
    }
  }
  if (UseTTCharmapMacRoman(face)) {
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
    const uint16_t* pUnicodes = PDF_UnicodesForPredefinedCharSet(base_encoding);
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

int CPDF_TrueTypeFont::DetermineEncoding() const {
  if (!m_pFontFile || !FontStyleIsSymbolic(m_Flags) ||
      !IsWinAnsiOrMacRomanEncoding(m_BaseEncoding)) {
    return m_BaseEncoding;
  }

  // Not null - caller checked.
  FXFT_FaceRec* face = m_Font.GetFaceRec();
  if (face->num_charmaps <= 0)
    return m_BaseEncoding;

  bool support_win = false;
  bool support_mac = false;
  for (int i = 0; i < face->num_charmaps; i++) {
    int platform_id = FXFT_Get_Charmap_PlatformID(face->charmaps[i]);
    if (platform_id == kNamePlatformAppleUnicode ||
        platform_id == kNamePlatformWindows) {
      support_win = true;
    } else if (platform_id == kNamePlatformMac) {
      support_mac = true;
    }
    if (support_win && support_mac)
      break;
  }

  if (m_BaseEncoding == PDFFONT_ENCODING_WINANSI && !support_win)
    return support_mac ? PDFFONT_ENCODING_MACROMAN : PDFFONT_ENCODING_BUILTIN;
  if (m_BaseEncoding == PDFFONT_ENCODING_MACROMAN && !support_mac)
    return support_win ? PDFFONT_ENCODING_WINANSI : PDFFONT_ENCODING_BUILTIN;
  return m_BaseEncoding;
}

void CPDF_TrueTypeFont::SetGlyphIndicesFromFirstChar() {
  int start_char = m_pFontDict->GetIntegerFor("FirstChar");
  if (start_char < 0 || start_char > 255)
    return;

  auto* it = std::begin(m_GlyphIndex);
  std::fill(it, it + start_char, 0);
  uint16_t glyph = 3;
  for (int charcode = start_char; charcode < 256; charcode++, glyph++)
    m_GlyphIndex[charcode] = glyph;
}
