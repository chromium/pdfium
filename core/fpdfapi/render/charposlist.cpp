// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/charposlist.h"

#include "build/build_config.h"
#include "core/fpdfapi/font/cpdf_cidfont.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/text_char_pos.h"

namespace {

bool ShouldUseExistingFont(const CPDF_Font* font,
                           uint32_t glyph_id,
                           bool has_to_unicode) {
  // Check for invalid glyph ID.
  if (glyph_id == static_cast<uint32_t>(-1))
    return false;

  if (!font->IsTrueTypeFont())
    return true;

  // For TrueType fonts, a glyph ID of 0 may be invalid.
  //
  // When a "ToUnicode" entry exists in the font dictionary, it indicates
  // a "ToUnicode" mapping file is used to convert from CIDs (which
  // begins at decimal 0) to Unicode code. (See ToUnicode Mapping File
  // Tutorial - Adobe
  // https://www.adobe.com/content/dam/acom/en/devnet/acrobat/pdfs/5411.ToUnicode.pdf
  // and
  // https://www.freetype.org/freetype2/docs/tutorial/step1.html#section-6)
  return glyph_id != 0 || has_to_unicode;
}

}  // namespace

std::vector<TextCharPos> GetCharPosList(pdfium::span<const uint32_t> char_codes,
                                        pdfium::span<const float> char_pos,
                                        CPDF_Font* font,
                                        float font_size) {
  std::vector<TextCharPos> results;
  results.reserve(char_codes.size());

  CPDF_CIDFont* cid_font = font->AsCIDFont();
  bool is_vertical_writing = cid_font && cid_font->IsVertWriting();
  bool has_to_unicode = !!font->GetFontDict()->GetStreamFor("ToUnicode");
  for (size_t i = 0; i < char_codes.size(); ++i) {
    uint32_t char_code = char_codes[i];
    if (char_code == static_cast<uint32_t>(-1))
      continue;

    bool is_vertical_glyph = false;
    results.emplace_back();
    TextCharPos& text_char_pos = results.back();
    if (cid_font)
      text_char_pos.m_bFontStyle = true;
    WideString unicode = font->UnicodeFromCharCode(char_code);
    text_char_pos.m_Unicode = !unicode.IsEmpty() ? unicode[0] : char_code;
    text_char_pos.m_GlyphIndex =
        font->GlyphFromCharCode(char_code, &is_vertical_glyph);
    uint32_t glyph_id = text_char_pos.m_GlyphIndex;
#if BUILDFLAG(IS_APPLE)
    text_char_pos.m_ExtGID = font->GlyphFromCharCodeExt(char_code);
    glyph_id = text_char_pos.m_ExtGID != static_cast<uint32_t>(-1)
                   ? text_char_pos.m_ExtGID
                   : text_char_pos.m_GlyphIndex;
#endif
    CFX_Font* current_font;
    if (ShouldUseExistingFont(font, glyph_id, has_to_unicode)) {
      current_font = font->GetFont();
      text_char_pos.m_FallbackFontPosition = -1;
    } else {
      int32_t fallback_position = font->FallbackFontFromCharcode(char_code);
      current_font = font->GetFontFallback(fallback_position);
      text_char_pos.m_FallbackFontPosition = fallback_position;
      text_char_pos.m_GlyphIndex =
          font->FallbackGlyphFromCharcode(fallback_position, char_code);
#if BUILDFLAG(IS_APPLE)
      text_char_pos.m_ExtGID = text_char_pos.m_GlyphIndex;
#endif
    }

    if (!font->IsEmbedded() && !font->IsCIDFont())
      text_char_pos.m_FontCharWidth = font->GetCharWidthF(char_code);
    else
      text_char_pos.m_FontCharWidth = 0;

    text_char_pos.m_Origin = CFX_PointF(i > 0 ? char_pos[i - 1] : 0, 0);
    text_char_pos.m_bGlyphAdjust = false;

    float scaling_factor = 1.0f;
    if (!font->IsEmbedded() && font->HasFontWidths() && !is_vertical_writing &&
        !current_font->GetSubstFont()->m_bFlagMM) {
      int pdf_glyph_width = font->GetCharWidthF(char_code);
      int font_glyph_width =
          current_font->GetGlyphWidth(text_char_pos.m_GlyphIndex);
      if (font_glyph_width && pdf_glyph_width > font_glyph_width + 1) {
        // Move the initial x position by half of the excess (transformed to
        // text space coordinates).
        text_char_pos.m_Origin.x +=
            (pdf_glyph_width - font_glyph_width) * font_size / 2000.0f;
      } else if (pdf_glyph_width && font_glyph_width &&
                 pdf_glyph_width < font_glyph_width) {
        scaling_factor = static_cast<float>(pdf_glyph_width) / font_glyph_width;
        text_char_pos.m_AdjustMatrix[0] = scaling_factor;
        text_char_pos.m_AdjustMatrix[1] = 0.0f;
        text_char_pos.m_AdjustMatrix[2] = 0.0f;
        text_char_pos.m_AdjustMatrix[3] = 1.0f;
        text_char_pos.m_bGlyphAdjust = true;
      }
    }
    if (!cid_font)
      continue;

    uint16_t cid = cid_font->CIDFromCharCode(char_code);
    if (is_vertical_writing) {
      text_char_pos.m_Origin = CFX_PointF(0, text_char_pos.m_Origin.x);

      CFX_Point16 vertical_origin = cid_font->GetVertOrigin(cid);
      text_char_pos.m_Origin.x -= font_size * vertical_origin.x / 1000;
      text_char_pos.m_Origin.y -= font_size * vertical_origin.y / 1000;
    }

    const uint8_t* cid_transform = cid_font->GetCIDTransform(cid);
    if (cid_transform && !is_vertical_glyph) {
      text_char_pos.m_AdjustMatrix[0] =
          cid_font->CIDTransformToFloat(cid_transform[0]) * scaling_factor;
      text_char_pos.m_AdjustMatrix[1] =
          cid_font->CIDTransformToFloat(cid_transform[1]) * scaling_factor;
      text_char_pos.m_AdjustMatrix[2] =
          cid_font->CIDTransformToFloat(cid_transform[2]);
      text_char_pos.m_AdjustMatrix[3] =
          cid_font->CIDTransformToFloat(cid_transform[3]);
      text_char_pos.m_Origin.x +=
          cid_font->CIDTransformToFloat(cid_transform[4]) * font_size;
      text_char_pos.m_Origin.y +=
          cid_font->CIDTransformToFloat(cid_transform[5]) * font_size;
      text_char_pos.m_bGlyphAdjust = true;
    }
  }

  return results;
}
