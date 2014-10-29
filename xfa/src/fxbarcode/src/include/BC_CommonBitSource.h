// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_COMMONBITSOURCE_H_
#define _BC_COMMONBITSOURCE_H_
class CBC_CommonBitSource;
class CBC_CommonBitSource  : public CFX_Object
{
public:
    CBC_CommonBitSource(CFX_ByteArray *bytes);
    virtual ~CBC_CommonBitSource();
    FX_INT32 ReadBits(FX_INT32 numBits, FX_INT32 &e);
    FX_INT32 Available();
    FX_INT32 getByteOffset();
private:
    CFX_ByteArray m_bytes;
    FX_INT32 m_byteOffset;
    FX_INT32 m_bitOffset;
};
#endif
