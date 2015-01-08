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

#include "barcode.h"
#include "include/BC_Reader.h"
#include "include/BC_OneDReader.h"
#include "include/BC_OnedCode39Reader.h"
#include "include/BC_CommonBitArray.h"
#include "include/BC_OnedCodaBarReader.h"
FX_LPCSTR CBC_OnedCodaBarReader::ALPHABET_STRING = "0123456789-$:/.+ABCDTN";
const FX_INT32 CBC_OnedCodaBarReader::CHARACTER_ENCODINGS[22] = {
    0x003, 0x006, 0x009, 0x060, 0x012, 0x042, 0x021, 0x024, 0x030, 0x048,
    0x00c, 0x018, 0x045, 0x051, 0x054, 0x015, 0x01A, 0x029, 0x00B, 0x00E,
    0x01A, 0x029
};
const FX_INT32 CBC_OnedCodaBarReader::minCharacterLength = 3;
const FX_CHAR CBC_OnedCodaBarReader::STARTEND_ENCODING[8] = {'E', '*', 'A', 'B', 'C', 'D', 'T', 'N'};
CBC_OnedCodaBarReader::CBC_OnedCodaBarReader()
{
}
CBC_OnedCodaBarReader::~CBC_OnedCodaBarReader()
{
}
CFX_ByteString CBC_OnedCodaBarReader::DecodeRow(FX_INT32 rowNumber, CBC_CommonBitArray *row, FX_INT32 hints, FX_INT32 &e)
{
    CFX_Int32Array *int32Ptr = FindAsteriskPattern(row, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    CBC_AutoPtr<CFX_Int32Array> start(int32Ptr);
    (*start)[1] = 0;
    FX_INT32 nextStart = (*start)[1];
    FX_INT32 end = row->GetSize();
    while (nextStart < end && !row->Get(nextStart)) {
        nextStart++;
    }
    CFX_ByteString result;
    CFX_Int32Array counters;
    counters.SetSize(7);
    FX_CHAR decodedChar;
    FX_INT32 lastStart;
    do {
        RecordPattern(row, nextStart, &counters, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, "");
        decodedChar = ToNarrowWidePattern(&counters);
        if (decodedChar == '!') {
            e = BCExceptionNotFound;
            return "";
        }
        result += decodedChar;
        lastStart = nextStart;
        for (FX_INT32 i = 0; i < counters.GetSize(); i++) {
            nextStart += counters[i];
        }
        while (nextStart < end && !row->Get(nextStart)) {
            nextStart++;
        }
    } while (nextStart < end);
    FX_INT32 lastPatternSize = 0;
    for (FX_INT32 j = 0; j < counters.GetSize(); j++) {
        lastPatternSize += counters[j];
    }
    FX_INT32 whiteSpaceAfterEnd = nextStart - lastStart - lastPatternSize;
    if (nextStart != end && (whiteSpaceAfterEnd / 2 < lastPatternSize)) {
        e = BCExceptionNotFound;
        return "";
    }
    if (result.GetLength() < 2) {
        e = BCExceptionNotFound;
        return "";
    }
    FX_CHAR startchar = result[0];
    if (!ArrayContains(STARTEND_ENCODING, startchar)) {
        e =  BCExceptionNotFound;
        return "";
    }
    FX_INT32 len = result.GetLength();
    CFX_ByteString temp = result;
    for (FX_INT32 k = 1; k < result.GetLength(); k++) {
        if (ArrayContains(STARTEND_ENCODING, result[k])) {
            if ((k + 1) != result.GetLength()) {
                result.Delete(1, k);
                k = 1;
            }
        }
    }
    if (result.GetLength() < 5) {
        FX_INT32 index = temp.Find(result.Mid(1, result.GetLength() - 1));
        if (index == len - (result.GetLength() - 1)) {
            e = BCExceptionNotFound;
            return "";
        }
    }
    if (result.GetLength() > minCharacterLength) {
        result = result.Mid(1, result.GetLength() - 2);
    } else {
        e = BCExceptionNotFound;
        return "";
    }
    return result;
}
CFX_Int32Array *CBC_OnedCodaBarReader::FindAsteriskPattern(CBC_CommonBitArray *row, FX_INT32 &e)
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
    counters.SetSize(7);
    FX_INT32 patternStart = rowOffset;
    FX_BOOL isWhite = FALSE;
    FX_INT32 patternLength = counters.GetSize();
    for (FX_INT32 i = rowOffset; i < width; i++) {
        FX_BOOL pixel = row->Get(i);
        if (pixel ^ isWhite) {
            counters[counterPosition]++;
        } else {
            if (counterPosition == patternLength - 1) {
                if (ArrayContains(STARTEND_ENCODING, ToNarrowWidePattern(&counters))) {
                    FX_BOOL btemp3 = row->IsRange(FX_MAX(0, patternStart - (i - patternStart) / 2), patternStart, FALSE, e);
                    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
                    if (btemp3) {
                        CFX_Int32Array *result = FX_NEW CFX_Int32Array();
                        result->SetSize(2);
                        (*result)[0] = patternStart;
                        (*result)[1] = i;
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
FX_BOOL CBC_OnedCodaBarReader::ArrayContains(const FX_CHAR array[], FX_CHAR key)
{
    for(FX_INT32 i = 0; i < 8; i++) {
        if(array[i] == key) {
            return TRUE;
        }
    }
    return FALSE;
}
FX_CHAR CBC_OnedCodaBarReader::ToNarrowWidePattern(CFX_Int32Array *counter)
{
    FX_INT32 numCounters = counter->GetSize();
    if (numCounters < 1) {
        return '!';
    }
    FX_INT32 averageCounter = 0;
    FX_INT32 totalCounters = 0;
    for (FX_INT32 i = 0; i < numCounters; i++) {
        totalCounters += (*counter)[i];
    }
    averageCounter = totalCounters / numCounters;
    FX_INT32 pattern = 0;
    FX_INT32 wideCounters = 0;
    for (FX_INT32 j = 0; j < numCounters; j++) {
        if ((*counter)[j] > averageCounter) {
            pattern |= 1 << (numCounters - 1 - j);
            wideCounters++;
        }
    }
    if ((wideCounters == 2) || (wideCounters == 3)) {
        for (FX_INT32 k = 0; k < 22; k++) {
            if (CHARACTER_ENCODINGS[k] == pattern) {
                return (ALPHABET_STRING)[k];
            }
        }
    }
    return '!';
}
