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

#include "barcode.h"
#include "include/BC_CommonBitSource.h"
CBC_CommonBitSource::CBC_CommonBitSource(CFX_ByteArray* bytes)
{
    m_bytes.Copy((*bytes));
    m_bitOffset = 0;
    m_byteOffset = 0;
}
CBC_CommonBitSource::~CBC_CommonBitSource()
{
}
FX_INT32 CBC_CommonBitSource::ReadBits(FX_INT32 numBits, FX_INT32 &e)
{
    if (numBits < 1 || numBits > 32) {
        e = BCExceptionIllegalArgument;
        return 0;
    }
    FX_INT32 result = 0;
    if (m_bitOffset > 0) {
        FX_INT32 bitsLeft = 8 - m_bitOffset;
        FX_INT32 toRead = numBits < bitsLeft ? numBits : bitsLeft;
        FX_INT32 bitsToNotRead = bitsLeft - toRead;
        FX_INT32 mask = (0xff >> (8 - toRead)) << bitsToNotRead;
        result = (m_bytes[m_byteOffset] & mask) >> bitsToNotRead;
        numBits -= toRead;
        m_bitOffset += toRead;
        if (m_bitOffset == 8) {
            m_bitOffset = 0;
            m_byteOffset++;
        }
    }
    if (numBits > 0) {
        while(numBits >= 8) {
            result = (result << 8) | (m_bytes[m_byteOffset] & 0xff);
            m_byteOffset++;
            numBits -= 8;
        }
        if (numBits > 0) {
            FX_INT32 bitsToNotRead = 8 - numBits;
            FX_INT32 mask = (0xff >> bitsToNotRead) << bitsToNotRead;
            result = (result << numBits) | ((m_bytes[m_byteOffset] & mask) >> bitsToNotRead);
            m_bitOffset += numBits;
        }
    }
    return result;
}
FX_INT32 CBC_CommonBitSource::Available()
{
    return 8 * (m_bytes.GetSize() - m_byteOffset) - m_bitOffset;
}
FX_INT32 CBC_CommonBitSource::getByteOffset()
{
    return m_byteOffset;
}
