// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2011 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fxbarcode/oned/BC_OneDimWriter.h"

#include <math.h>

#include <algorithm>
#include <memory>
#include <vector>

#include "build/build_config.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/cfx_unicodeencodingex.h"
#include "core/fxge/text_char_pos.h"
#include "fxbarcode/BC_Writer.h"

// static
bool CBC_OneDimWriter::HasValidContentSize(WideStringView contents) {
  // Limit the size of 1D barcodes. Typical 1D barcodes are short so this should
  // be sufficient for most use cases.
  static constexpr size_t kMaxInputLengthBytes = 8192;

  size_t size = contents.GetLength();
  return size > 0 && size <= kMaxInputLengthBytes;
}

CBC_OneDimWriter::CBC_OneDimWriter() = default;

CBC_OneDimWriter::~CBC_OneDimWriter() = default;

void CBC_OneDimWriter::SetPrintChecksum(bool checksum) {
  print_checksum_ = checksum;
}

void CBC_OneDimWriter::SetDataLength(int32_t length) {
  data_length_ = length;
}

void CBC_OneDimWriter::SetCalcChecksum(bool state) {
  calc_checksum_ = state;
}

bool CBC_OneDimWriter::SetFont(CFX_Font* cFont) {
  if (!cFont) {
    return false;
  }

  font_ = cFont;
  return true;
}

void CBC_OneDimWriter::SetFontSize(float size) {
  font_size_ = size;
}

void CBC_OneDimWriter::SetFontStyle(int32_t style) {
  font_style_ = style;
}

void CBC_OneDimWriter::SetFontColor(FX_ARGB color) {
  font_color_ = color;
}

pdfium::span<uint8_t> CBC_OneDimWriter::AppendPattern(
    pdfium::span<uint8_t> target,
    pdfium::span<const uint8_t> pattern,
    bool startColor) {
  bool color = startColor;
  size_t added = 0;
  size_t pos = 0;
  for (const int8_t pattern_value : pattern) {
    for (int32_t i = 0; i < pattern_value; ++i) {
      target[pos++] = color ? 1 : 0;
    }
    added += pattern_value;
    color = !color;
  }
  return target.subspan(added);
}

void CBC_OneDimWriter::CalcTextInfo(const ByteString& text,
                                    pdfium::span<TextCharPos> charPos,
                                    CFX_Font* cFont,
                                    float geWidth,
                                    int32_t fontSize,
                                    float& charsLen) {
  std::unique_ptr<CFX_UnicodeEncodingEx> encoding =
      FX_CreateFontEncodingEx(cFont);

  const size_t length = text.GetLength();
  std::vector<uint32_t> charcodes(length);
  float charWidth = 0;
  for (size_t i = 0; i < length; ++i) {
    charcodes[i] = encoding->CharCodeFromUnicode(text[i]);
    int32_t glyph_code = encoding->GlyphFromCharCode(charcodes[i]);
    int glyph_value = cFont->GetGlyphWidth(glyph_code);
    float temp = glyph_value * fontSize / 1000.0;
    charWidth += temp;
  }
  charsLen = charWidth;
  float leftPositon = (float)(geWidth - charsLen) / 2.0f;
  if (leftPositon < 0 && geWidth == 0) {
    leftPositon = 0;
  }
  float penX = 0.0;
  float penY = (float)abs(cFont->GetDescent()) * (float)fontSize / 1000.0f;
  float left = leftPositon;
  float top = 0.0;
  charPos[0].origin_ = CFX_PointF(penX + left, penY + top);
  charPos[0].glyph_index_ = encoding->GlyphFromCharCode(charcodes[0]);
  charPos[0].font_char_width_ = cFont->GetGlyphWidth(charPos[0].glyph_index_);
#if BUILDFLAG(IS_APPLE)
  charPos[0].ext_gid_ = charPos[0].glyph_index_;
#endif
  penX += (float)(charPos[0].font_char_width_) * (float)fontSize / 1000.0f;
  for (size_t i = 1; i < length; i++) {
    charPos[i].origin_ = CFX_PointF(penX + left, penY + top);
    charPos[i].glyph_index_ = encoding->GlyphFromCharCode(charcodes[i]);
    charPos[i].font_char_width_ = cFont->GetGlyphWidth(charPos[i].glyph_index_);
#if BUILDFLAG(IS_APPLE)
    charPos[i].ext_gid_ = charPos[i].glyph_index_;
#endif
    penX += (float)(charPos[i].font_char_width_) * (float)fontSize / 1000.0f;
  }
}

