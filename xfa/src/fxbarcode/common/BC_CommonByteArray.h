// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_COMMONBYTEARRAY_H_
#define _BC_COMMONBYTEARRAY_H_
class CBC_CommonByteArray;
class CBC_CommonByteArray  : public CFX_Object
{
private:
    FX_INT32 m_size;
    FX_INT32 m_index;
    FX_BYTE* m_bytes;
public:
    CBC_CommonByteArray();
    CBC_CommonByteArray(FX_INT32 size);
    CBC_CommonByteArray(FX_BYTE* byteArray, FX_INT32 size);
    virtual ~CBC_CommonByteArray();
    FX_INT32 At(FX_INT32 index);
    void Set(FX_INT32 index, FX_INT32 value);
    FX_INT32 Size();
    FX_BOOL IsEmpty();
    void AppendByte(FX_INT32 value);
    void Reserve(FX_INT32 capacity);
    void Set(FX_BYTE* source, FX_INT32 offset, FX_INT32 count);
    void Set(CFX_ByteArray* source, FX_INT32 offset, FX_INT32 count);
};
#endif
