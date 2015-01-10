// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DECODERRESULT_H_
#define _BC_DECODERRESULT_H_
class CBC_DecoderResult;
class CBC_DecoderResult : public CFX_Object
{
public:
    CBC_DecoderResult(CFX_ByteArray* rawBytes, CFX_ByteString text,  CFX_ByteString ecLevel);
    virtual ~CBC_DecoderResult();
    CFX_ByteArray* getRawBytes();
    CFX_ByteString getText();
    CFX_ByteString getECLevel();
    FX_INT32 getErrorsCorrected();
    void setErrorsCorrected(FX_INT32 errorsCorrected);
    FX_INT32 getErasures();
    void setErasures(FX_INT32 erasures);
    CFX_Object* getOther();
    void setOther(CFX_Object* other);
private:
    CFX_ByteArray* m_rawBytes;
    CFX_ByteString m_text;
    CFX_ByteString m_ecLevel;
    FX_INT32 m_errorsCorrected;
    FX_INT32 m_erasures;
    CFX_Object* m_other;
};
#endif
