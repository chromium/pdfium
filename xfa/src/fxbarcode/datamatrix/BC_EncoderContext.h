// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ENCODERCONTEXT_H_
#define _BC_ENCODERCONTEXT_H_
class CBC_SymbolShapeHint;
class CBC_SymbolInfo;
class CBC_Dimension;
class CBC_EncoderContext;
class CBC_EncoderContext : public CBC_SymbolShapeHint
{
public:
    CBC_EncoderContext(const CFX_WideString msg, CFX_WideString ecLevel, FX_INT32 &e);
    virtual ~CBC_EncoderContext();
    void setSymbolShape(SymbolShapeHint shape);
    void setSizeConstraints(CBC_Dimension* minSize, CBC_Dimension* maxSize);
    CFX_WideString getMessage();
    void setSkipAtEnd(FX_INT32 count);
    FX_WCHAR getCurrentChar();
    FX_WCHAR getCurrent();
    void writeCodewords(CFX_WideString codewords);
    void writeCodeword(FX_WCHAR codeword);
    FX_INT32 getCodewordCount();
    void signalEncoderChange(FX_INT32 encoding);
    void resetEncoderSignal();
    FX_BOOL hasMoreCharacters();
    FX_INT32 getRemainingCharacters();
    void updateSymbolInfo(FX_INT32 &e);
    void updateSymbolInfo(FX_INT32 len, FX_INT32 &e);
    void resetSymbolInfo();
public:
    CFX_WideString m_msg;
    CFX_WideString m_codewords;
    FX_INT32 m_pos;
    FX_INT32 m_newEncoding;
    CBC_SymbolInfo* m_symbolInfo;
private:
    FX_INT32 getTotalMessageCharCount();
private:
    SymbolShapeHint m_shape;
    CBC_Dimension* m_minSize;
    CBC_Dimension* m_maxSize;
    FX_INT32 m_skipAtEnd;
};
#endif
