// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/fx_font.h"

#include <stdint.h>

#include <algorithm>

#include "core/fxcrt/byteorder.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/widestring.h"
#include "core/fxge/cfx_glyphbitmap.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/freetype/fx_freetype.h"
#include "core/fxge/text_glyph_pos.h"

namespace {

// These numbers come from the OpenType name table specification.
constexpr uint16_t kNameMacEncodingRoman = 0;
constexpr uint16_t kNameWindowsEncodingUnicode = 1;

ByteString GetStringFromTable(pdfium::span<const uint8_t> string_span,
                              uint16_t offset,
                              uint16_t length) {
  if (string_span.size() < static_cast<uint32_t>(offset + length)) {
    return ByteString();
  }

  return ByteString(ByteStringView(string_span.subspan(offset, length)));
}

}  // namespace

FX_RECT GetGlyphsBBox(const std::vector<TextGlyphPos>& glyphs, int anti_alias) {
  FX_RECT rect;
  bool bStarted = false;
  for (const TextGlyphPos& glyph : glyphs) {
    if (!glyph.glyph_) {
      continue;
    }

    std::optional<CFX_Point> point = glyph.GetOrigin({0, 0});
    if (!point.has_value()) {
      continue;
    }

    int char_width = glyph.glyph_->GetBitmap()->GetWidth();
    if (anti_alias == FT_RENDER_MODE_LCD) {
      char_width /= 3;
    }

    FX_SAFE_INT32 char_right = point.value().x;
    char_right += char_width;
    if (!char_right.IsValid()) {
      continue;
    }

    FX_SAFE_INT32 char_bottom = point.value().y;
    char_bottom += glyph.glyph_->GetBitmap()->GetHeight();
    if (!char_bottom.IsValid()) {
      continue;
    }

    if (bStarted) {
      rect.left = std::min(rect.left, point.value().x);
      rect.top = std::min(rect.top, point.value().y);
      rect.right = pdfium::ValueOrDieForType<int32_t>(
          pdfium::CheckMax(rect.right, char_right));
      rect.bottom = pdfium::ValueOrDieForType<int32_t>(
          pdfium::CheckMax(rect.bottom, char_bottom));
      continue;
    }

    rect.left = point.value().x;
    rect.top = point.value().y;
    rect.right = char_right.ValueOrDie();
    rect.bottom = char_bottom.ValueOrDie();
    bStarted = true;
  }
  return rect;
}

ByteString GetNameFromTT(pdfium::span<const uint8_t> name_table,
                         uint32_t name_id) {
  if (name_table.size() < 6) {
    return ByteString();
  }

  uint32_t name_count = fxcrt::GetUInt16MSBFirst(name_table.subspan<2u, 2u>());
  uint32_t string_offset =
      fxcrt::GetUInt16MSBFirst(name_table.subspan<4u, 2u>());
  // We will ignore the possibility of overlap of structures and
  // string table as if it's all corrupt there's not a lot we can do.
  if (name_table.size() < string_offset) {
    return ByteString();
  }

  pdfium::span<const uint8_t> string_span = name_table.subspan(string_offset);
  name_table = name_table.subspan<6u>();
  if (name_table.size() < name_count * 12) {
    return ByteString();
  }

  for (uint32_t i = 0; i < name_count;
       i++, name_table = name_table.subspan<12u>()) {
    if (fxcrt::GetUInt16MSBFirst(name_table.subspan<6u, 2u>()) == name_id) {
      const uint16_t platform_identifier =
          fxcrt::GetUInt16MSBFirst(name_table.first<2u>());
      const uint16_t platform_encoding =
          fxcrt::GetUInt16MSBFirst(name_table.subspan<2u, 2u>());

      if (platform_identifier == kNamePlatformMac &&
          platform_encoding == kNameMacEncodingRoman) {
        return GetStringFromTable(
            string_span,
            fxcrt::GetUInt16MSBFirst(name_table.subspan<10u, 2u>()),
            fxcrt::GetUInt16MSBFirst(name_table.subspan<8u, 2u>()));
      }
      if (platform_identifier == kNamePlatformWindows &&
          platform_encoding == kNameWindowsEncodingUnicode) {
        // This name is always UTF16-BE and we have to convert it to UTF8.
        ByteString utf16_be = GetStringFromTable(
            string_span,
            fxcrt::GetUInt16MSBFirst(name_table.subspan<10u, 2u>()),
            fxcrt::GetUInt16MSBFirst(name_table.subspan<8u, 2u>()));
        if (utf16_be.IsEmpty() || utf16_be.GetLength() % 2 != 0) {
          return ByteString();
        }

        return WideString::FromUTF16BE(utf16_be.unsigned_span()).ToUTF8();
      }
    }
  }
  return ByteString();
}

size_t GetTTCIndex(pdfium::span<const uint8_t> font_data, size_t font_offset) {
  pdfium::span<const uint8_t> p = font_data.subspan<8u>();
  size_t nfont = fxcrt::GetUInt32MSBFirst(p.first<4u>());
  for (size_t index = 0; index < nfont; index++) {
    p = font_data.subspan(12 + index * 4);
    if (fxcrt::GetUInt32MSBFirst(p.first<4u>()) == font_offset) {
      return index;
    }
  }
  return 0;
}

wchar_t UnicodeFromAdobeName(const char* name) {
  return (wchar_t)(FXFT_unicode_from_adobe_name(name) & 0x7FFFFFFF);
}

ByteString AdobeNameFromUnicode(wchar_t unicode) {
  char glyph_name[64];
  FXFT_adobe_name_from_unicode(glyph_name, unicode);
  return ByteString(glyph_name);
}

int NormalizeFontMetric(int64_t value, uint16_t upem) {
  if (upem == 0) {
    return pdfium::saturated_cast<int>(value);
  }

  const double scaled_value = (value * 1000.0 + upem / 2) / upem;
  return pdfium::saturated_cast<int>(scaled_value);
}
