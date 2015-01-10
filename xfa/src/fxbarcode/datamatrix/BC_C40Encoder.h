// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_C40ENCODER_H_
#define _BC_C40ENCODER_H_
class CBC_C40Encoder;
class CBC_C40Encoder : public CBC_Encoder
{
public:
    CBC_C40Encoder();
    virtual ~CBC_C40Encoder();
    virtual FX_INT32 getEncodingMode();
    virtual void Encode(CBC_EncoderContext &context, FX_INT32 &e);
    static void writeNextTriplet(CBC_EncoderContext &context, CFX_WideString &buffer);
    virtual void handleEOD(CBC_EncoderContext &context, CFX_WideString &buffer, FX_INT32 &e);
    virtual FX_INT32 encodeChar(FX_WCHAR c, CFX_WideString &sb, FX_INT32 &e);
private:
    FX_INT32 backtrackOneCharacter(CBC_EncoderContext &context, CFX_WideString &buffer, CFX_WideString &removed, FX_INT32 lastCharSize, FX_INT32 &e);
    static CFX_WideString encodeToCodewords(CFX_WideString sb, FX_INT32 startPos);
};
#endif
