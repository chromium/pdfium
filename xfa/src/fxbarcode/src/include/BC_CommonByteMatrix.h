// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_COMMONBYTEMATRIX_H_
#define _BC_COMMONBYTEMATRIX_H_
class CBC_CommonByteMatrix;
class CBC_CommonByteMatrix  : public CFX_Object
{
public:
    CBC_CommonByteMatrix(FX_INT32 width, FX_INT32 height);
    virtual ~CBC_CommonByteMatrix();
    FX_INT32 GetHeight();
    FX_INT32 GetWidth();
    FX_BYTE Get(FX_INT32 x, FX_INT32 y);
    FX_BYTE* GetArray();

    void Set(FX_INT32 x, FX_INT32 y, FX_INT32 value);
    void Set(FX_INT32 x, FX_INT32 y, FX_BYTE value);
    void clear(FX_BYTE value);
    virtual void Init();
private:
    FX_BYTE *m_bytes;
    FX_INT32 m_width;
    FX_INT32 m_height;
};
#endif
