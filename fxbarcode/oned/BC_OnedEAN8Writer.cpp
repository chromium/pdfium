// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2009 ZXing authors
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

#include "fxbarcode/oned/BC_OnedEAN8Writer.h"

#include <math.h>

#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include "core/fxcrt/fx_extension.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/text_char_pos.h"
#include "fxbarcode/BC_Writer.h"
#include "fxbarcode/oned/BC_OneDimWriter.h"
#include "fxbarcode/oned/BC_OnedEANChecksum.h"

namespace {

const uint8_t kOnedEAN8StartPattern[3] = {1, 1, 1};
const uint8_t kOnedEAN8MiddlePattern[5] = {1, 1, 1, 1, 1};

using LPatternRow = std::array<uint8_t, 4>;
constexpr std::array<const LPatternRow, 10> kOnedEAN8LPatternTable = {
    {{3, 2, 1, 1},
     {2, 2, 2, 1},
     {2, 1, 2, 2},
     {1, 4, 1, 1},
     {1, 1, 3, 2},
     {1, 2, 3, 1},
     {1, 1, 1, 4},
     {1, 3, 1, 2},
     {1, 2, 1, 3},
     {3, 1, 1, 2}}};

}  // namespace

CBC_OnedEAN8Writer::CBC_OnedEAN8Writer() {
  data_length_ = 8;
}

CBC_OnedEAN8Writer::~CBC_OnedEAN8Writer() = default;

void CBC_OnedEAN8Writer::SetDataLength(int32_t length) {
  data_length_ = 8;
}

void CBC_OnedEAN8Writer::SetTextLocation(BC_TEXT_LOC location) {
  if (location == BC_TEXT_LOC::kBelowEmbed) {
    loc_text_loc_ = location;
  }
}

bool CBC_OnedEAN8Writer::CheckContentValidity(WideStringView contents) {
  return HasValidContentSize(contents) &&
         std::all_of(contents.begin(), contents.end(),
                     [](wchar_t c) { return FXSYS_IsDecimalDigit(c); });
}

WideString CBC_OnedEAN8Writer::FilterContents(WideStringView contents) {
  WideString filtercontents;
  filtercontents.Reserve(contents.GetLength());
  wchar_t ch;
  for (size_t i = 0; i < contents.GetLength(); i++) {
    ch = contents[i];
    if (ch > 175) {
      i++;
      continue;
    }
    if (FXSYS_IsDecimalDigit(ch)) {
      filtercontents += ch;
    }
  }
  return filtercontents;
}

int32_t CBC_OnedEAN8Writer::CalcChecksum(const ByteString& contents) {
  return EANCalcChecksum(contents);
}

DataVector<uint8_t> CBC_OnedEAN8Writer::Encode(const ByteString& contents) {
  if (contents.GetLength() != 8) {
    return {};
  }

  DataVector<uint8_t> result(code_width_);
  auto result_span = pdfium::span(result);
  result_span = AppendPattern(result_span, kOnedEAN8StartPattern, true);

  for (int i = 0; i <= 3; i++) {
    int32_t digit = FXSYS_DecimalCharToInt(contents[i]);
    result_span =
        AppendPattern(result_span, kOnedEAN8LPatternTable[digit], false);
  }
  result_span = AppendPattern(result_span, kOnedEAN8MiddlePattern, false);

  for (int i = 4; i <= 7; i++) {
    int32_t digit = FXSYS_DecimalCharToInt(contents[i]);
    result_span =
        AppendPattern(result_span, kOnedEAN8LPatternTable[digit], true);
  }
  AppendPattern(result_span, kOnedEAN8StartPattern, true);
  return result;
}

bool CBC_OnedEAN8Writer::ShowChars(WideStringView contents,
                                   CFX_RenderDevice* device,
                                   const CFX_Matrix& matrix,
                                   int32_t barWidth) {
  if (!device) {
    return false;
  }

  static constexpr float kLeftPosition = 3.0f;
  ByteString str = FX_UTF8Encode(contents);
  size_t iLength = str.GetLength();
  std::vector<TextCharPos> charpos(iLength);
  ByteString tempStr = str.First(4);
  size_t iLen = tempStr.GetLength();
  static constexpr int32_t kWidth = 28;
  float blank = 0.0f;

  int32_t iFontSize = static_cast<int32_t>(fabs(font_size_));
  int32_t iTextHeight = iFontSize + 1;

  CFX_Matrix matr(output_hscale_, 0.0, 0.0, 1.0, 0.0, 0.0);
  CFX_FloatRect rect(kLeftPosition, (float)(height_ - iTextHeight),
                     kLeftPosition + kWidth - 0.5, (float)height_);
  matr.Concat(matrix);
  FX_RECT re = matr.TransformRect(rect).GetOuterRect();
  device->FillRect(re, kBackgroundColor);
  CFX_Matrix matr1(output_hscale_, 0.0, 0.0, 1.0, 0.0, 0.0);
  CFX_FloatRect rect1(kLeftPosition + 33, (float)(height_ - iTextHeight),
                      kLeftPosition + 33 + kWidth - 0.5, (float)height_);
  matr1.Concat(matrix);
  re = matr1.TransformRect(rect1).GetOuterRect();
  device->FillRect(re, kBackgroundColor);
  int32_t strWidth = static_cast<int32_t>(kWidth * output_hscale_);

  pdfium::span<TextCharPos> charpos_span = pdfium::span(charpos);
  CalcTextInfo(tempStr, charpos, font_, (float)strWidth, iFontSize, blank);
  {
    CFX_Matrix affine_matrix1(1.0, 0.0, 0.0, -1.0,
                              kLeftPosition * output_hscale_,
                              (float)(height_ - iTextHeight + iFontSize));
    affine_matrix1.Concat(matrix);
    device->DrawNormalText(charpos_span.first(iLen), font_,
                           static_cast<float>(iFontSize), affine_matrix1,
                           font_color_, GetTextRenderOptions());
  }
  tempStr = str.Substr(4, 4);
  iLen = tempStr.GetLength();
  CalcTextInfo(tempStr, charpos_span.subspan<4u>(), font_, (float)strWidth,
               iFontSize, blank);
  {
    CFX_Matrix affine_matrix1(1.0, 0.0, 0.0, -1.0,
                              (kLeftPosition + 33) * output_hscale_,
                              (float)(height_ - iTextHeight + iFontSize));
    affine_matrix1.Concat(matrix);
    device->DrawNormalText(charpos_span.subspan(4u, iLen), font_,
                           static_cast<float>(iFontSize), affine_matrix1,
                           font_color_, GetTextRenderOptions());
  }
  return true;
}
