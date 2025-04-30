// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_simplefont.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <utility>

#include "constants/font_encodings.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxge/freetype/fx_freetype.h"
#include "core/fxge/fx_font.h"

namespace {

void GetPredefinedEncoding(const ByteString& value, FontEncoding* basemap) {
  if (value == pdfium::font_encodings::kWinAnsiEncoding) {
    *basemap = FontEncoding::kWinAnsi;
  } else if (value == pdfium::font_encodings::kMacRomanEncoding) {
    *basemap = FontEncoding::kMacRoman;
  } else if (value == pdfium::font_encodings::kMacExpertEncoding) {
    *basemap = FontEncoding::kMacExpert;
  } else if (value == pdfium::font_encodings::kPDFDocEncoding) {
    *basemap = FontEncoding::kPdfDoc;
  }
}

}  // namespace

CPDF_SimpleFont::CPDF_SimpleFont(CPDF_Document* pDocument,
                                 RetainPtr<CPDF_Dictionary> pFontDict)
    : CPDF_Font(pDocument, std::move(pFontDict)) {
  char_width_.fill(0xffff);
  glyph_index_.fill(0xffff);
  char_bbox_.fill(FX_RECT(-1, -1, -1, -1));
}

CPDF_SimpleFont::~CPDF_SimpleFont() = default;

int CPDF_SimpleFont::GlyphFromCharCode(uint32_t charcode, bool* pVertGlyph) {
  if (pVertGlyph) {
    *pVertGlyph = false;
  }

  if (charcode > 0xff) {
    return -1;
  }

  int index = glyph_index_[charcode];
  if (index == 0xffff) {
    return -1;
  }

  return index;
}