void CBC_OneDimWriter::ShowDeviceChars(CFX_RenderDevice* device,
                                       const CFX_Matrix& matrix,
                                       const ByteString str,
                                       float geWidth,
                                       pdfium::span<TextCharPos> pCharPos,
                                       float locX,
                                       float locY,
                                       int32_t barWidth) {
  int32_t iFontSize = static_cast<int32_t>(fabs(font_size_));
  int32_t iTextHeight = iFontSize + 1;
  CFX_FloatRect rect((float)locX, (float)locY, (float)(locX + geWidth),
                     (float)(locY + iTextHeight));
  if (geWidth != width_) {
    rect.right -= 1;
  }
  FX_RECT re = matrix.TransformRect(rect).GetOuterRect();
  device->FillRect(re, kBackgroundColor);
  CFX_Matrix affine_matrix(1.0, 0.0, 0.0, -1.0, (float)locX,
                           (float)(locY + iFontSize));
  affine_matrix.Concat(matrix);
  device->DrawNormalText(pCharPos.first(str.GetLength()), font_,
                         static_cast<float>(iFontSize), affine_matrix,
                         font_color_, GetTextRenderOptions());
}

bool CBC_OneDimWriter::ShowChars(WideStringView contents,
                                 CFX_RenderDevice* device,
                                 const CFX_Matrix& matrix,
                                 int32_t barWidth) {
  if (!device || !font_) {
    return false;
  }

  ByteString str = FX_UTF8Encode(contents);
  std::vector<TextCharPos> charpos(str.GetLength());
  float charsLen = 0;
  float geWidth = 0;
  if (loc_text_loc_ == BC_TEXT_LOC::kAboveEmbed ||
      loc_text_loc_ == BC_TEXT_LOC::kBelowEmbed) {
    geWidth = 0;
  } else if (loc_text_loc_ == BC_TEXT_LOC::kAbove ||
             loc_text_loc_ == BC_TEXT_LOC::kBelow) {
    geWidth = (float)barWidth;
  }
  int32_t iFontSize = static_cast<int32_t>(fabs(font_size_));
  int32_t iTextHeight = iFontSize + 1;
  CalcTextInfo(str, charpos, font_, geWidth, iFontSize, charsLen);
  if (charsLen < 1) {
    return true;
  }

  int32_t locX = 0;
  int32_t locY = 0;
  switch (loc_text_loc_) {
    case BC_TEXT_LOC::kAboveEmbed:
      locX = static_cast<int32_t>(barWidth - charsLen) / 2;
      locY = 0;
      geWidth = charsLen;
      break;
    case BC_TEXT_LOC::kAbove:
      locX = 0;
      locY = 0;
      geWidth = (float)barWidth;
      break;
    case BC_TEXT_LOC::kBelowEmbed:
      locX = static_cast<int32_t>(barWidth - charsLen) / 2;
      locY = height_ - iTextHeight;
      geWidth = charsLen;
      break;
    case BC_TEXT_LOC::kBelow:
    default:
      locX = 0;
      locY = height_ - iTextHeight;
      geWidth = (float)barWidth;
      break;
  }
  ShowDeviceChars(device, matrix, str, geWidth, charpos, (float)locX,
                  (float)locY, barWidth);
  return true;
}

bool CBC_OneDimWriter::RenderDeviceResult(CFX_RenderDevice* device,
                                          const CFX_Matrix& matrix,
                                          WideStringView contents) {
  if (output_.empty()) {
    return false;
  }

  CFX_GraphStateData stateData;
  CFX_Path path;
  path.AppendRect(0, 0, static_cast<float>(width_),
                  static_cast<float>(height_));
  device->DrawPath(path, &matrix, &stateData, kBackgroundColor,
                   kBackgroundColor, CFX_FillRenderOptions::EvenOddOptions());
  CFX_Matrix scaledMatrix(output_hscale_, 0.0, 0.0, static_cast<float>(height_),
                          0.0, 0.0);
  scaledMatrix.Concat(matrix);
  for (const auto& rect : output_) {
    CFX_GraphStateData data;
    device->DrawPath(rect, &scaledMatrix, &data, kBarColor, 0,
                     CFX_FillRenderOptions::WindingOptions());
  }

  return loc_text_loc_ == BC_TEXT_LOC::kNone || !contents.Contains(' ') ||
         ShowChars(contents, device, matrix, bar_width_);
}

bool CBC_OneDimWriter::RenderResult(WideStringView contents,
                                    pdfium::span<const uint8_t> code) {
  if (code.empty()) {
    return false;
  }

  module_height_ = std::max(module_height_, 20);
  const size_t original_codelength = code.size();
  const int32_t leftPadding = left_padding_ ? 7 : 0;
  const int32_t rightPadding = right_padding_ ? 7 : 0;
  const size_t codelength = code.size() + leftPadding + rightPadding;
  output_hscale_ =
      width_ > 0 ? static_cast<float>(width_) / static_cast<float>(codelength)
                 : 1.0;
  bar_width_ = width_;

  output_.clear();
  output_.reserve(original_codelength);
  for (size_t i = 0; i < original_codelength; ++i) {
    if (code[i] != 1) {
      continue;
    }

    size_t output_index = i + leftPadding;
    if (output_index >= codelength) {
      return true;
    }

    output_.emplace_back();
    output_.back().AppendRect(output_index, 0.0f, output_index + 1, 1.0f);
  }
  return true;
}
