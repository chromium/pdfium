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

uint16_t GetGlyphIndexForMSSymbol(const RetainPtr<CFX_Face>& face,
                                  uint32_t charcode) {
  for (uint8_t c : kPrefix) {
    uint16_t unicode = c * 256 + charcode;
    uint16_t val = face->GetCharIndex(unicode);
    if (val) {
      return val;
    }
  }
  return 0;
}

bool IsWinAnsiOrMacRomanEncoding(FontEncoding encoding) {
  return encoding == FontEncoding::kWinAnsi ||
         encoding == FontEncoding::kMacRoman;
}

}  // namespace

CPDF_TrueTypeFont::CPDF_TrueTypeFont(CPDF_Document* document,
                                     RetainPtr<CPDF_Dictionary> font_dict)
    : CPDF_SimpleFont(document, std::move(font_dict)) {}

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
  RetainPtr<CFX_Face> face = font_.GetFace();
  if (!face) {
    return;
  }

  const FontEncoding base_encoding = DetermineEncoding();
  if ((IsWinAnsiOrMacRomanEncoding(base_encoding) && char_names_.empty()) ||
      FontStyleIsNonSymbolic(flags_)) {
    if (font_.GetFace()->HasGlyphNames() && face->GetCharMapCount() == 0) {
      SetGlyphIndicesFromFirstChar();
      return;
    }

    const CharmapType charmap_type = DetermineCharmapType();
    bool bToUnicode = font_dict_->KeyExist("ToUnicode");
    for (uint32_t charcode = 0; charcode < 256; charcode++) {
      const char* name = GetAdobeCharName(base_encoding, char_names_, charcode);
      if (!name) {
        glyph_index_[charcode] = font_file_ ? face->GetCharIndex(charcode) : -1;
        continue;
      }
      encoding_.SetUnicode(charcode, UnicodeFromAdobeName(name));
      if (charmap_type == CharmapType::kMSSymbol) {
        glyph_index_[charcode] = GetGlyphIndexForMSSymbol(face, charcode);
      } else if (encoding_.UnicodeFromCharCode(charcode)) {
        if (charmap_type == CharmapType::kMSUnicode) {
          glyph_index_[charcode] =
              face->GetCharIndex(encoding_.UnicodeFromCharCode(charcode));
        } else if (charmap_type == CharmapType::kMacRoman) {
          uint32_t maccode = CharCodeFromUnicodeForEncoding(
              fxge::FontEncoding::kAppleRoman,
              encoding_.UnicodeFromCharCode(charcode));
          if (!maccode) {
            glyph_index_[charcode] = face->GetNameIndex(name);
          } else {
            glyph_index_[charcode] = face->GetCharIndex(maccode);
          }
        }
      }
      if ((glyph_index_[charcode] != 0 && glyph_index_[charcode] != 0xffff) ||
          !name) {
        continue;
      }
      if (UNSAFE_TODO(strcmp(name, kNotDef)) == 0) {
        glyph_index_[charcode] = face->GetCharIndex(32);
        continue;
      }
      glyph_index_[charcode] = face->GetNameIndex(name);
      if (glyph_index_[charcode] != 0 || !bToUnicode) {
        continue;
      }

      WideString wsUnicode = UnicodeFromCharCode(charcode);
      if (!wsUnicode.IsEmpty()) {
        glyph_index_[charcode] = face->GetCharIndex(wsUnicode[0]);
        encoding_.SetUnicode(charcode, wsUnicode[0]);
      }
    }
    return;
  }
  if (UseTTCharmap(face, CFX_Face::kWindowsSymbolCmapId)) {
    for (uint32_t charcode = 0; charcode < 256; charcode++) {
      glyph_index_[charcode] = GetGlyphIndexForMSSymbol(face, charcode);
    }
    if (HasAnyGlyphIndex()) {
      if (base_encoding != FontEncoding::kBuiltin) {
        for (uint32_t charcode = 0; charcode < 256; charcode++) {
          const char* name =
              GetAdobeCharName(base_encoding, char_names_, charcode);
          if (name) {
            encoding_.SetUnicode(charcode, UnicodeFromAdobeName(name));
          }
        }
      } else if (UseTTCharmap(face, CFX_Face::kMacRomanCmapId)) {
        for (uint32_t charcode = 0; charcode < 256; charcode++) {
          encoding_.SetUnicode(charcode,
                               UnicodeFromAppleRomanCharCode(charcode));
        }
      }
      return;
    }
  }
  if (UseTTCharmap(face, CFX_Face::kMacRomanCmapId)) {
    for (uint32_t charcode = 0; charcode < 256; charcode++) {
      glyph_index_[charcode] = face->GetCharIndex(charcode);
      encoding_.SetUnicode(charcode, UnicodeFromAppleRomanCharCode(charcode));
    }
    if (font_file_ || HasAnyGlyphIndex()) {
      return;
    }
  }
  if (font_.GetFace()->SelectCharMap(fxge::FontEncoding::kUnicode)) {
    pdfium::span<const uint16_t> unicodes =
        UnicodesForPredefinedCharSet(base_encoding);
    for (uint32_t charcode = 0; charcode < 256; charcode++) {
      if (font_file_) {
        encoding_.SetUnicode(charcode, charcode);
      } else {
        const char* name =
            GetAdobeCharName(FontEncoding::kBuiltin, char_names_, charcode);
        if (name) {
          encoding_.SetUnicode(charcode, UnicodeFromAdobeName(name));
        } else if (!unicodes.empty()) {
          encoding_.SetUnicode(charcode, unicodes[charcode]);
        }
      }
      glyph_index_[charcode] =
          face->GetCharIndex(encoding_.UnicodeFromCharCode(charcode));
    }
    if (HasAnyGlyphIndex()) {
      return;
    }
  }
  for (int charcode = 0; charcode < 256; charcode++) {
    glyph_index_[charcode] = charcode;
  }
}

