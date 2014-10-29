// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_Reader.h"
#include "include/BC_OneDReader.h"
#include "include/BC_OneDimReader.h"
#include "include/BC_CommonBitArray.h"
#include "include/BC_OnedEAN13Reader.h"
const FX_INT32 CBC_OnedEAN13Reader::FIRST_DIGIT_ENCODINGS[10] = {
    0x00, 0x0B, 0x0D, 0xE, 0x13, 0x19, 0x1C, 0x15, 0x16, 0x1A
};
CBC_OnedEAN13Reader::CBC_OnedEAN13Reader()
{
}
CBC_OnedEAN13Reader::~CBC_OnedEAN13Reader()
{
}
void CBC_OnedEAN13Reader::DetermineFirstDigit(CFX_ByteString &result, FX_INT32 lgPatternFound, FX_INT32 &e)
{
    for (FX_INT32 d = 0; d < 10; d++) {
        if (lgPatternFound == FIRST_DIGIT_ENCODINGS[d]) {
            result.Insert(0, (FX_CHAR) ('0' + d));
            return;
        }
    }
    e = BCExceptionNotFound;
    BC_EXCEPTION_CHECK_ReturnVoid(e);
}
FX_INT32 CBC_OnedEAN13Reader::DecodeMiddle(CBC_CommonBitArray *row, CFX_Int32Array *startRange, CFX_ByteString &resultString, FX_INT32 &e)
{
    CFX_Int32Array counters;
    counters.Add(0);
    counters.Add(0);
    counters.Add(0);
    counters.Add(0);
    FX_INT32 end = row->GetSize();
    FX_INT32 rowOffset = (*startRange)[1];
    FX_INT32 lgPatternFound = 0;
    for (FX_INT32 x = 0; x < 6 && rowOffset < end; x++) {
        FX_INT32 bestMatch = DecodeDigit(row, &counters, rowOffset, &(CBC_OneDimReader::L_AND_G_PATTERNS[0][0]), 20, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, 0);
        resultString += (FX_CHAR) ('0' + bestMatch % 10);
        for (FX_INT32 i = 0; i < counters.GetSize(); i++) {
            rowOffset += counters[i];
        }
        if (bestMatch >= 10) {
            lgPatternFound |= 1 << (5 - x);
        }
    }
    DetermineFirstDigit(resultString, lgPatternFound, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, 0);
    CFX_Int32Array result;
    result.Add(CBC_OneDimReader::MIDDLE_PATTERN[0]);
    result.Add(CBC_OneDimReader::MIDDLE_PATTERN[1]);
    result.Add(CBC_OneDimReader::MIDDLE_PATTERN[2]);
    result.Add(CBC_OneDimReader::MIDDLE_PATTERN[3]);
    result.Add(CBC_OneDimReader::MIDDLE_PATTERN[4]);
    CFX_Int32Array *middleRange = FindGuardPattern(row, rowOffset, TRUE, &result, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, 0);
    rowOffset = (*middleRange)[1];
    if(middleRange != NULL) {
        delete middleRange;
        middleRange = NULL;
    }
    for (FX_INT32 Y = 0; Y < 6 && rowOffset < end; Y++) {
        FX_INT32 bestMatch = DecodeDigit(row, &counters, rowOffset, &(CBC_OneDimReader::L_PATTERNS[0][0]), 10, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, 0);
        resultString += (FX_CHAR) ('0' + bestMatch);
        for (FX_INT32 k = 0; k < counters.GetSize(); k++) {
            rowOffset += counters[k];
        }
    }
    return rowOffset;
}
