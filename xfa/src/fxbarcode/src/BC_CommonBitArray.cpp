// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_CommonBitArray.h"
CBC_CommonBitArray::CBC_CommonBitArray(CBC_CommonBitArray* array)
{
    m_size = array->GetSize();
    m_bits.Copy(array->GetBits());
}
CBC_CommonBitArray::CBC_CommonBitArray()
{
    m_bits.SetSize(1);
    m_size = 0;
}
CBC_CommonBitArray::CBC_CommonBitArray(FX_INT32 size)
{
    m_bits.SetSize((size + 31) >> 5);
    m_size = size;
}
CBC_CommonBitArray::~CBC_CommonBitArray()
{
    m_size = 0;
}
FX_INT32 CBC_CommonBitArray::GetSize()
{
    return m_size;
}
CFX_Int32Array& CBC_CommonBitArray::GetBits()
{
    return m_bits;
}
FX_INT32 CBC_CommonBitArray::GetSizeInBytes()
{
    return (m_size + 7) >> 3;
}
FX_BOOL CBC_CommonBitArray::Get(FX_INT32 i)
{
    return (m_bits[i >> 5] & (1 << (i & 0x1f))) != 0;
}
void CBC_CommonBitArray::Set(FX_INT32 i)
{
    m_bits[i >> 5] |= 1 << (i & 0x1F);
}
void CBC_CommonBitArray::Flip(FX_INT32 i)
{
    m_bits[i >> 5] ^= 1 << (i & 0x1F);
}
void CBC_CommonBitArray::SetBulk(FX_INT32 i, FX_INT32 newBits)
{
    m_bits[i >> 5] = newBits;
}
void CBC_CommonBitArray::Clear()
{
    FXSYS_memset32(&m_bits[0], 0x00, m_bits.GetSize() * sizeof(FX_INT32));
}
FX_BOOL CBC_CommonBitArray::IsRange(FX_INT32 start, FX_INT32 end, FX_BOOL value, FX_INT32 &e)
{
    if (end < start) {
        e = BCExceptionEndLessThanStart;
        return FALSE;
    }
    if (end == start) {
        return TRUE;
    }
    end--;
    FX_INT32 firstInt = start >> 5;
    FX_INT32 lastInt = end >> 5;
    FX_INT32 i;
    for (i = firstInt; i <= lastInt; i++) {
        FX_INT32 firstBit = i > firstInt ? 0 : start & 0x1F;
        FX_INT32 lastBit = i < lastInt ? 31 : end & 0x1F;
        FX_INT32 mask;
        if (firstBit == 0 && lastBit == 31) {
            mask = -1;
        } else {
            mask = 0;
            for (FX_INT32 j = firstBit; j <= lastBit; j++) {
                mask |= 1 << j;
            }
        }
        if ((m_bits[i] & mask) != (value ? mask : 0)) {
            return FALSE;
        }
    }
    return TRUE;
}
FX_INT32* CBC_CommonBitArray::GetBitArray()
{
    return &m_bits[0];
}
void CBC_CommonBitArray::Reverse()
{
    FX_INT32* newBits = FX_Alloc(FX_INT32, m_bits.GetSize());
    FXSYS_memset32(newBits, 0x00, m_bits.GetSize() * sizeof(FX_INT32));
    FX_INT32 size = m_size;
    FX_INT32 i;
    for (i = 0; i < size; i++) {
        if (Get(size - i - 1)) {
            newBits[i >> 5] |= 1 << (i & 0x1F);
        }
    }
    FXSYS_memcpy32(&m_bits[0], newBits, m_bits.GetSize() * sizeof(FX_INT32));
    FX_Free(newBits);
}
