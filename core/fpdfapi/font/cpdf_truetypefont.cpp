// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_truetypefont.h"

#include <algorithm>
#include <utility>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxge/fx_font.h"

namespace {

constexpr uint8_t kPrefix[4] = {0x00, 0xf0, 0xf1, 0xf2};

uint16_t GetGlyphIndexForMSSymbol(FXFT_FaceRec* face, uint32_t charcode) {
  for (uint8_t c : kPrefix) {
    uint16_t unicode = c * 256 + charcode;
    uint16_t val = FT_Get_Char_Index(face, unicode);
    if (val)
      return val;
  }
  return 0;
}

bool IsWinAnsiOrMacRomanEncoding(FontEncoding encoding) {
  return encoding == FontEncoding::kWinAnsi ||
         encoding == FontEncoding::kMacRoman;
}

}  // namespace

CPDF_TrueTypeFont::CPDF_TrueTypeFont(CPDF_Document* pDocument,
                                     RetainPtr<CPDF_Dictionary> pFontDict)
    : CPDF_SimpleFont(pDocument, std::move(pFontDict)) {}

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

  const FontEncoding base_encoding = DetermineEncoding();
  if ((IsWinAnsiOrMacRomanEncoding(base_encoding) && m_CharNames.empty()) ||
      FontStyleIsNonSymbolic(m_Flags)) {
    if (!FXFT_Has_Glyph_Names(face) &&
        (!face->num_charmaps || !face->charmaps)) {
      SetGlyphIndicesFromFirstChar();
      return;
    }

    const CharmapType charmap_type = DetermineCharmapType();
    bool bToUnicode = m_pFontDict->KeyExist("ToUnicode");
    for (uint32_t charcode = 0; charcode < 256; charcode++) {
      const char* name = GetAdobeCharName(base_encoding, m_CharNames, charcode);
      if (!name) {
        m_GlyphIndex[charcode] =
            m_pFontFile ? FT_Get_Char_Index(face, charcode) : -1;
        continue;
      }
      m_Encoding.SetUnicode(charcode, UnicodeFromAdobeName(name));
      if (charmap_type == CharmapType::kMSSymbol) {
        m_GlyphIndex[charcode] = GetGlyphIndexForMSSymbol(face, charcode);
      } else if (m_Encoding.UnicodeFromCharCode(charcode)) {
        if (charmap_type == CharmapType::kMSUnicode) {
          m_GlyphIndex[charcode] =
              FT_Get_Char_Index(face, m_Encoding.UnicodeFromCharCode(charcode));
        } else if (charmap_type == CharmapType::kMacRoman) {
          uint32_t maccode = CharCodeFromUnicodeForFreetypeEncoding(
              FT_ENCODING_APPLE_ROMAN,
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
    for (uint32_t charcode = 0; charcode < 256; charcode++)
      m_GlyphIndex[charcode] = GetGlyphIndexForMSSymbol(face, charcode);
    if (HasAnyGlyphIndex()) {
      if (base_encoding != FontEncoding::kBuiltin) {
        for (uint32_t charcode = 0; charcode < 256; charcode++) {
          const char* name =
              GetAdobeCharName(base_encoding, m_CharNames, charcode);
          if (name)
            m_Encoding.SetUnicode(charcode, UnicodeFromAdobeName(name));
        }
      } else if (UseTTCharmapMacRoman(face)) {
        for (uint32_t charcode = 0; charcode < 256; charcode++) {
          m_Encoding.SetUnicode(charcode,
                                UnicodeFromAppleRomanCharCode(charcode));
        }
      }
      return;
    }
  }
  if (UseTTCharmapMacRoman(face)) {
    for (uint32_t charcode = 0; charcode < 256; charcode++) {
      m_GlyphIndex[charcode] = FT_Get_Char_Index(face, charcode);
      m_Encoding.SetUnicode(charcode, UnicodeFromAppleRomanCharCode(charcode));
    }
    if (m_pFontFile || HasAnyGlyphIndex())
      return;
  }
  if (FXFT_Select_Charmap(face, FT_ENCODING_UNICODE) == 0) {
    pdfium::span<const uint16_t> unicodes =
        UnicodesForPredefinedCharSet(base_encoding);
    for (uint32_t charcode = 0; charcode < 256; charcode++) {
      if (m_pFontFile) {
        m_Encoding.SetUnicode(charcode, charcode);
      } else {
        const char* name =
            GetAdobeCharName(FontEncoding::kBuiltin, m_CharNames, charcode);
        if (name) {
          m_Encoding.SetUnicode(charcode, UnicodeFromAdobeName(name));
        } else if (!unicodes.empty()) {
          m_Encoding.SetUnicode(charcode, unicodes[charcode]);
        }
      }
      m_GlyphIndex[charcode] =
          FT_Get_Char_Index(face, m_Encoding.UnicodeFromCharCode(charcode));
    }
    if (HasAnyGlyphIndex())
      return;
  }
  for (int charcode = 0; charcode < 256; charcode++)
    m_GlyphIndex[charcode] = charcode;
}

bool CPDF_TrueTypeFont::HasAnyGlyphIndex() const {
  for (uint32_t charcode = 0; charcode < kInternalTableSize; charcode++) {
    if (m_GlyphIndex[charcode])
      return true;
  }
  return false;
}

CPDF_TrueTypeFont::CharmapType CPDF_TrueTypeFont::DetermineCharmapType() const {
  if (UseTTCharmapMSUnicode(m_Font.GetFaceRec()))
    return CharmapType::kMSUnicode;

  if (FontStyleIsNonSymbolic(m_Flags)) {
    if (UseTTCharmapMacRoman(m_Font.GetFaceRec()))
      return CharmapType::kMacRoman;
    if (UseTTCharmapMSSymbol(m_Font.GetFaceRec()))
      return CharmapType::kMSSymbol;
  } else {
    if (UseTTCharmapMSSymbol(m_Font.GetFaceRec()))
      return CharmapType::kMSSymbol;
    if (UseTTCharmapMacRoman(m_Font.GetFaceRec()))
      return CharmapType::kMacRoman;
  }
  return CharmapType::kOther;
}

FontEncoding CPDF_TrueTypeFont::DetermineEncoding() const {
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

  if (m_BaseEncoding == FontEncoding::kWinAnsi && !support_win)
    return support_mac ? FontEncoding::kMacRoman : FontEncoding::kBuiltin;
  if (m_BaseEncoding == FontEncoding::kMacRoman && !support_mac)
    return support_win ? FontEncoding::kWinAnsi : FontEncoding::kBuiltin;
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
