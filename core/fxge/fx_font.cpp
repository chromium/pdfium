// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/fx_font.h"

#include <algorithm>

#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/cfx_glyphbitmap.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/text_glyph_pos.h"

namespace {

ByteString GetStringFromTable(const uint8_t* string_ptr,
                              uint32_t string_ptr_length,
                              uint16_t offset,
                              uint16_t length) {
  if (string_ptr_length < static_cast<uint32_t>(offset + length)) {
    return ByteString();
  }
  return ByteString(string_ptr + offset, length);
}

}  // namespace

FX_RECT GetGlyphsBBox(const std::vector<TextGlyphPos>& glyphs, int anti_alias) {
  FX_RECT rect;
  bool bStarted = false;
  for (const TextGlyphPos& glyph : glyphs) {
    if (!glyph.m_pGlyph)
      continue;

    Optional<CFX_Point> point = glyph.GetOrigin({0, 0});
    if (!point.has_value())
      continue;

    int char_width = glyph.m_pGlyph->GetBitmap()->GetWidth();
    if (anti_alias == FXFT_RENDER_MODE_LCD)
      char_width /= 3;

    FX_SAFE_INT32 char_right = point.value().x;
    char_right += char_width;
    if (!char_right.IsValid())
      continue;

    FX_SAFE_INT32 char_bottom = point.value().y;
    char_bottom += glyph.m_pGlyph->GetBitmap()->GetHeight();
    if (!char_bottom.IsValid())
      continue;

    if (bStarted) {
      rect.left = std::min(rect.left, point.value().x);
      rect.top = std::min(rect.top, point.value().y);
      rect.right = pdfium::base::ValueOrDieForType<int32_t>(
          pdfium::base::CheckMax(rect.right, char_right));
      rect.bottom = pdfium::base::ValueOrDieForType<int32_t>(
          pdfium::base::CheckMax(rect.bottom, char_bottom));
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

ByteString GetNameFromTT(const uint8_t* name_table,
                         uint32_t name_table_size,
                         uint32_t name_id) {
  if (!name_table || name_table_size < 6) {
    return ByteString();
  }
  uint32_t name_count = GET_TT_SHORT(name_table + 2);
  uint32_t string_offset = GET_TT_SHORT(name_table + 4);
  // We will ignore the possibility of overlap of structures and
  // string table as if it's all corrupt there's not a lot we can do.
  if (name_table_size < string_offset) {
    return ByteString();
  }

  const uint8_t* string_ptr = name_table + string_offset;
  uint32_t string_ptr_size = name_table_size - string_offset;
  name_table += 6;
  name_table_size -= 6;
  if (name_table_size < name_count * 12) {
    return ByteString();
  }

  for (uint32_t i = 0; i < name_count; i++, name_table += 12) {
    if (GET_TT_SHORT(name_table + 6) == name_id &&
        GET_TT_SHORT(name_table) == 1 && GET_TT_SHORT(name_table + 2) == 0) {
      return GetStringFromTable(string_ptr, string_ptr_size,
                                GET_TT_SHORT(name_table + 10),
                                GET_TT_SHORT(name_table + 8));
    }
  }
  return ByteString();
}

wchar_t PDF_UnicodeFromAdobeName(const char* name) {
  return (wchar_t)(FXFT_unicode_from_adobe_name(name) & 0x7FFFFFFF);
}

ByteString PDF_AdobeNameFromUnicode(wchar_t unicode) {
  char glyph_name[64];
  FXFT_adobe_name_from_unicode(glyph_name, unicode);
  return ByteString(glyph_name);
}
