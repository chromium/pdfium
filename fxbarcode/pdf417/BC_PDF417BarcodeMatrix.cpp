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

#include "fxbarcode/pdf417/BC_PDF417BarcodeMatrix.h"

#include <stdint.h>

#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/stl_util.h"
#include "fxbarcode/pdf417/BC_PDF417BarcodeRow.h"

CBC_BarcodeMatrix::CBC_BarcodeMatrix(size_t width, size_t height)
    : width_((width + 4) * 17 + 1), height_(height) {
  matrix_.resize(height_);
  for (size_t i = 0; i < height_; ++i) {
    matrix_[i] = std::make_unique<CBC_BarcodeRow>(width_);
  }
}

CBC_BarcodeMatrix::~CBC_BarcodeMatrix() = default;

DataVector<uint8_t> CBC_BarcodeMatrix::toBitArray() {
  DataVector<uint8_t> bit_array(width_ * height_);
  pdfium::span<uint8_t> bit_array_span(bit_array);
  for (size_t i = 0; i < height_; ++i) {
    fxcrt::Copy(matrix_[i]->GetRow(), bit_array_span.subspan(i * width_));
  }
  return bit_array;
}
