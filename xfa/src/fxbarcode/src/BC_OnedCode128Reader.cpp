// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_Reader.h"
#include "include/BC_OneDReader.h"
#include "include/BC_CommonBitArray.h"
#include "include/BC_OnedCode128Reader.h"
const FX_INT32 CBC_OnedCode128Reader::CODE_PATTERNS[107][7] =  {
    {2, 1, 2, 2, 2, 2, 0},
    {2, 2, 2, 1, 2, 2, 0},
    {2, 2, 2, 2, 2, 1, 0},
    {1, 2, 1, 2, 2, 3, 0},
    {1, 2, 1, 3, 2, 2, 0},
    {1, 3, 1, 2, 2, 2, 0},
    {1, 2, 2, 2, 1, 3, 0},
    {1, 2, 2, 3, 1, 2, 0},
    {1, 3, 2, 2, 1, 2, 0},
    {2, 2, 1, 2, 1, 3, 0},
    {2, 2, 1, 3, 1, 2, 0},
    {2, 3, 1, 2, 1, 2, 0},
    {1, 1, 2, 2, 3, 2, 0},
    {1, 2, 2, 1, 3, 2, 0},
    {1, 2, 2, 2, 3, 1, 0},
    {1, 1, 3, 2, 2, 2, 0},
    {1, 2, 3, 1, 2, 2, 0},
    {1, 2, 3, 2, 2, 1, 0},
    {2, 2, 3, 2, 1, 1, 0},
    {2, 2, 1, 1, 3, 2, 0},
    {2, 2, 1, 2, 3, 1, 0},
    {2, 1, 3, 2, 1, 2, 0},
    {2, 2, 3, 1, 1, 2, 0},
    {3, 1, 2, 1, 3, 1, 0},
    {3, 1, 1, 2, 2, 2, 0},
    {3, 2, 1, 1, 2, 2, 0},
    {3, 2, 1, 2, 2, 1, 0},
    {3, 1, 2, 2, 1, 2, 0},
    {3, 2, 2, 1, 1, 2, 0},
    {3, 2, 2, 2, 1, 1, 0},
    {2, 1, 2, 1, 2, 3, 0},
    {2, 1, 2, 3, 2, 1, 0},
    {2, 3, 2, 1, 2, 1, 0},
    {1, 1, 1, 3, 2, 3, 0},
    {1, 3, 1, 1, 2, 3, 0},
    {1, 3, 1, 3, 2, 1, 0},
    {1, 1, 2, 3, 1, 3, 0},
    {1, 3, 2, 1, 1, 3, 0},
    {1, 3, 2, 3, 1, 1, 0},
    {2, 1, 1, 3, 1, 3, 0},
    {2, 3, 1, 1, 1, 3, 0},
    {2, 3, 1, 3, 1, 1, 0},
    {1, 1, 2, 1, 3, 3, 0},
    {1, 1, 2, 3, 3, 1, 0},
    {1, 3, 2, 1, 3, 1, 0},
    {1, 1, 3, 1, 2, 3, 0},
    {1, 1, 3, 3, 2, 1, 0},
    {1, 3, 3, 1, 2, 1, 0},
    {3, 1, 3, 1, 2, 1, 0},
    {2, 1, 1, 3, 3, 1, 0},
    {2, 3, 1, 1, 3, 1, 0},
    {2, 1, 3, 1, 1, 3, 0},
    {2, 1, 3, 3, 1, 1, 0},
    {2, 1, 3, 1, 3, 1, 0},
    {3, 1, 1, 1, 2, 3, 0},
    {3, 1, 1, 3, 2, 1, 0},
    {3, 3, 1, 1, 2, 1, 0},
    {3, 1, 2, 1, 1, 3, 0},
    {3, 1, 2, 3, 1, 1, 0},
    {3, 3, 2, 1, 1, 1, 0},
    {3, 1, 4, 1, 1, 1, 0},
    {2, 2, 1, 4, 1, 1, 0},
    {4, 3, 1, 1, 1, 1, 0},
    {1, 1, 1, 2, 2, 4, 0},
    {1, 1, 1, 4, 2, 2, 0},
    {1, 2, 1, 1, 2, 4, 0},
    {1, 2, 1, 4, 2, 1, 0},
    {1, 4, 1, 1, 2, 2, 0},
    {1, 4, 1, 2, 2, 1, 0},
    {1, 1, 2, 2, 1, 4, 0},
    {1, 1, 2, 4, 1, 2, 0},
    {1, 2, 2, 1, 1, 4, 0},
    {1, 2, 2, 4, 1, 1, 0},
    {1, 4, 2, 1, 1, 2, 0},
    {1, 4, 2, 2, 1, 1, 0},
    {2, 4, 1, 2, 1, 1, 0},
    {2, 2, 1, 1, 1, 4, 0},
    {4, 1, 3, 1, 1, 1, 0},
    {2, 4, 1, 1, 1, 2, 0},
    {1, 3, 4, 1, 1, 1, 0},
    {1, 1, 1, 2, 4, 2, 0},
    {1, 2, 1, 1, 4, 2, 0},
    {1, 2, 1, 2, 4, 1, 0},
    {1, 1, 4, 2, 1, 2, 0},
    {1, 2, 4, 1, 1, 2, 0},
    {1, 2, 4, 2, 1, 1, 0},
    {4, 1, 1, 2, 1, 2, 0},
    {4, 2, 1, 1, 1, 2, 0},
    {4, 2, 1, 2, 1, 1, 0},
    {2, 1, 2, 1, 4, 1, 0},
    {2, 1, 4, 1, 2, 1, 0},
    {4, 1, 2, 1, 2, 1, 0},
    {1, 1, 1, 1, 4, 3, 0},
    {1, 1, 1, 3, 4, 1, 0},
    {1, 3, 1, 1, 4, 1, 0},
    {1, 1, 4, 1, 1, 3, 0},
    {1, 1, 4, 3, 1, 1, 0},
    {4, 1, 1, 1, 1, 3, 0},
    {4, 1, 1, 3, 1, 1, 0},
    {1, 1, 3, 1, 4, 1, 0},
    {1, 1, 4, 1, 3, 1, 0},
    {3, 1, 1, 1, 4, 1, 0},
    {4, 1, 1, 1, 3, 1, 0},
    {2, 1, 1, 4, 1, 2, 0},
    {2, 1, 1, 2, 1, 4, 0},
    {2, 1, 1, 2, 3, 2, 0},
    {2, 3, 3, 1, 1, 1, 2}
};
const FX_INT32 CBC_OnedCode128Reader::MAX_AVG_VARIANCE = (FX_INT32) (256 * 0.25f);
const FX_INT32 CBC_OnedCode128Reader::MAX_INDIVIDUAL_VARIANCE = (FX_INT32) (256 * 0.7f);
const FX_INT32 CBC_OnedCode128Reader::CODE_SHIFT = 98;
const FX_INT32 CBC_OnedCode128Reader::CODE_CODE_C = 99;
const FX_INT32 CBC_OnedCode128Reader::CODE_CODE_B = 100;
const FX_INT32 CBC_OnedCode128Reader::CODE_CODE_A = 101;
const FX_INT32 CBC_OnedCode128Reader::CODE_FNC_1 = 102;
const FX_INT32 CBC_OnedCode128Reader::CODE_FNC_2 = 97;
const FX_INT32 CBC_OnedCode128Reader::CODE_FNC_3 = 96;
const FX_INT32 CBC_OnedCode128Reader::CODE_FNC_4_A = 101;
const FX_INT32 CBC_OnedCode128Reader::CODE_FNC_4_B = 100;
const FX_INT32 CBC_OnedCode128Reader::CODE_START_A = 103;
const FX_INT32 CBC_OnedCode128Reader::CODE_START_B = 104;
const FX_INT32 CBC_OnedCode128Reader::CODE_START_C = 105;
const FX_INT32 CBC_OnedCode128Reader::CODE_STOP = 106;
CBC_OnedCode128Reader::CBC_OnedCode128Reader()
{
}
CBC_OnedCode128Reader::~CBC_OnedCode128Reader()
{
}
CFX_Int32Array *CBC_OnedCode128Reader::FindStartPattern(CBC_CommonBitArray *row, FX_INT32 &e)
{
    FX_INT32 width = row->GetSize();
    FX_INT32 rowOffset = 0;
    while (rowOffset < width) {
        if (row->Get(rowOffset)) {
            break;
        }
        rowOffset++;
    }
    FX_INT32 counterPosition = 0;
    CFX_Int32Array counters;
    counters.SetSize(6);
    FX_INT32 patternStart = rowOffset;
    FX_BOOL isWhite = FALSE;
    FX_INT32 patternLength = counters.GetSize();
    for (FX_INT32 i = rowOffset; i < width; i++) {
        FX_BOOL pixel = row->Get(i);
        if (pixel ^ isWhite) {
            counters[counterPosition]++;
        } else {
            if (counterPosition == patternLength - 1) {
                FX_INT32 bestVariance = MAX_AVG_VARIANCE;
                FX_INT32 bestMatch = -1;
                for (FX_INT32 startCode = CODE_START_A; startCode <= CODE_START_C; startCode++) {
                    FX_INT32 variance = PatternMatchVariance(&counters, &CODE_PATTERNS[startCode][0], MAX_INDIVIDUAL_VARIANCE);
                    if (variance < bestVariance) {
                        bestVariance = variance;
                        bestMatch = startCode;
                    }
                }
                if (bestMatch >= 0) {
                    FX_BOOL btemp2 = row->IsRange(FX_MAX(0, patternStart - (i - patternStart) / 2), patternStart, FALSE, e);
                    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
                    if (btemp2) {
                        CFX_Int32Array *result = FX_NEW CFX_Int32Array;
                        result->SetSize(3);
                        (*result)[0] = patternStart;
                        (*result)[1] = i;
                        (*result)[2] = bestMatch;
                        return result;
                    }
                }
                patternStart += counters[0] + counters[1];
                for (FX_INT32 y = 2; y < patternLength; y++) {
                    counters[y - 2] = counters[y];
                }
                counters[patternLength - 2] = 0;
                counters[patternLength - 1] = 0;
                counterPosition--;
            } else {
                counterPosition++;
            }
            counters[counterPosition] = 1;
            isWhite = !isWhite;
        }
    }
    e = BCExceptionNotFound;
    return NULL;
}
FX_INT32 CBC_OnedCode128Reader::DecodeCode(CBC_CommonBitArray *row, CFX_Int32Array *counters, FX_INT32 rowOffset, FX_INT32 &e)
{
    RecordPattern(row, rowOffset, counters, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, 0);
    FX_INT32 bestVariance = MAX_AVG_VARIANCE;
    FX_INT32 bestMatch = -1;
    for (FX_INT32 d = 0; d < 107; d++) {
        FX_INT32 variance = PatternMatchVariance(counters, &CODE_PATTERNS[d][0], MAX_INDIVIDUAL_VARIANCE);
        if (variance < bestVariance) {
            bestVariance = variance;
            bestMatch = d;
        }
    }
    if (bestMatch >= 0) {
        return bestMatch;
    } else {
        e = BCExceptionNotFound;
        return 0;
    }
    return 0;
}
CFX_ByteString CBC_OnedCode128Reader::DecodeRow(FX_INT32 rowNumber, CBC_CommonBitArray *row, FX_INT32 hints, FX_INT32 &e)
{
    CFX_Int32Array *startPatternInfo = FindStartPattern(row, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    FX_INT32 startCode = (*startPatternInfo)[2];
    FX_INT32 codeSet;
    switch (startCode) {
        case 103:
            codeSet = CODE_CODE_A;
            break;
        case 104:
            codeSet = CODE_CODE_B;
            break;
        case 105:
            codeSet = CODE_CODE_C;
            break;
        default:
            if(startPatternInfo != NULL) {
                startPatternInfo->RemoveAll();
                delete startPatternInfo;
                startPatternInfo = NULL;
            }
            e = BCExceptionFormatException;
            return "";
    }
    FX_BOOL done = FALSE;
    FX_BOOL isNextShifted = FALSE;
    CFX_ByteString result;
    FX_INT32 lastStart = (*startPatternInfo)[0];
    FX_INT32 nextStart = (*startPatternInfo)[1];
    if(startPatternInfo != NULL) {
        startPatternInfo->RemoveAll();
        delete startPatternInfo;
        startPatternInfo = NULL;
    }
    CFX_Int32Array counters;
    counters.SetSize(6);
    FX_INT32 lastCode = 0;
    FX_INT32 code = 0;
    FX_INT32 checksumTotal = startCode;
    FX_INT32 multiplier = 0;
    FX_BOOL lastCharacterWasPrintable = TRUE;
    while (!done) {
        FX_BOOL unshift = isNextShifted;
        isNextShifted = FALSE;
        lastCode = code;
        code = DecodeCode(row, &counters, nextStart, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, "");
        if (code != CODE_STOP) {
            lastCharacterWasPrintable = TRUE;
        }
        if (code != CODE_STOP) {
            multiplier++;
            checksumTotal += multiplier * code;
        }
        lastStart = nextStart;
        for (FX_INT32 i = 0; i < counters.GetSize(); i++) {
            nextStart += counters[i];
        }
        switch (code) {
            case 103:
            case 104:
            case 105:
                e = BCExceptionFormatException;
                return "";
        }
        switch (codeSet) {
            case 101:
                if (code < 64) {
                    result += (FX_CHAR) (' ' + code);
                } else if (code < 96) {
                    result += (FX_CHAR) (code - 64);
                } else {
                    if (code != CODE_STOP) {
                        lastCharacterWasPrintable = FALSE;
                    }
                    switch (code) {
                        case 102:
                        case 97:
                        case 96:
                        case 101:
                            break;
                        case 98:
                            isNextShifted = TRUE;
                            codeSet = CODE_CODE_B;
                            break;
                        case 100:
                            codeSet = CODE_CODE_B;
                            break;
                        case 99:
                            codeSet = CODE_CODE_C;
                            break;
                        case 106:
                            done = TRUE;
                            break;
                    }
                }
                break;
            case 100:
                if (code < 96) {
                    result += (FX_CHAR) (' ' + code);
                } else {
                    if (code != CODE_STOP) {
                        lastCharacterWasPrintable = FALSE;
                    }
                    switch (code) {
                        case 102:
                        case 97:
                        case 96:
                        case 100:
                            break;
                        case 98:
                            isNextShifted = TRUE;
                            codeSet = CODE_CODE_A;
                            break;
                        case 101:
                            codeSet = CODE_CODE_A;
                            break;
                        case 99:
                            codeSet = CODE_CODE_C;
                            break;
                        case 106:
                            done = TRUE;
                            break;
                    }
                }
                break;
            case 99:
                if (code < 100) {
                    if (code < 10) {
                        result += '0';
                    }
                    FX_CHAR temp[128];
#if defined(_FX_WINAPI_PARTITION_APP_)
                    sprintf_s(temp, 128, "%d", code);
#else
                    sprintf(temp, "%d", code);
#endif
                    result += temp;
                } else {
                    if (code != CODE_STOP) {
                        lastCharacterWasPrintable = FALSE;
                    }
                    switch (code) {
                        case 102:
                            break;
                        case 101:
                            codeSet = CODE_CODE_A;
                            break;
                        case 100:
                            codeSet = CODE_CODE_B;
                            break;
                        case 106:
                            done = TRUE;
                            break;
                    }
                }
                break;
        }
        if (unshift) {
            codeSet = codeSet == CODE_CODE_A ? CODE_CODE_B : CODE_CODE_A;
        }
    }
    FX_INT32 width = row->GetSize();
    while (nextStart < width && row->Get(nextStart)) {
        nextStart++;
    }
    FX_BOOL boolT1 = row->IsRange(nextStart, FX_MIN(width, nextStart + (nextStart - lastStart) / 2), FALSE, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    if (!boolT1) {
        e = BCExceptionNotFound;
        return "";
    }
    checksumTotal -= multiplier * lastCode;
    if (checksumTotal % 103 != lastCode) {
        e = BCExceptionChecksumException;
        return "";
    }
    FX_INT32 resultLength = result.GetLength();
    if (resultLength > 0 && lastCharacterWasPrintable) {
        if (codeSet == CODE_CODE_C) {
            result = result.Mid(0, result.GetLength() - 2);
        } else {
            result = result.Mid(0, result.GetLength() - 1);
        }
    }
    if (result.GetLength() == 0) {
        e = BCExceptionFormatException;
        return "";
    }
    return result;
}
