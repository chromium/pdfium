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

#include "fxbarcode/pdf417/BC_PDF417BarcodeRow.h"

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/stl_util.h"

CBC_BarcodeRow::CBC_BarcodeRow(size_t width)
    : row_(FixedSizeDataVector<uint8_t>::Zeroed(width)) {}

CBC_BarcodeRow::~CBC_BarcodeRow() = default;

void CBC_BarcodeRow::AddBar(bool black, size_t width) {
  pdfium::span<uint8_t> available = row_.subspan(offset_);
  CHECK_LE(width, available.size());
  fxcrt::Fill(available.first(width), black ? 1 : 0);
  offset_ += width;
}
