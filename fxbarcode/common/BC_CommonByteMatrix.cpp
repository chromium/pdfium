// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2008 ZXing authors
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
#include "fxbarcode/common/BC_CommonByteMatrix.h"

#include <algorithm>
#include <iterator>
#include <utility>

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/data_vector.h"

CBC_CommonByteMatrix::CBC_CommonByteMatrix(size_t width, size_t height)
    : width_(width), height_(height) {
  static constexpr size_t kMaxBytes = 256 * 1024 * 1024;  // 256 MB.
  static constexpr uint8_t kDefaultFill = 0xff;
  CHECK_LT(width_, kMaxBytes / height_);
  bytes_.resize(width_ * height_, kDefaultFill);
}

CBC_CommonByteMatrix::~CBC_CommonByteMatrix() = default;

DataVector<uint8_t> CBC_CommonByteMatrix::TakeArray() {
  return std::move(bytes_);
}

uint8_t CBC_CommonByteMatrix::Get(size_t x, size_t y) const {
  const size_t offset = y * width_ + x;
  CHECK_LT(offset, bytes_.size());
  return bytes_[offset];
}

void CBC_CommonByteMatrix::Set(size_t x, size_t y, uint8_t value) {
  const size_t offset = y * width_ + x;
  CHECK_LT(offset, bytes_.size());
  bytes_[offset] = value;
}

void CBC_CommonByteMatrix::Fill(uint8_t value) {
  std::fill(std::begin(bytes_), std::end(bytes_), value);
}
