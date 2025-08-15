// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_type1font.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <utility>

#include "build/build_config.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/span_util.h"
#include "core/fxge/cfx_fontmapper.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/freetype/fx_freetype.h"
#include "core/fxge/fx_font.h"

#if BUILDFLAG(IS_APPLE)
#include <CoreFoundation/CFString.h>
#include <CoreGraphics/CoreGraphics.h>
#endif  // BUILDFLAG(IS_APPLE)

namespace {

#if BUILDFLAG(IS_APPLE)
struct GlyphNameMap {
  const char* str_adobe_;    // Raw, POD struct.
  const char* str_unicode_;  // Raw, POD struct.
};

const GlyphNameMap kGlyphNameSubsts[] = {{"ff", "uniFB00"},
                                         {"ffi", "uniFB03"},
                                         {"ffl", "uniFB04"},
                                         {"fi", "uniFB01"},
                                         {"fl", "uniFB02"}};

const char* GlyphNameRemap(const char* pStrAdobe) {
  for (const auto& element : kGlyphNameSubsts) {
    if (!FXSYS_stricmp(element.str_adobe_, pStrAdobe)) {
      return element.str_unicode_;
    }
  }
  return nullptr;
}

#endif  // BUILDFLAG(IS_APPLE)

bool UseType1Charmap(const RetainPtr<CFX_Face>& face) {
  size_t num_charmaps = face->GetCharMapCount();
  if (num_charmaps == 0) {
    return false;
  }

  bool is_first_charmap_unicode =
      face->GetCharMapEncodingByIndex(0) == fxge::FontEncoding::kUnicode;
  if (num_charmaps == 1 && is_first_charmap_unicode) {
    return false;
  }

  int index = is_first_charmap_unicode ? 1 : 0;
  face->SetCharMapByIndex(index);
  return true;
}

}  // namespace

CPDF_Type1Font::CPDF_Type1Font(CPDF_Document* document,
                               RetainPtr<CPDF_Dictionary> font_dict)
    : CPDF_SimpleFont(document, std::move(font_dict)) {
#if BUILDFLAG(IS_APPLE)
  ext_gid_.fill(0xffff);
#endif
}

CPDF_Type1Font::~CPDF_Type1Font() = default;

bool CPDF_Type1Font::IsType1Font() const {
  return true;
}

const CPDF_Type1Font* CPDF_Type1Font::AsType1Font() const {
  return this;
}

CPDF_Type1Font* CPDF_Type1Font::AsType1Font() {
  return this;
}

bool CPDF_Type1Font::Load() {
  base14_font_ = CFX_FontMapper::GetStandardFontName(&base_font_name_);
  if (!IsBase14Font()) {
    return LoadCommon();
  }

  RetainPtr<const CPDF_Dictionary> font_desc =
      font_dict_->GetDictFor("FontDescriptor");
  if (font_desc && font_desc->KeyExist("Flags")) {
    flags_ = font_desc->GetIntegerFor("Flags");
  } else if (IsSymbolicFont()) {
    flags_ = pdfium::kFontStyleSymbolic;
  } else {
    flags_ = pdfium::kFontStyleNonSymbolic;
  }
  if (IsFixedFont()) {
    std::fill(std::begin(char_width_), std::end(char_width_), 600);
  }
  if (base14_font_ == CFX_FontMapper::kSymbol) {
    base_encoding_ = FontEncoding::kAdobeSymbol;
  } else if (base14_font_ == CFX_FontMapper::kDingbats) {
    base_encoding_ = FontEncoding::kZapfDingbats;
  } else if (FontStyleIsNonSymbolic(flags_)) {
    base_encoding_ = FontEncoding::kStandard;
  }
  return LoadCommon();
}

#if BUILDFLAG(IS_APPLE)
int CPDF_Type1Font::GlyphFromCharCodeExt(uint32_t charcode) {
  if (charcode > 0xff) {
    return -1;
  }

  int index = ext_gid_[static_cast<uint8_t>(charcode)];
  return index != 0xffff ? index : -1;
}
#endif

