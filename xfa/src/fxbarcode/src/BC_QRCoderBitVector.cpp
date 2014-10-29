// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_QRCoderBitVector.h"
CBC_QRCoderBitVector::CBC_QRCoderBitVector()
{
    m_sizeInBits = 0;
    m_size = 32;
}
void CBC_QRCoderBitVector::Init()
{
    m_array = FX_Alloc(FX_BYTE, m_size);
}
CBC_QRCoderBitVector::~CBC_QRCoderBitVector()
{
    if(m_array != NULL) {
        FX_Free(m_array);
    }
    m_size = 0;
    m_sizeInBits = 0;
}
void CBC_QRCoderBitVector::Clear()
{
    if(m_array != NULL) {
        FX_Free(m_array);
        m_array = NULL;
    }
    m_sizeInBits = 0;
    m_size = 32;
    m_array = FX_Alloc(FX_BYTE, m_size);
}
FX_INT32 CBC_QRCoderBitVector::At(FX_INT32 index, FX_INT32 &e)
{
    if(index < 0 || index >= m_sizeInBits) {
        e = BCExceptionBadIndexException;
        BC_EXCEPTION_CHECK_ReturnValue(e, 0);
    }
    FX_INT32 value = m_array[index >> 3] & 0xff;
    return (value >> (7 - (index & 0x7))) & 1;
}
FX_INT32 CBC_QRCoderBitVector::sizeInBytes()
{
    return (m_sizeInBits + 7) >> 3;
}
FX_INT32 CBC_QRCoderBitVector::Size()
{
    return m_sizeInBits;
}
void CBC_QRCoderBitVector::AppendBit(FX_INT32 bit, FX_INT32 &e)
{
    if(!(bit == 0 || bit == 1)) {
        e = BCExceptionBadValueException;
        BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
    FX_INT32 numBitsInLastByte = m_sizeInBits & 0x7;
    if(numBitsInLastByte == 0) {
        AppendByte(0);
        m_sizeInBits -= 8;
    }
    m_array[m_sizeInBits >> 3] |= (bit << (7 - numBitsInLastByte));
    ++m_sizeInBits;
}
void CBC_QRCoderBitVector::AppendBits(FX_INT32 value, FX_INT32 numBits, FX_INT32 &e)
{
    if (numBits < 0 || numBits > 32) {
        e = BCExceptionBadNumBitsException;
        BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
    FX_INT32 numBitsLeft = numBits;
    while (numBitsLeft > 0) {
        if ((m_sizeInBits & 0x7) == 0 && numBitsLeft >= 8) {
            FX_INT32 newByte = (value >> (numBitsLeft - 8)) & 0xff;
            AppendByte(newByte);
            numBitsLeft -= 8;
        } else {
            FX_INT32 bit = (value >> (numBitsLeft - 1)) & 1;
            AppendBit(bit, e);
            BC_EXCEPTION_CHECK_ReturnVoid(e);
            --numBitsLeft;
        }
    }
}
void CBC_QRCoderBitVector::AppendBitVector(CBC_QRCoderBitVector *bits, FX_INT32 &e)
{
    FX_INT32 size = bits->Size();
    for(FX_INT32 i = 0; i < size; i++) {
        FX_INT32 num = bits->At(i, e);
        BC_EXCEPTION_CHECK_ReturnVoid(e);
        AppendBit(num, e);
        BC_EXCEPTION_CHECK_ReturnVoid(e)
    }
}
void CBC_QRCoderBitVector::XOR(CBC_QRCoderBitVector *other, FX_INT32 &e)
{
    if(m_sizeInBits != other->Size()) {
        e = BCExceptioncanNotOperatexorOperator;
        BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
    FX_INT32 sizeInBytes = (m_sizeInBits + 7) >> 3;
    for(FX_INT32 i = 0; i < sizeInBytes; ++i) {
        m_array[i] ^= (other->GetArray())[i];
    }
}
FX_BYTE* CBC_QRCoderBitVector::GetArray()
{
    return m_array;
}
void CBC_QRCoderBitVector::AppendByte(FX_INT32 value)
{
    if((m_sizeInBits >> 3) == m_size) {
        FX_BYTE* newArray = FX_Alloc(FX_BYTE, m_size << 1);
        FXSYS_memcpy32(newArray, m_array, m_size);
        if(m_array != NULL) {
            FX_Free(m_array);
        }
        m_array = newArray;
        m_size = m_size << 1;
    }
    m_array[m_sizeInBits >> 3] = (FX_BYTE) value;
    m_sizeInBits += 8;
}
