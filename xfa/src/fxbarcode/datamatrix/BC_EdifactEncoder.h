// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_EDIFACTENCODER_H_
#define _BC_EDIFACTENCODER_H_
class CBC_EncoderContext;
class CBC_EdifactEncoder;
class CBC_EdifactEncoder : public CBC_Encoder
{
public:
    CBC_EdifactEncoder();
    virtual ~CBC_EdifactEncoder();
    FX_INT32 getEncodingMode();
    void Encode(CBC_EncoderContext &context, FX_INT32 &e);
private:
    static void handleEOD(CBC_EncoderContext &context, CFX_WideString buffer, FX_INT32 &e);
    static void encodeChar(FX_WCHAR c, CFX_WideString &sb, FX_INT32 &e);
    static CFX_WideString encodeToCodewords(CFX_WideString sb, FX_INT32 startPos, FX_INT32 &e);
};
#endif
