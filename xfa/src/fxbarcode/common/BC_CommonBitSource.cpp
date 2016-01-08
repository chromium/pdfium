// Copyright 2014 PDFium Authors. All rights reserved.
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

#include "xfa/src/fxbarcode/barcode.h"
#include "BC_CommonBitSource.h"
CBC_CommonBitSource::CBC_CommonBitSource(CFX_ByteArray* bytes) {
  m_bytes.Copy((*bytes));
  m_bitOffset = 0;
  m_byteOffset = 0;
}
CBC_CommonBitSource::~CBC_CommonBitSource() {}
int32_t CBC_CommonBitSource::ReadBits(int32_t numBits, int32_t& e) {
  if (numBits < 1 || numBits > 32) {
    e = BCExceptionIllegalArgument;
    return 0;
  }
  int32_t result = 0;
  if (m_bitOffset > 0) {
    int32_t bitsLeft = 8 - m_bitOffset;
    int32_t toRead = numBits < bitsLeft ? numBits : bitsLeft;
    int32_t bitsToNotRead = bitsLeft - toRead;
    int32_t mask = (0xff >> (8 - toRead)) << bitsToNotRead;
    result = (m_bytes[m_byteOffset] & mask) >> bitsToNotRead;
    numBits -= toRead;
    m_bitOffset += toRead;
    if (m_bitOffset == 8) {
      m_bitOffset = 0;
      m_byteOffset++;
    }
  }
  if (numBits > 0) {
    while (numBits >= 8) {
      result = (result << 8) | (m_bytes[m_byteOffset] & 0xff);
      m_byteOffset++;
      numBits -= 8;
    }
    if (numBits > 0) {
      int32_t bitsToNotRead = 8 - numBits;
      int32_t mask = (0xff >> bitsToNotRead) << bitsToNotRead;
      result = (result << numBits) |
               ((m_bytes[m_byteOffset] & mask) >> bitsToNotRead);
      m_bitOffset += numBits;
    }
  }
  return result;
}
int32_t CBC_CommonBitSource::Available() {
  return 8 * (m_bytes.GetSize() - m_byteOffset) - m_bitOffset;
}
int32_t CBC_CommonBitSource::getByteOffset() {
  return m_byteOffset;
}
