// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_Encoder.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_Dimension.h"
#include "include/BC_SymbolShapeHint.h"
#include "include/BC_SymbolInfo.h"
#include "include/BC_EncoderContext.h"
#include "include/BC_UtilCodingConvert.h"
CBC_EncoderContext::CBC_EncoderContext(const CFX_WideString msg, CFX_WideString ecLevel, FX_INT32 &e)
{
    CFX_ByteString dststr;
    CBC_UtilCodingConvert::UnicodeToUTF8(msg, dststr);
    CFX_WideString sb;
    FX_INT32 c = dststr.GetLength();
    for (FX_INT32 i = 0; i < c; i++) {
        FX_WCHAR ch =  (FX_WCHAR)(dststr.GetAt(i) & 0xff);
        if (ch == '?' && dststr.GetAt(i) != '?') {
            e = BCExceptionCharactersOutsideISO88591Encoding;
        }
        sb += ch;
    }
    m_msg = sb;
    m_shape = FORCE_NONE;
    m_newEncoding = -1;
    m_pos = 0;
    m_symbolInfo = NULL;
    m_skipAtEnd = 0;
    m_maxSize = NULL;
    m_minSize = NULL;
}
CBC_EncoderContext::~CBC_EncoderContext()
{
}
void CBC_EncoderContext::setSymbolShape(SymbolShapeHint shape)
{
    m_shape = shape;
}
void CBC_EncoderContext::setSizeConstraints(CBC_Dimension* minSize, CBC_Dimension* maxSize)
{
    m_maxSize = maxSize;
    m_minSize = minSize;
}
CFX_WideString CBC_EncoderContext::getMessage()
{
    return m_msg;
}
void CBC_EncoderContext::setSkipAtEnd(FX_INT32 count)
{
    m_skipAtEnd = count;
}
FX_WCHAR CBC_EncoderContext::getCurrentChar()
{
    return m_msg.GetAt(m_pos);
}
FX_WCHAR CBC_EncoderContext::getCurrent()
{
    return m_msg.GetAt(m_pos);
}
void CBC_EncoderContext::writeCodewords(CFX_WideString codewords)
{
    m_codewords += codewords;
}
void CBC_EncoderContext::writeCodeword(FX_WCHAR codeword)
{
    m_codewords += codeword;
}
FX_INT32 CBC_EncoderContext::getCodewordCount()
{
    return m_codewords.GetLength();
}
void CBC_EncoderContext::signalEncoderChange(FX_INT32 encoding)
{
    m_newEncoding = encoding;
}
void CBC_EncoderContext::resetEncoderSignal()
{
    m_newEncoding = -1;
}
FX_BOOL CBC_EncoderContext::hasMoreCharacters()
{
    return m_pos < getTotalMessageCharCount();
}
FX_INT32 CBC_EncoderContext::getRemainingCharacters()
{
    return getTotalMessageCharCount() - m_pos;
}
void CBC_EncoderContext::updateSymbolInfo(FX_INT32 &e)
{
    updateSymbolInfo(getCodewordCount(), e);
}
void CBC_EncoderContext::updateSymbolInfo(FX_INT32 len, FX_INT32 &e)
{
    if (m_symbolInfo == NULL || len > m_symbolInfo->m_dataCapacity) {
        m_symbolInfo = CBC_SymbolInfo::lookup(len, m_shape, m_minSize, m_maxSize, true, e);
        BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
}
void CBC_EncoderContext::resetSymbolInfo()
{
    m_shape = FORCE_NONE;
}
FX_INT32 CBC_EncoderContext::getTotalMessageCharCount()
{
    return m_msg.GetLength() - m_skipAtEnd;
}
