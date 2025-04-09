// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/charposlist.h"

#include "build/build_config.h"
#include "core/fpdfapi/font/cpdf_cidfont.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxge/cfx_fontmapper.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/text_char_pos.h"

namespace {

bool ShouldUseExistingFont(const CPDF_Font* font,
                           uint32_t glyph_id,
                           bool has_to_unicode) {
  // Check for invalid glyph ID.
  if (glyph_id == static_cast<uint32_t>(-1)) {
    return false;
  }

  if (font->IsEmbedded()) {
    return true;
  }

  if (!font->IsTrueTypeFont()) {
    return true;
  }

  // For non-embedded TrueType fonts, a glyph ID of 0 may be invalid.
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

// The following is not a perfect solution and can be further improved.
// For example, if `subst_font` is "Book" and the `base_font_name` is "Bookman",
// this function will return "true" even though the actual font "Bookman"
// is not loaded.
// An exact string match is not possible here, because `subst_font_name`
// will be the same value for different postscript names.
// For example: "Times New Roman" as `subst_font_name` for all of these
// `base_font_name` values: "TimesNewRoman,Bold", "TimesNewRomanPS-Bold",
// "TimesNewRomanBold" and "TimesNewRoman-Bold".
bool IsActualFontLoaded(const CFX_SubstFont* subst_font,
                        const ByteString& base_font_name) {
  // Skip if we loaded the actual font.
  // example: TimesNewRoman,Bold -> Times New Roman
  ByteString subst_font_name = subst_font->family_;
  subst_font_name.Remove(' ');
  subst_font_name.MakeLower();

  std::optional<size_t> find =
      base_font_name.Find(subst_font_name.AsStringView());
  return find.has_value() && find.value() == 0;
}

bool ApplyGlyphSpacingHeuristic(const CPDF_Font* font,
                                const CFX_Font* current_font,
                                bool is_vertical_writing) {
  if (is_vertical_writing || font->IsEmbedded() || !font->HasFontWidths()) {
    return false;
  }

  // Skip if we loaded a standard alternate font.
  // example: Helvetica -> Arial
  ByteString base_font_name = font->GetBaseFontName();
  base_font_name.MakeLower();

  auto standard_font_name =
      CFX_FontMapper::GetStandardFontName(&base_font_name);
  if (standard_font_name.has_value()) {
    return false;
  }

  CFX_SubstFont* subst_font = current_font->GetSubstFont();
  if (subst_font->IsBuiltInGenericFont()) {
    return false;
  }

  return !IsActualFontLoaded(subst_font, base_font_name);
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
    if (char_code == static_cast<uint32_t>(-1)) {
      continue;
    }

    bool is_vertical_glyph = false;
    results.emplace_back();
    TextCharPos& text_char_pos = results.back();
    if (cid_font) {
      text_char_pos.font_style_ = true;
    }
    WideString unicode = font->UnicodeFromCharCode(char_code);
    text_char_pos.unicode_ = !unicode.IsEmpty() ? unicode[0] : char_code;
    text_char_pos.glyph_index_ =
        font->GlyphFromCharCode(char_code, &is_vertical_glyph);
    uint32_t glyph_id = text_char_pos.glyph_index_;
#if BUILDFLAG(IS_APPLE)
    text_char_pos.ext_gid_ = font->GlyphFromCharCodeExt(char_code);
    glyph_id = text_char_pos.ext_gid_ != static_cast<uint32_t>(-1)
                   ? text_char_pos.ext_gid_
                   : text_char_pos.glyph_index_;
#endif
    CFX_Font* current_font;
    if (ShouldUseExistingFont(font, glyph_id, has_to_unicode)) {
      current_font = font->GetFont();
      text_char_pos.fallback_font_position_ = -1;
    } else {
      int32_t fallback_position = font->FallbackFontFromCharcode(char_code);
      current_font = font->GetFontFallback(fallback_position);
      text_char_pos.fallback_font_position_ = fallback_position;
      text_char_pos.glyph_index_ =
          font->FallbackGlyphFromCharcode(fallback_position, char_code);
#if BUILDFLAG(IS_APPLE)
      text_char_pos.ext_gid_ = text_char_pos.glyph_index_;
#endif
    }

    if (!font->IsEmbedded() && !font->IsCIDFont()) {
      text_char_pos.font_char_width_ = font->GetCharWidthF(char_code);
    } else {
      text_char_pos.font_char_width_ = 0;
    }

    text_char_pos.origin_ = CFX_PointF(i > 0 ? char_pos[i - 1] : 0, 0);
    text_char_pos.glyph_adjust_ = false;

    float scaling_factor = 1.0f;
    if (ApplyGlyphSpacingHeuristic(font, current_font, is_vertical_writing)) {
      int pdf_glyph_width = font->GetCharWidthF(char_code);
      int font_glyph_width =
          current_font->GetGlyphWidth(text_char_pos.glyph_index_);
      if (font_glyph_width && pdf_glyph_width > font_glyph_width + 1) {
        // Move the initial x position by half of the excess (transformed to
        // text space coordinates).
        text_char_pos.origin_.x +=
            (pdf_glyph_width - font_glyph_width) * font_size / 2000.0f;
      } else if (pdf_glyph_width && font_glyph_width &&
                 pdf_glyph_width < font_glyph_width) {
        scaling_factor = static_cast<float>(pdf_glyph_width) / font_glyph_width;
        text_char_pos.adjust_matrix_[0] = scaling_factor;
        text_char_pos.adjust_matrix_[1] = 0.0f;
        text_char_pos.adjust_matrix_[2] = 0.0f;
        text_char_pos.adjust_matrix_[3] = 1.0f;
        text_char_pos.glyph_adjust_ = true;
      }
    }
    if (!cid_font) {
      continue;
    }

    uint16_t cid = cid_font->CIDFromCharCode(char_code);
    if (is_vertical_writing) {
      text_char_pos.origin_ = CFX_PointF(0, text_char_pos.origin_.x);

      CFX_Point16 vertical_origin = cid_font->GetVertOrigin(cid);
      text_char_pos.origin_.x -= font_size * vertical_origin.x / 1000;
      text_char_pos.origin_.y -= font_size * vertical_origin.y / 1000;
    }

    const CIDTransform* cid_transform = cid_font->GetCIDTransform(cid);
    if (cid_transform && !is_vertical_glyph) {
      text_char_pos.adjust_matrix_[0] =
          cid_font->CIDTransformToFloat(cid_transform->a) * scaling_factor;
      text_char_pos.adjust_matrix_[1] =
          cid_font->CIDTransformToFloat(cid_transform->b) * scaling_factor;
      text_char_pos.adjust_matrix_[2] =
          cid_font->CIDTransformToFloat(cid_transform->c);
      text_char_pos.adjust_matrix_[3] =
          cid_font->CIDTransformToFloat(cid_transform->d);
      text_char_pos.origin_.x +=
          cid_font->CIDTransformToFloat(cid_transform->e) * font_size;
      text_char_pos.origin_.y +=
          cid_font->CIDTransformToFloat(cid_transform->f) * font_size;
      text_char_pos.glyph_adjust_ = true;
    }
  }

  return results;
}