void CPDF_Type1Font::LoadGlyphMap() {
  RetainPtr<CFX_Face> face = font_.GetFace();
  if (!face) {
    return;
  }

#if BUILDFLAG(IS_APPLE)
  bool bCoreText = true;
  if (!font_.GetPlatformFont()) {
    if (font_.GetPsName() == "DFHeiStd-W5") {
      bCoreText = false;
    }

    auto* pPlatform = CFX_GEModule::Get()->GetPlatform();
    pdfium::span<const uint8_t> span = font_.GetFontSpan();
    font_.SetPlatformFont(pPlatform->CreatePlatformFont(span));
    if (!font_.GetPlatformFont()) {
      bCoreText = false;
    }
  }
#endif
  if (!IsEmbedded() && !IsSymbolicFont() && font_.IsTTFont()) {
    if (UseTTCharmap(face, CFX_Face::kWindowsSymbolCmapId)) {
      bool bGotOne = false;
      for (uint32_t charcode = 0; charcode < kInternalTableSize; charcode++) {
        static constexpr std::array<uint8_t, 4> prefix = {
            {0x00, 0xf0, 0xf1, 0xf2}};
        for (int j = 0; j < 4; j++) {
          uint16_t unicode = prefix[j] * 256 + charcode;
          glyph_index_[charcode] = face->GetCharIndex(unicode);
#if BUILDFLAG(IS_APPLE)
          CalcExtGID(charcode);
#endif
          if (glyph_index_[charcode]) {
            bGotOne = true;
            break;
          }
        }
      }
      if (bGotOne) {
#if BUILDFLAG(IS_APPLE)
        if (!bCoreText) {
          ext_gid_ = glyph_index_;
        }
#endif
        return;
      }
    }
    face->SelectCharMap(fxge::FontEncoding::kUnicode);
    if (base_encoding_ == FontEncoding::kBuiltin) {
      base_encoding_ = FontEncoding::kStandard;
    }

    for (uint32_t charcode = 0; charcode < kInternalTableSize; charcode++) {
      const char* name =
          GetAdobeCharName(base_encoding_, char_names_, charcode);
      if (!name) {
        continue;
      }

      encoding_.SetUnicode(charcode, UnicodeFromAdobeName(name));
      glyph_index_[charcode] =
          face->GetCharIndex(encoding_.UnicodeFromCharCode(charcode));
#if BUILDFLAG(IS_APPLE)
      CalcExtGID(charcode);
#endif
      if (glyph_index_[charcode] == 0 &&
          UNSAFE_TODO(strcmp(name, kNotDef)) == 0) {
        encoding_.SetUnicode(charcode, 0x20);
        glyph_index_[charcode] = face->GetCharIndex(0x20);
#if BUILDFLAG(IS_APPLE)
        CalcExtGID(charcode);
#endif
      }
    }
#if BUILDFLAG(IS_APPLE)
    if (!bCoreText) {
      ext_gid_ = glyph_index_;
    }
#endif
    return;
  }
  UseType1Charmap(face);
#if BUILDFLAG(IS_APPLE)
  if (bCoreText) {
    if (FontStyleIsSymbolic(flags_)) {
      for (uint32_t charcode = 0; charcode < kInternalTableSize; charcode++) {
        const char* name =
            GetAdobeCharName(base_encoding_, char_names_, charcode);
        if (name) {
          encoding_.SetUnicode(charcode, UnicodeFromAdobeName(name));
          glyph_index_[charcode] = font_.GetFace()->GetNameIndex(name);
          SetExtGID(name, charcode);
        } else {
          glyph_index_[charcode] = face->GetCharIndex(charcode);
          ByteString glyph_name = face->GetGlyphName(glyph_index_[charcode]);
          const wchar_t unicode =
              glyph_name.IsEmpty() ? 0
                                   : UnicodeFromAdobeName(glyph_name.c_str());
          encoding_.SetUnicode(charcode, unicode);
          SetExtGID(glyph_name.c_str(), charcode);
        }
      }
      return;
    }

    bool bUnicode = face->SelectCharMap(fxge::FontEncoding::kUnicode);
    for (uint32_t charcode = 0; charcode < kInternalTableSize; charcode++) {
      const char* name =
          GetAdobeCharName(base_encoding_, char_names_, charcode);
      if (!name) {
        continue;
      }

      encoding_.SetUnicode(charcode, UnicodeFromAdobeName(name));
      const char* pStrUnicode = GlyphNameRemap(name);
      int name_index = font_.GetFace()->GetNameIndex(name);
      if (pStrUnicode && name_index == 0) {
        name = pStrUnicode;
      }
      glyph_index_[charcode] = name_index;
      SetExtGID(name, charcode);
      if (glyph_index_[charcode] != 0) {
        continue;
      }

      if (UNSAFE_TODO(strcmp(name, kNotDef)) != 0 &&
          UNSAFE_TODO(strcmp(name, kSpace)) != 0) {
        glyph_index_[charcode] = face->GetCharIndex(
            bUnicode ? encoding_.UnicodeFromCharCode(charcode) : charcode);
        CalcExtGID(charcode);
      } else {
        encoding_.SetUnicode(charcode, 0x20);
        glyph_index_[charcode] = bUnicode ? face->GetCharIndex(0x20) : 0xffff;
        CalcExtGID(charcode);
      }
    }
    return;
  }
#endif  // BUILDFLAG(IS_APPLE)
  if (FontStyleIsSymbolic(flags_)) {
    for (size_t charcode = 0; charcode < kInternalTableSize; charcode++) {
      const char* name = GetAdobeCharName(base_encoding_, char_names_,
                                          static_cast<uint32_t>(charcode));
      if (name) {
        encoding_.SetUnicode(charcode, UnicodeFromAdobeName(name));
        glyph_index_[charcode] = font_.GetFace()->GetNameIndex(name);
      } else {
        glyph_index_[charcode] =
            face->GetCharIndex(static_cast<uint32_t>(charcode));
        if (glyph_index_[charcode]) {
          ByteString glyph_name = face->GetGlyphName(glyph_index_[charcode]);
          const wchar_t unicode =
              glyph_name.IsEmpty() ? 0
                                   : UnicodeFromAdobeName(glyph_name.c_str());
          encoding_.SetUnicode(charcode, unicode);
        }
      }
    }
#if BUILDFLAG(IS_APPLE)
    if (!bCoreText) {
      ext_gid_ = glyph_index_;
    }
#endif
    return;
  }

  bool bUnicode = face->SelectCharMap(fxge::FontEncoding::kUnicode);
  for (size_t charcode = 0; charcode < kInternalTableSize; charcode++) {
    const char* name = GetAdobeCharName(base_encoding_, char_names_,
                                        static_cast<uint32_t>(charcode));
    if (!name) {
      continue;
    }

    encoding_.SetUnicode(charcode, UnicodeFromAdobeName(name));
    glyph_index_[charcode] = font_.GetFace()->GetNameIndex(name);
    if (glyph_index_[charcode] != 0) {
      continue;
    }

    if (UNSAFE_TODO(strcmp(name, kNotDef)) != 0 &&
        UNSAFE_TODO(strcmp(name, kSpace)) != 0) {
      glyph_index_[charcode] =
          face->GetCharIndex(bUnicode ? encoding_.UnicodeFromCharCode(charcode)
                                      : static_cast<uint32_t>(charcode));
    } else {
      encoding_.SetUnicode(charcode, 0x20);
      glyph_index_[charcode] = 0xffff;
    }
  }
#if BUILDFLAG(IS_APPLE)
  if (!bCoreText) {
    ext_gid_ = glyph_index_;
  }
#endif
}

bool CPDF_Type1Font::IsSymbolicFont() const {
  return base14_font_.has_value() &&
         CFX_FontMapper::IsSymbolicFont(base14_font_.value());
}

bool CPDF_Type1Font::IsFixedFont() const {
  return base14_font_.has_value() &&
         CFX_FontMapper::IsFixedFont(base14_font_.value());
}

#if BUILDFLAG(IS_APPLE)
void CPDF_Type1Font::SetExtGID(const char* name, uint32_t charcode) {
  CFStringRef name_ct = CFStringCreateWithCStringNoCopy(
      kCFAllocatorDefault, name, kCFStringEncodingASCII, kCFAllocatorNull);
  ext_gid_[charcode] =
      CGFontGetGlyphWithGlyphName((CGFontRef)font_.GetPlatformFont(), name_ct);
  if (name_ct) {
    CFRelease(name_ct);
  }
}

void CPDF_Type1Font::CalcExtGID(uint32_t charcode) {
  ByteString glyph_name = font_.GetFace()->GetGlyphName(glyph_index_[charcode]);
  SetExtGID(glyph_name.c_str(), charcode);
}
#endif  // BUILDFLAG(IS_APPLE)
