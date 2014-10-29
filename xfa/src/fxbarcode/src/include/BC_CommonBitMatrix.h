// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_COMMONBITMATRIX_H_
#define _BC_COMMONBITMATRIX_H_
class CBC_CommonBitArray;
class CBC_CommonBitMatrix;
class CBC_CommonBitMatrix  : public CFX_Object
{
public:
    CBC_CommonBitMatrix();
    virtual ~CBC_CommonBitMatrix();
    FX_BOOL Get(FX_INT32 x, FX_INT32 y);
    void Set(FX_INT32 x, FX_INT32 y);
    void Flip(FX_INT32 x, FX_INT32 y);
    void Clear();
    void SetRegion(FX_INT32 left, FX_INT32 top, FX_INT32 width, FX_INT32 height, FX_INT32 &e);
    CBC_CommonBitArray* GetRow(FX_INT32 y, CBC_CommonBitArray* row);
    void SetRow(FX_INT32 y, CBC_CommonBitArray* row);
    CBC_CommonBitArray* GetCol(FX_INT32 y, CBC_CommonBitArray* row);
    void SetCol(FX_INT32 y, CBC_CommonBitArray* col);
    FX_INT32 GetWidth();
    FX_INT32 GetHeight();
    FX_INT32 GetRowSize();
    FX_INT32 GetDimension(FX_INT32 &e);
    virtual void Init(FX_INT32 dimension);
    virtual void Init(FX_INT32 width, FX_INT32 height);
    FX_INT32* GetBits();
private:
    FX_INT32 m_width;
    FX_INT32 m_height;
    FX_INT32 m_rowSize;
    FX_INT32* m_bits;
};
#endif
