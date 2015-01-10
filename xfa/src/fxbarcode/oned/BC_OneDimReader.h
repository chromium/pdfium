// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ONEDUPCEANREADER_H_
#define _BC_ONEDUPCEANREADER_H_
class CBC_OneDReader;
class CBC_CommonBitArray;
class CBC_OneDimReader;
class CBC_OneDimReader : public CBC_OneDReader
{
private:
    const static FX_INT32 MAX_AVG_VARIANCE;
    const static FX_INT32 MAX_INDIVIDUAL_VARIANCE;

    FX_BOOL CheckStandardUPCEANChecksum(CFX_ByteString &s, FX_INT32 &e);
public:
    const static FX_INT32 START_END_PATTERN[3];
    const static FX_INT32 MIDDLE_PATTERN[5];
    const static FX_INT32 L_PATTERNS[10][4];
    const static FX_INT32 L_AND_G_PATTERNS[20][4];
    CBC_OneDimReader();
    virtual ~CBC_OneDimReader();
    CFX_ByteString DecodeRow(FX_INT32 rowNumber, CBC_CommonBitArray *row, FX_INT32 hints, FX_INT32 &e);
    CFX_ByteString DecodeRow(FX_INT32 rowNumber, CBC_CommonBitArray *row, CFX_Int32Array *startGuardRange, FX_INT32 hints, FX_INT32 &e);
protected:
    CFX_Int32Array *FindStartGuardPattern(CBC_CommonBitArray *row, FX_INT32 &e);
    virtual FX_BOOL CheckChecksum(CFX_ByteString &s, FX_INT32 &e);
    CFX_Int32Array *FindGuardPattern(CBC_CommonBitArray *row, FX_INT32 rowOffset, FX_BOOL whiteFirst, CFX_Int32Array *pattern, FX_INT32 &e);
    FX_INT32 DecodeDigit(CBC_CommonBitArray *row, CFX_Int32Array *counters, FX_INT32 rowOffset, const FX_INT32* patterns, FX_INT32 patternLength, FX_INT32 &e);
    virtual FX_INT32 DecodeMiddle(CBC_CommonBitArray *row, CFX_Int32Array *startRange, CFX_ByteString &resultResult, FX_INT32 &e)
    {
        return 0;
    }
    virtual CFX_Int32Array *DecodeEnd(CBC_CommonBitArray *row, FX_INT32 endStart, FX_INT32 &e);
};
#endif
