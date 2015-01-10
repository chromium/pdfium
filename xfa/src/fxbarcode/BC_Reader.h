// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_READER_H_
#define _BC_READER_H_
class CBC_BinaryBitmap;
class CBC_Reader;
class CBC_Reader : public CFX_Object
{
public:
    CBC_Reader();
    virtual ~CBC_Reader();
    virtual CFX_ByteString Decode(CBC_BinaryBitmap *image, FX_INT32 &e) = 0;
    virtual CFX_ByteString Decode(CBC_BinaryBitmap *image, FX_INT32 hints, FX_INT32 &e) = 0;
};
#endif
