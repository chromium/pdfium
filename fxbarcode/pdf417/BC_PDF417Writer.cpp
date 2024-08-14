// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2012 ZXing authors
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

#include "fxbarcode/pdf417/BC_PDF417Writer.h"

#include <stdint.h>

#include <algorithm>
#include <utility>

#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/stl_util.h"
#include "fxbarcode/BC_TwoDimWriter.h"
#include "fxbarcode/pdf417/BC_PDF417.h"
#include "fxbarcode/pdf417/BC_PDF417BarcodeMatrix.h"

namespace {

void RotateArray(DataVector<uint8_t>& bitarray, int32_t width, int32_t height) {
  DataVector<uint8_t> temp = bitarray;
  for (int32_t i = 0; i < height; i++) {
    int32_t inverse_i = height - i - 1;
    for (int32_t j = 0; j < width; j++) {
      bitarray[j * height + inverse_i] = temp[i * width + j];
    }
  }
}

}  // namespace

CBC_PDF417Writer::CBC_PDF417Writer() : CBC_TwoDimWriter(false) {}

CBC_PDF417Writer::~CBC_PDF417Writer() = default;

bool CBC_PDF417Writer::SetErrorCorrectionLevel(int32_t level) {
  if (level < 0 || level > 8) {
    return false;
  }
  set_error_correction_level(level);
  return true;
}

CBC_PDF417Writer::EncodeResult CBC_PDF417Writer::Encode(
    WideStringView contents) const {
  CBC_PDF417 encoder;
  int32_t col = (m_Width / m_ModuleWidth - 69) / 17;
  int32_t row = m_Height / (m_ModuleWidth * 20);
  if (row >= 3 && row <= 90 && col >= 1 && col <= 30)
    encoder.setDimensions(col, 1, row, 3);
  else if (col >= 1 && col <= 30)
    encoder.setDimensions(col, col, 90, 3);
  else if (row >= 3 && row <= 90)
    encoder.setDimensions(30, 1, row, row);
  if (!encoder.GenerateBarcodeLogic(contents, error_correction_level())) {
    return {DataVector<uint8_t>(), 0, 0};
  }

  CBC_BarcodeMatrix* barcodeMatrix = encoder.getBarcodeMatrix();
  DataVector<uint8_t> matrix_data = barcodeMatrix->toBitArray();
  int32_t matrix_width = barcodeMatrix->getWidth();
  int32_t matrix_height = barcodeMatrix->getHeight();

  if (matrix_width < matrix_height) {
    RotateArray(matrix_data, matrix_width, matrix_height);
    std::swap(matrix_width, matrix_height);
  }
  return {std::move(matrix_data), matrix_width, matrix_height};
}

CBC_PDF417Writer::EncodeResult::EncodeResult(DataVector<uint8_t> data,
                                             int32_t width,
                                             int32_t height)
    : data(std::move(data)), width(width), height(height) {}

CBC_PDF417Writer::EncodeResult::~EncodeResult() = default;
