// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRECODERBITVECTOR_H_
#define _BC_QRECODERBITVECTOR_H_
class CBC_QRCoderBitVector;
class CBC_QRCoderBitVector  : public CFX_Object
{
private:
    FX_INT32 m_sizeInBits;
    FX_BYTE *m_array;
    FX_INT32 m_size;

    void AppendByte(FX_INT32 value);
public:
    CBC_QRCoderBitVector();
    virtual ~CBC_QRCoderBitVector();
    FX_INT32 At(FX_INT32 index, FX_INT32 &e);
    FX_INT32 Size();
    FX_INT32 sizeInBytes();
    void AppendBit(FX_INT32 bit, FX_INT32 &e);
    void AppendBits(FX_INT32 value, FX_INT32 numBits, FX_INT32 &e);
    void AppendBitVector(CBC_QRCoderBitVector *bits, FX_INT32 &e);
    void XOR(CBC_QRCoderBitVector *other, FX_INT32 &e);
    FX_BYTE* GetArray();
    void Clear();
    virtual void Init();
};
#endif
