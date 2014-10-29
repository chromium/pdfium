// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_HIGHLEVALENCODER_H_
#define _BC_HIGHLEVALENCODER_H_
class CBC_SymbolShapeHint;
class CBC_HighLevelEncoder;
#define  ASCII_ENCODATION     0
#define  C40_ENCODATION       1
#define  TEXT_ENCODATION      2
#define  X12_ENCODATION       3
#define  EDIFACT_ENCODATION   4
#define  BASE256_ENCODATION   5
class CBC_HighLevelEncoder : public CBC_SymbolShapeHint
{
public:
    CBC_HighLevelEncoder();
    virtual ~CBC_HighLevelEncoder();
    CFX_ByteArray& getBytesForMessage(CFX_WideString msg);
    static CFX_WideString encodeHighLevel(CFX_WideString msg, CFX_WideString ecLevel, FX_INT32 &e);
    static CFX_WideString encodeHighLevel(CFX_WideString msg, CFX_WideString ecLevel, SymbolShapeHint shape, CBC_Dimension* minSize, CBC_Dimension* maxSize, FX_INT32 &e);
    static FX_INT32 lookAheadTest(CFX_WideString msg, FX_INT32 startpos, FX_INT32 currentMode);
    static FX_BOOL isDigit(FX_WCHAR ch);
    static FX_BOOL isExtendedASCII(FX_WCHAR ch);
    static FX_INT32 determineConsecutiveDigitCount(CFX_WideString msg, FX_INT32 startpos);
    static void illegalCharacter(FX_WCHAR c, FX_INT32 &e);

public:
    static FX_WCHAR LATCH_TO_C40;
    static FX_WCHAR LATCH_TO_BASE256;
    static FX_WCHAR UPPER_SHIFT;
    static FX_WCHAR LATCH_TO_ANSIX12;
    static FX_WCHAR LATCH_TO_TEXT;
    static FX_WCHAR LATCH_TO_EDIFACT;
    static FX_WCHAR C40_UNLATCH;
    static FX_WCHAR X12_UNLATCH;
private:
    static FX_WCHAR PAD;
    static FX_WCHAR MACRO_05;
    static FX_WCHAR MACRO_06;
    static const wchar_t* MACRO_05_HEADER;
    static const wchar_t* MACRO_06_HEADER;
    static const wchar_t MACRO_TRAILER;
    CFX_ByteArray m_bytearray;
private:
    static FX_WCHAR randomize253State(FX_WCHAR ch, FX_INT32 codewordPosition);
    static FX_INT32 findMinimums(CFX_FloatArray &charCounts, CFX_Int32Array &intCharCounts, FX_INT32 min, CFX_ByteArray &mins);
    static FX_INT32 getMinimumCount(CFX_ByteArray &mins);
    static FX_BOOL isNativeC40(FX_WCHAR ch);
    static FX_BOOL isNativeText(FX_WCHAR ch);
    static FX_BOOL isNativeX12(FX_WCHAR ch);
    static FX_BOOL isX12TermSep(FX_WCHAR ch);
    static FX_BOOL isNativeEDIFACT(FX_WCHAR ch);
    static FX_BOOL isSpecialB256(FX_WCHAR ch);

};
#endif
