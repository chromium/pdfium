// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_X12ENCODER_H_
#define _BC_X12ENCODER_H_
class CBC_C40Encoder;
class CBC_X12Encoder;
class CBC_X12Encoder : public CBC_C40Encoder
{
public:
    CBC_X12Encoder();
    virtual ~CBC_X12Encoder();
    FX_INT32 getEncodingMode();
    void Encode(CBC_EncoderContext &context, FX_INT32 &e);
    void handleEOD(CBC_EncoderContext &context, CFX_WideString &buffer, FX_INT32 &e);
    FX_INT32 encodeChar(FX_WCHAR c, CFX_WideString &sb, FX_INT32 &e);
};
#endif
