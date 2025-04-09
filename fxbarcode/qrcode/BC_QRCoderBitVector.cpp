// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2007 ZXing authors
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

#include "fxbarcode/qrcode/BC_QRCoderBitVector.h"

#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_system.h"

CBC_QRCoderBitVector::CBC_QRCoderBitVector() = default;

CBC_QRCoderBitVector::~CBC_QRCoderBitVector() = default;

int32_t CBC_QRCoderBitVector::At(size_t index) const {
  CHECK(index < size_in_bits_);
  int32_t value = array_[index >> 3] & 0xff;
  return (value >> (7 - (index & 0x7))) & 1;
}

size_t CBC_QRCoderBitVector::sizeInBytes() const {
  return (size_in_bits_ + 7) >> 3;
}

size_t CBC_QRCoderBitVector::Size() const {
  return size_in_bits_;
}

void CBC_QRCoderBitVector::AppendBit(int32_t bit) {
  DCHECK(bit == 0 || bit == 1);
  int32_t numBitsInLastByte = size_in_bits_ & 0x7;
  if (numBitsInLastByte == 0) {
    AppendByte(0);
    size_in_bits_ -= 8;
  }
  array_[size_in_bits_ >> 3] |= (bit << (7 - numBitsInLastByte));
  ++size_in_bits_;
}

void CBC_QRCoderBitVector::AppendBits(int32_t value, int32_t numBits) {
  DCHECK(numBits > 0);
  DCHECK(numBits <= 32);

  int32_t numBitsLeft = numBits;
  while (numBitsLeft > 0) {
    if ((size_in_bits_ & 0x7) == 0 && numBitsLeft >= 8) {
      AppendByte(static_cast<int8_t>((value >> (numBitsLeft - 8)) & 0xff));
      numBitsLeft -= 8;
    } else {
      AppendBit((value >> (numBitsLeft - 1)) & 1);
      --numBitsLeft;
    }
  }
}

void CBC_QRCoderBitVector::AppendBitVector(const CBC_QRCoderBitVector* bits) {
  for (size_t i = 0; i < bits->Size(); i++) {
    AppendBit(bits->At(i));
  }
}

bool CBC_QRCoderBitVector::XOR(const CBC_QRCoderBitVector* other) {
  if (size_in_bits_ != other->Size()) {
    return false;
  }

  pdfium::span<const uint8_t> other_span = other->GetArray();
  for (size_t i = 0; i < sizeInBytes(); ++i) {
    array_[i] ^= other_span[i];
  }
  return true;
}

pdfium::span<const uint8_t> CBC_QRCoderBitVector::GetArray() const {
  return array_;
}

void CBC_QRCoderBitVector::AppendByte(int8_t value) {
  if ((size_in_bits_ >> 3) == array_.size()) {
    array_.push_back(0);
  }
  array_[size_in_bits_ >> 3] = value;
  size_in_bits_ += 8;
}
