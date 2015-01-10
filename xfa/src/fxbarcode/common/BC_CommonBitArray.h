// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_COMMONBITARRAY_H_
#define _BC_COMMONBITARRAY_H_
class CBC_CommonBitArray;
class CBC_CommonBitArray  : public CFX_Object
{
public:
    CBC_CommonBitArray(CBC_CommonBitArray* array);
    CBC_CommonBitArray(FX_INT32 size);
    CBC_CommonBitArray();
    virtual ~CBC_CommonBitArray();
    FX_INT32 GetSize();
    CFX_Int32Array& GetBits();
    FX_INT32 GetSizeInBytes();
    FX_BOOL Get(FX_INT32 i);
    void Set(FX_INT32 i);
    void Flip(FX_INT32 i);
    void SetBulk(FX_INT32 i, FX_INT32 newBits);
    FX_BOOL IsRange(FX_INT32 start, FX_INT32 end, FX_BOOL value, FX_INT32 &e);
    FX_INT32 *GetBitArray();
    void Reverse();
    void Clear();
private:
    FX_INT32  m_size;
    CFX_Int32Array m_bits;
};
#endif
