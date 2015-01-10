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

#include "../barcode.h"
#include "BC_CommonBitArray.h"
#include "BC_CommonBitMatrix.h"
CBC_CommonBitMatrix::CBC_CommonBitMatrix()
{
    m_width = 0;
    m_height = 0;
    m_rowSize = 0;
    m_bits = NULL;
}
void CBC_CommonBitMatrix::Init(FX_INT32 dimension)
{
    m_width = dimension;
    m_height = dimension;
    FX_INT32 rowSize = (m_height + 31) >> 5;
    m_rowSize = rowSize;
    m_bits = FX_Alloc(FX_INT32, m_rowSize * m_height);
    FXSYS_memset32(m_bits, 0, m_rowSize * m_height * sizeof(FX_INT32));
}
void CBC_CommonBitMatrix::Init(FX_INT32 width, FX_INT32 height)
{
    m_width = width;
    m_height = height;
    FX_INT32 rowSize = (width + 31) >> 5;
    m_rowSize = rowSize;
    m_bits = FX_Alloc(FX_INT32, m_rowSize * m_height);
    FXSYS_memset32(m_bits, 0, m_rowSize * m_height * sizeof(FX_INT32));
}
CBC_CommonBitMatrix::~CBC_CommonBitMatrix()
{
    if (m_bits != NULL) {
        FX_Free(m_bits);
    }
    m_bits = NULL;
    m_height = m_width = m_rowSize = 0;
}
FX_BOOL CBC_CommonBitMatrix::Get(FX_INT32 x, FX_INT32 y)
{
    FX_INT32 offset = y * m_rowSize + (x >> 5);
    if (offset >= m_rowSize * m_height || offset < 0) {
        return false;
    }
    return ((((FX_DWORD)m_bits[offset]) >> (x & 0x1f)) & 1) != 0;
}
FX_INT32* CBC_CommonBitMatrix::GetBits()
{
    return m_bits;
}
void CBC_CommonBitMatrix::Set(FX_INT32 x, FX_INT32 y)
{
    FX_INT32 offset = y * m_rowSize + (x >> 5);
    if (offset >= m_rowSize * m_height || offset < 0) {
        return;
    }
    m_bits[offset] |= 1 << (x & 0x1f);
}
void CBC_CommonBitMatrix::Flip(FX_INT32 x, FX_INT32 y)
{
    FX_INT32 offset = y * m_rowSize + (x >> 5);
    m_bits[offset] ^= 1 << (x & 0x1f);
}
void CBC_CommonBitMatrix::Clear()
{
    FXSYS_memset32(m_bits, 0, m_rowSize * m_height * sizeof(FX_INT32));
}
void CBC_CommonBitMatrix::SetRegion(FX_INT32 left, FX_INT32 top, FX_INT32 width, FX_INT32 height, FX_INT32 &e)
{
    if (top < 0 || left < 0) {
        e = BCExceptionLeftAndTopMustBeNonnegative;
        return;
    }
    if (height < 1 || width < 1) {
        e = BCExceptionHeightAndWidthMustBeAtLeast1;
        return;
    }
    FX_INT32 right = left + width;
    FX_INT32 bottom = top + height;
    if (m_height < bottom || m_width < right) {
        e = BCExceptionRegionMustFitInsideMatrix;
        return;
    }
    FX_INT32 y;
    for (y = top; y < bottom; y++) {
        FX_INT32 offset = y * m_rowSize;
        FX_INT32 x;
        for (x = left; x < right; x++) {
            m_bits[offset + (x >> 5)] |= 1 << (x & 0x1f);
        }
    }
}
CBC_CommonBitArray* CBC_CommonBitMatrix::GetRow(FX_INT32 y, CBC_CommonBitArray* row)
{
    CBC_CommonBitArray* rowArray = NULL;
    if (row == NULL || row->GetSize() < m_width) {
        rowArray = FX_NEW CBC_CommonBitArray(m_width);
    } else {
        rowArray = FX_NEW CBC_CommonBitArray(row);
    }
    FX_INT32 offset = y * m_rowSize;
    FX_INT32 x;
    for (x = 0; x < m_rowSize; x++) {
        rowArray->SetBulk(x << 5, m_bits[offset + x]);
    }
    return rowArray;
}
void CBC_CommonBitMatrix::SetRow(FX_INT32 y, CBC_CommonBitArray* row)
{
    FX_INT32 l = y * m_rowSize;
    for (FX_INT32 i = 0; i < m_rowSize; i++) {
        m_bits[l] = row->GetBitArray()[i];
        l++;
    }
}
void CBC_CommonBitMatrix::SetCol(FX_INT32 y, CBC_CommonBitArray* col)
{
    for (FX_INT32 i = 0; i < col->GetBits().GetSize(); i++) {
        m_bits[i * m_rowSize + y] = col->GetBitArray()[i];
    }
}
FX_INT32 CBC_CommonBitMatrix::GetWidth()
{
    return m_width;
}
FX_INT32 CBC_CommonBitMatrix::GetHeight()
{
    return m_height;
}
FX_INT32 CBC_CommonBitMatrix::GetRowSize()
{
    return m_rowSize;
}
FX_INT32 CBC_CommonBitMatrix::GetDimension(FX_INT32 &e)
{
    if (m_width != m_height) {
        e = BCExceptionCanNotCallGetDimensionOnNonSquareMatrix;
        return 0;
    }
    return m_width;
}
