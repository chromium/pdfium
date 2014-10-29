// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_BASE256ENCODER_H_
#define _BC_BASE256ENCODER_H_
class CBC_Encoder;
class CBC_Base256Encoder;
class CBC_Base256Encoder : public CBC_Encoder
{
public:
    CBC_Base256Encoder();
    virtual ~CBC_Base256Encoder();
    FX_INT32 getEncodingMode();
    void Encode(CBC_EncoderContext &context, FX_INT32 &e);
private:
    static FX_WCHAR randomize255State(FX_WCHAR ch, FX_INT32 codewordPosition);
};
#endif