void CPDF_SimpleFont::LoadCharMetrics(int charcode) {
  if (!font_.GetFaceRec()) {
    return;
  }

  if (charcode < 0 || charcode > 0xff) {
    return;
  }
  int glyph_index = glyph_index_[charcode];
  if (glyph_index == 0xffff) {
    if (!font_file_ && charcode != 32) {
      LoadCharMetrics(32);
      char_bbox_[charcode] = char_bbox_[32];
      if (use_font_width_) {
        char_width_[charcode] = char_width_[32];
      }
    }
    return;
  }
  RetainPtr<CFX_Face> face = font_.GetFace();
  if (!face) {
    return;
  }

  FXFT_FaceRec* face_rec = face->GetRec();
  int err =
      FT_Load_Glyph(face_rec, glyph_index,
                    FT_LOAD_NO_SCALE | FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
  if (err) {
    return;
  }

  char_bbox_[charcode] = face->GetGlyphBBox();

  if (use_font_width_) {
    int TT_Width = NormalizeFontMetric(FXFT_Get_Glyph_HoriAdvance(face_rec),
                                       face->GetUnitsPerEm());
    if (char_width_[charcode] == 0xffff) {
      char_width_[charcode] = TT_Width;
    } else if (TT_Width && !IsEmbedded()) {
      char_bbox_[charcode].right =
          char_bbox_[charcode].right * char_width_[charcode] / TT_Width;
      char_bbox_[charcode].left =
          char_bbox_[charcode].left * char_width_[charcode] / TT_Width;
    }
  }
}

void CPDF_SimpleFont::LoadCharWidths(const CPDF_Dictionary* font_desc) {
  RetainPtr<const CPDF_Array> width_array = font_dict_->GetArrayFor("Widths");
  use_font_width_ = !width_array;
  if (!width_array) {
    return;
  }

  if (font_desc && font_desc->KeyExist("MissingWidth")) {
    int missing_width = font_desc->GetIntegerFor("MissingWidth");
    std::fill(std::begin(char_width_), std::end(char_width_), missing_width);
  }

  size_t width_start = font_dict_->GetIntegerFor("FirstChar", 0);
  size_t width_end = font_dict_->GetIntegerFor("LastChar", 0);
  if (width_start > 255) {
    return;
  }

  if (width_end == 0 || width_end >= width_start + width_array->size()) {
    width_end = width_start + width_array->size() - 1;
  }
  if (width_end > 255) {
    width_end = 255;
  }
  for (size_t i = width_start; i <= width_end; i++) {
    char_width_[i] = width_array->GetIntegerAt(i - width_start);
  }
}

void CPDF_SimpleFont::LoadDifferences(const CPDF_Dictionary* encoding) {
  RetainPtr<const CPDF_Array> diffs = encoding->GetArrayFor("Differences");
  if (!diffs) {
    return;
  }

  char_names_.resize(kInternalTableSize);
  uint32_t cur_code = 0;
  for (uint32_t i = 0; i < diffs->size(); i++) {
    RetainPtr<const CPDF_Object> element = diffs->GetDirectObjectAt(i);
    if (!element) {
      continue;
    }

    const CPDF_Name* name = element->AsName();
    if (name) {
      if (cur_code < char_names_.size()) {
        char_names_[cur_code] = name->GetString();
      }
      cur_code++;
    } else {
      cur_code = element->GetInteger();
    }
  }
}

void CPDF_SimpleFont::LoadPDFEncoding(bool bEmbedded, bool bTrueType) {
  RetainPtr<const CPDF_Object> pEncoding =
      font_dict_->GetDirectObjectFor("Encoding");
  if (!pEncoding) {
    if (base_font_name_ == "Symbol") {
      base_encoding_ =
          bTrueType ? FontEncoding::kMsSymbol : FontEncoding::kAdobeSymbol;
    } else if (!bEmbedded && base_encoding_ == FontEncoding::kBuiltin) {
      base_encoding_ = FontEncoding::kWinAnsi;
    }
    return;
  }
  if (pEncoding->IsName()) {
    if (base_encoding_ == FontEncoding::kAdobeSymbol ||
        base_encoding_ == FontEncoding::kZapfDingbats) {
      return;
    }
    if (FontStyleIsSymbolic(flags_) && base_font_name_ == "Symbol") {
      if (!bTrueType) {
        base_encoding_ = FontEncoding::kAdobeSymbol;
      }
      return;
    }
    ByteString bsEncoding = pEncoding->GetString();
    if (bsEncoding == pdfium::font_encodings::kMacExpertEncoding) {
      bsEncoding = pdfium::font_encodings::kWinAnsiEncoding;
    }
    GetPredefinedEncoding(bsEncoding, &base_encoding_);
    return;
  }

  const CPDF_Dictionary* pDict = pEncoding->AsDictionary();
  if (!pDict) {
    return;
  }

  if (base_encoding_ != FontEncoding::kAdobeSymbol &&
      base_encoding_ != FontEncoding::kZapfDingbats) {
    ByteString bsEncoding = pDict->GetByteStringFor("BaseEncoding");
    if (bTrueType && bsEncoding == pdfium::font_encodings::kMacExpertEncoding) {
      bsEncoding = pdfium::font_encodings::kWinAnsiEncoding;
    }
    GetPredefinedEncoding(bsEncoding, &base_encoding_);
  }
  if ((!bEmbedded || bTrueType) && base_encoding_ == FontEncoding::kBuiltin) {
    base_encoding_ = FontEncoding::kStandard;
  }

  LoadDifferences(pDict);
}

int CPDF_SimpleFont::GetCharWidthF(uint32_t charcode) {
  if (charcode > 0xff) {
    charcode = 0;
  }

  if (char_width_[charcode] == 0xffff) {
    LoadCharMetrics(charcode);
    if (char_width_[charcode] == 0xffff) {
      char_width_[charcode] = 0;
    }
  }
  return char_width_[charcode];
}

FX_RECT CPDF_SimpleFont::GetCharBBox(uint32_t charcode) {
  if (charcode > 0xff) {
    charcode = 0;
  }

  if (char_bbox_[charcode].left == -1) {
    LoadCharMetrics(charcode);
  }

  return char_bbox_[charcode];
}

bool CPDF_SimpleFont::LoadCommon() {
  RetainPtr<const CPDF_Dictionary> pFontDesc =
      font_dict_->GetDictFor("FontDescriptor");
  if (pFontDesc) {
    LoadFontDescriptor(pFontDesc.Get());
  }
  LoadCharWidths(pFontDesc.Get());
  if (font_file_) {
    if (base_font_name_.GetLength() > 7 && base_font_name_[6] == '+') {
      base_font_name_ = base_font_name_.Last(base_font_name_.GetLength() - 7);
    }
  } else {
    LoadSubstFont();
  }
  if (!FontStyleIsSymbolic(flags_)) {
    base_encoding_ = FontEncoding::kStandard;
  }
  LoadPDFEncoding(!!font_file_, font_.IsTTFont());
  LoadGlyphMap();
  char_names_.clear();
  if (!HasFace()) {
    return true;
  }

  if (FontStyleIsAllCaps(flags_)) {
    static const auto kLowercases =
        std::to_array<std::pair<const uint8_t, const uint8_t>>(
            {{'a', 'z'}, {0xe0, 0xf6}, {0xf8, 0xfd}});
    for (const auto& lower : kLowercases) {
      for (int i = lower.first; i <= lower.second; ++i) {
        if (glyph_index_[i] != 0xffff && font_file_) {
          continue;
        }
        int j = i - 32;
        glyph_index_[i] = glyph_index_[j];
        if (char_width_[j]) {
          char_width_[i] = char_width_[j];
          char_bbox_[i] = char_bbox_[j];
        }
      }
    }
  }
  CheckFontMetrics();
  return true;
}

void CPDF_SimpleFont::LoadSubstFont() {
  if (!use_font_width_ && !FontStyleIsFixedPitch(flags_)) {
    int width = 0;
    size_t i;
    for (i = 0; i < kInternalTableSize; i++) {
      if (char_width_[i] == 0 || char_width_[i] == 0xffff) {
        continue;
      }

      if (width == 0) {
        width = char_width_[i];
      } else if (width != char_width_[i]) {
        break;
      }
    }
    if (i == kInternalTableSize && width) {
      flags_ |= pdfium::kFontStyleFixedPitch;
    }
  }

  int weight = GetFontWeight().value_or(pdfium::kFontWeightNormal);
  if (weight < pdfium::kFontWeightExtraLight ||
      weight > pdfium::kFontWeightExtraBold) {
    weight = pdfium::kFontWeightNormal;
  }
  font_.LoadSubst(base_font_name_, IsTrueTypeFont(), flags_, weight,
                  italic_angle_, FX_CodePage::kDefANSI, /*bVertical=*/false);
}

bool CPDF_SimpleFont::IsUnicodeCompatible() const {
  return base_encoding_ != FontEncoding::kBuiltin &&
         base_encoding_ != FontEncoding::kAdobeSymbol &&
         base_encoding_ != FontEncoding::kZapfDingbats;
}

WideString CPDF_SimpleFont::UnicodeFromCharCode(uint32_t charcode) const {
  WideString unicode = CPDF_Font::UnicodeFromCharCode(charcode);
  if (!unicode.IsEmpty()) {
    return unicode;
  }
  wchar_t ret = encoding_.UnicodeFromCharCode((uint8_t)charcode);
  if (ret == 0) {
    return WideString();
  }
  return WideString(ret);
}

uint32_t CPDF_SimpleFont::CharCodeFromUnicode(wchar_t unicode) const {
  uint32_t ret = CPDF_Font::CharCodeFromUnicode(unicode);
  if (ret) {
    return ret;
  }
  return encoding_.CharCodeFromUnicode(unicode);
}

bool CPDF_SimpleFont::HasFontWidths() const {
  return !use_font_width_;
}
