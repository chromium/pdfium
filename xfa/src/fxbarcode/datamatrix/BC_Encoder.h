// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ENCODER_H_
#define _BC_ENCODER_H_
class CBC_EncoderContext;
class CBC_Encoder;
class CBC_Encoder : public CFX_Object
{
public:
    CBC_Encoder();
    virtual ~CBC_Encoder();
    virtual FX_INT32 getEncodingMode() = 0;
    virtual void Encode(CBC_EncoderContext &context, FX_INT32 &e) = 0;
};
#endif
