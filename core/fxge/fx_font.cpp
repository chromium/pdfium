// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/fx_font.h"

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
  FX_RECT rect(0, 0, 0, 0);
  bool bStarted = false;
  for (const TextGlyphPos& glyph : glyphs) {
    const CFX_GlyphBitmap* pGlyph = glyph.m_pGlyph;
    if (!pGlyph)
      continue;

    FX_SAFE_INT32 char_left = glyph.m_Origin.x;
    char_left += pGlyph->left();
    if (!char_left.IsValid())
      continue;

    FX_SAFE_INT32 char_width = pGlyph->GetBitmap()->GetWidth();
    if (anti_alias == FXFT_RENDER_MODE_LCD)
      char_width /= 3;
    if (!char_width.IsValid())
      continue;

    FX_SAFE_INT32 char_right = char_left + char_width;
    if (!char_right.IsValid())
      continue;

    FX_SAFE_INT32 char_top = glyph.m_Origin.y;
    char_top -= pGlyph->top();
    if (!char_top.IsValid())
      continue;

    FX_SAFE_INT32 char_height = pGlyph->GetBitmap()->GetHeight();
    if (!char_height.IsValid())
      continue;

    FX_SAFE_INT32 char_bottom = char_top + char_height;
    if (!char_bottom.IsValid())
      continue;

    if (bStarted) {
      rect.left = pdfium::base::ValueOrDieForType<int32_t>(
          pdfium::base::CheckMin(rect.left, char_left));
      rect.right = pdfium::base::ValueOrDieForType<int32_t>(
          pdfium::base::CheckMax(rect.right, char_right));
      rect.top = pdfium::base::ValueOrDieForType<int32_t>(
          pdfium::base::CheckMin(rect.top, char_top));
      rect.bottom = pdfium::base::ValueOrDieForType<int32_t>(
          pdfium::base::CheckMax(rect.bottom, char_bottom));
      continue;
    }

    rect.left = char_left.ValueOrDie();
    rect.right = char_right.ValueOrDie();
    rect.top = char_top.ValueOrDie();
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
