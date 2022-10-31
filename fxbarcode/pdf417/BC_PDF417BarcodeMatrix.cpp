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
#include "core/fxcrt/span_util.h"
#include "fxbarcode/pdf417/BC_PDF417BarcodeRow.h"

CBC_BarcodeMatrix::CBC_BarcodeMatrix(size_t width, size_t height)
    : m_width((width + 4) * 17 + 1), m_height(height) {
  m_matrix.resize(m_height);
  for (size_t i = 0; i < m_height; ++i)
    m_matrix[i] = std::make_unique<CBC_BarcodeRow>(m_width);
}

CBC_BarcodeMatrix::~CBC_BarcodeMatrix() = default;

DataVector<uint8_t> CBC_BarcodeMatrix::toBitArray() {
  DataVector<uint8_t> bit_array(m_width * m_height);
  pdfium::span<uint8_t> bit_array_span(bit_array);
  for (size_t i = 0; i < m_height; ++i)
    fxcrt::spancpy(bit_array_span.subspan(i * m_width), m_matrix[i]->GetRow());
  return bit_array;
}
