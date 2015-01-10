// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2008 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../barcode.h"
#include "../BC_Reader.h"
#include "../common/BC_CommonBitArray.h"
#include "BC_OneDReader.h"
#include "BC_OneDimReader.h"
const FX_INT32 CBC_OneDimReader::MAX_AVG_VARIANCE = (FX_INT32)(256 * 0.48f);
const FX_INT32 CBC_OneDimReader::MAX_INDIVIDUAL_VARIANCE = (FX_INT32)(256 * 0.7f);
const FX_INT32 CBC_OneDimReader::START_END_PATTERN[3] = {1, 1, 1};
const FX_INT32 CBC_OneDimReader::MIDDLE_PATTERN[5] = {1, 1, 1, 1, 1};
const FX_INT32 CBC_OneDimReader::L_PATTERNS[10][4] = {
    {3, 2, 1, 1},
    {2, 2, 2, 1},
    {2, 1, 2, 2},
    {1, 4, 1, 1},
    {1, 1, 3, 2},
    {1, 2, 3, 1},
    {1, 1, 1, 4},
    {1, 3, 1, 2},
    {1, 2, 1, 3},
    {3, 1, 1, 2}
};
const FX_INT32 CBC_OneDimReader::L_AND_G_PATTERNS[20][4] = {
    {3, 2, 1, 1},
    {2, 2, 2, 1},
    {2, 1, 2, 2},
    {1, 4, 1, 1},
    {1, 1, 3, 2},
    {1, 2, 3, 1},
    {1, 1, 1, 4},
    {1, 3, 1, 2},
    {1, 2, 1, 3},
    {3, 1, 1, 2},
    {1, 1, 2, 3},
    {1, 2, 2, 2},
    {2, 2, 1, 2},
    {1, 1, 4, 1},
    {2, 3, 1, 1},
    {1, 3, 2, 1},
    {4, 1, 1, 1},
    {2, 1, 3, 1},
    {3, 1, 2, 1},
    {2, 1, 1, 3}
};
CBC_OneDimReader::CBC_OneDimReader()
{
}
CBC_OneDimReader::~CBC_OneDimReader()
{
}
CFX_Int32Array *CBC_OneDimReader::FindStartGuardPattern(CBC_CommonBitArray *row, FX_INT32 &e)
{
    FX_BOOL foundStart = FALSE;
    CFX_Int32Array *startRange = NULL;
    CFX_Int32Array startEndPattern;
    startEndPattern.SetSize(3);
    startEndPattern[0] = START_END_PATTERN[0];
    startEndPattern[1] = START_END_PATTERN[1];
    startEndPattern[2] = START_END_PATTERN[2];
    FX_INT32 nextStart = 0;
    while (!foundStart) {
        if(startRange != NULL) {
            delete startRange;
            startRange = NULL;
        }
        startRange = FindGuardPattern(row, nextStart, FALSE, &startEndPattern, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        FX_INT32 start = (*startRange)[0];
        nextStart = (*startRange)[1];
        if (start <= 1) {
            break;
        }
        FX_INT32 quietStart = start - (nextStart - start);
        if (quietStart >= 0) {
            FX_BOOL booT = row->IsRange(quietStart, start, FALSE, e);
            BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
            foundStart = booT;
        }
    }
    return startRange;
}
CFX_ByteString CBC_OneDimReader::DecodeRow(FX_INT32 rowNumber, CBC_CommonBitArray *row, FX_INT32 hints, FX_INT32 &e)
{
    CFX_Int32Array* StartPattern = FindStartGuardPattern(row, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    CBC_AutoPtr<CFX_Int32Array > result(StartPattern);
    CFX_ByteString temp = DecodeRow(rowNumber, row, result.get(), hints, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    return temp;
}
CFX_ByteString CBC_OneDimReader::DecodeRow(FX_INT32 rowNumber, CBC_CommonBitArray *row, CFX_Int32Array *startGuardRange, FX_INT32 hints, FX_INT32 &e)
{
    CFX_ByteString result;
    FX_INT32 endStart = DecodeMiddle(row, startGuardRange, result, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    FX_BOOL b = CheckChecksum(result, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    if (!b) {
        e = BCExceptionChecksumException;
        return "";
    }
    return result;
}
FX_BOOL CBC_OneDimReader::CheckChecksum(CFX_ByteString &s, FX_INT32 &e)
{
    FX_BOOL temp = CheckStandardUPCEANChecksum(s, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
    return temp;
}
FX_BOOL CBC_OneDimReader::CheckStandardUPCEANChecksum(CFX_ByteString &s, FX_INT32 &e)
{
    FX_INT32 length = s.GetLength();
    if (length == 0) {
        return FALSE;
    }
    FX_INT32 sum = 0;
    for (FX_INT32 i = length - 2; i >= 0; i -= 2) {
        FX_INT32 digit = (FX_INT32) s[i] - (FX_INT32) '0';
        if (digit < 0 || digit > 9) {
            e = BCExceptionFormatException;
            return FALSE;
        }
        sum += digit;
    }
    sum *= 3;
    for (FX_INT32 j = length - 1; j >= 0; j -= 2) {
        FX_INT32 digit = (FX_INT32) s[j] - (FX_INT32) '0';
        if (digit < 0 || digit > 9) {
            e = BCExceptionFormatException;
            return FALSE;
        }
        sum += digit;
    }
    return sum % 10 == 0;
}
CFX_Int32Array *CBC_OneDimReader::DecodeEnd(CBC_CommonBitArray* row, FX_INT32 endStart, FX_INT32 &e)
{
    CFX_Int32Array startEndPattern;
    startEndPattern.Add(START_END_PATTERN[0]);
    startEndPattern.Add(START_END_PATTERN[1]);
    startEndPattern.Add(START_END_PATTERN[2]);
    CFX_Int32Array* FindGuard = FindGuardPattern(row, endStart, FALSE, &startEndPattern, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return FindGuard;
}
CFX_Int32Array *CBC_OneDimReader::FindGuardPattern(CBC_CommonBitArray *row, FX_INT32 rowOffset, FX_BOOL whiteFirst, CFX_Int32Array *pattern, FX_INT32 &e)
{
    FX_INT32 patternLength = pattern->GetSize();
    CFX_Int32Array counters;
    counters.SetSize(patternLength);
    FX_INT32 width = row->GetSize();
    FX_BOOL isWhite = FALSE;
    while (rowOffset < width) {
        isWhite = !row->Get(rowOffset);
        if (whiteFirst == isWhite) {
            break;
        }
        rowOffset++;
    }
    FX_INT32 counterPosition = 0;
    FX_INT32 patternStart = rowOffset;
    for (FX_INT32 x = rowOffset; x < width; x++) {
        FX_BOOL pixel = row->Get(x);
        if (pixel ^ isWhite) {
            counters[counterPosition]++;
        } else {
            if (counterPosition == patternLength - 1) {
                if (PatternMatchVariance(&counters, &(*pattern)[0], MAX_INDIVIDUAL_VARIANCE) < MAX_AVG_VARIANCE) {
                    CFX_Int32Array *result = FX_NEW CFX_Int32Array();
                    result->SetSize(2);
                    (*result)[0] = patternStart;
                    (*result)[1] = x;
                    return result;
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
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return NULL;
}
FX_INT32 CBC_OneDimReader::DecodeDigit(CBC_CommonBitArray *row, CFX_Int32Array *counters, FX_INT32 rowOffset, const FX_INT32* patterns, FX_INT32 patternLength, FX_INT32 &e)
{
    RecordPattern(row, rowOffset, counters, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, 0);
    FX_INT32 bestVariance = MAX_AVG_VARIANCE;
    FX_INT32 bestMatch = -1;
    FX_INT32 max = patternLength;
    for (FX_INT32 i = 0; i < max; i++) {
        FX_INT32 variance = PatternMatchVariance(counters, &patterns[i * 4], MAX_INDIVIDUAL_VARIANCE);
        if (variance < bestVariance) {
            bestVariance = variance;
            bestMatch = i;
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