bool CPDF_TrueTypeFont::HasAnyGlyphIndex() const {
  for (uint32_t charcode = 0; charcode < kInternalTableSize; charcode++) {
    if (glyph_index_[charcode]) {
      return true;
    }
  }
  return false;
}

CPDF_TrueTypeFont::CharmapType CPDF_TrueTypeFont::DetermineCharmapType() const {
  if (UseTTCharmapUnicode(font_.GetFace())) {
    return CharmapType::kMSUnicode;
  }

  if (FontStyleIsNonSymbolic(flags_)) {
    if (UseTTCharmap(font_.GetFace(), CFX_Face::kMacRomanCmapId)) {
      return CharmapType::kMacRoman;
    }
    if (UseTTCharmap(font_.GetFace(), CFX_Face::kWindowsSymbolCmapId)) {
      return CharmapType::kMSSymbol;
    }
  } else {
    if (UseTTCharmap(font_.GetFace(), CFX_Face::kWindowsSymbolCmapId)) {
      return CharmapType::kMSSymbol;
    }
    if (UseTTCharmap(font_.GetFace(), CFX_Face::kMacRomanCmapId)) {
      return CharmapType::kMacRoman;
    }
  }
  return CharmapType::kOther;
}

FontEncoding CPDF_TrueTypeFont::DetermineEncoding() const {
  if (!font_file_ || !FontStyleIsSymbolic(flags_) ||
      !IsWinAnsiOrMacRomanEncoding(base_encoding_)) {
    return base_encoding_;
  }

  // Not null - caller checked.
  RetainPtr<CFX_Face> face = font_.GetFace();
  const size_t num_charmaps = face->GetCharMapCount();
  if (num_charmaps == 0) {
    return base_encoding_;
  }

  bool support_win = false;
  bool support_mac = false;
  for (size_t i = 0; i < num_charmaps; i++) {
    int platform_id = face->GetCharMapPlatformIdByIndex(i);
    if (platform_id == kNamePlatformAppleUnicode ||
        platform_id == kNamePlatformWindows) {
      support_win = true;
    } else if (platform_id == kNamePlatformMac) {
      support_mac = true;
    }
    if (support_win && support_mac) {
      break;
    }
  }

  if (base_encoding_ == FontEncoding::kWinAnsi && !support_win) {
    return support_mac ? FontEncoding::kMacRoman : FontEncoding::kBuiltin;
  }
  if (base_encoding_ == FontEncoding::kMacRoman && !support_mac) {
    return support_win ? FontEncoding::kWinAnsi : FontEncoding::kBuiltin;
  }
  return base_encoding_;
}

void CPDF_TrueTypeFont::SetGlyphIndicesFromFirstChar() {
  int start_char = font_dict_->GetIntegerFor("FirstChar");
  if (start_char < 0 || start_char > 255) {
    return;
  }

  auto it = std::begin(glyph_index_);
  std::fill(it, it + start_char, 0);
  uint16_t glyph = 3;
  for (int charcode = start_char; charcode < 256; charcode++, glyph++) {
    glyph_index_[charcode] = glyph;
  }
}
