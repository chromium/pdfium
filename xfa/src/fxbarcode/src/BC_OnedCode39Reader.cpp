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
#include "include/BC_CommonBitArray.h"
#include "include/BC_OnedCode39Reader.h"
FX_LPCSTR CBC_OnedCode39Reader::ALPHABET_STRING = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. *$/+%";
FX_LPCSTR CBC_OnedCode39Reader::CHECKSUM_STRING = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%";
const FX_INT32 CBC_OnedCode39Reader::CHARACTER_ENCODINGS[44] = {
    0x034, 0x121, 0x061, 0x160, 0x031, 0x130, 0x070, 0x025, 0x124, 0x064,
    0x109, 0x049, 0x148, 0x019, 0x118, 0x058, 0x00D, 0x10C, 0x04C, 0x01C,
    0x103, 0x043, 0x142, 0x013, 0x112, 0x052, 0x007, 0x106, 0x046, 0x016,
    0x181, 0x0C1, 0x1C0, 0x091, 0x190, 0x0D0, 0x085, 0x184, 0x0C4, 0x094,
    0x0A8, 0x0A2, 0x08A, 0x02A
};
const FX_INT32 CBC_OnedCode39Reader::ASTERISK_ENCODING = 0x094;
CBC_OnedCode39Reader::CBC_OnedCode39Reader(): m_extendedMode(FALSE), m_usingCheckDigit(FALSE)
{
}
CBC_OnedCode39Reader::CBC_OnedCode39Reader(FX_BOOL usingCheckDigit)
{
    m_usingCheckDigit = usingCheckDigit;
    m_extendedMode = FALSE;
}
CBC_OnedCode39Reader::CBC_OnedCode39Reader(FX_BOOL usingCheckDigit, FX_BOOL extendedMode)
{
    m_extendedMode = extendedMode;
    m_usingCheckDigit = usingCheckDigit;
}
CBC_OnedCode39Reader::~CBC_OnedCode39Reader()
{
}
CFX_ByteString CBC_OnedCode39Reader::DecodeRow(FX_INT32 rowNumber, CBC_CommonBitArray *row, FX_INT32 hints, FX_INT32 &e)
{
    CFX_Int32Array *start = FindAsteriskPattern(row, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    FX_INT32 nextStart = (*start)[1];
    if(start != NULL) {
        delete start;
        start = NULL;
    }
    FX_INT32 end = row->GetSize();
    while (nextStart < end && !row->Get(nextStart)) {
        nextStart++;
    }
    CFX_ByteString result;
    CFX_Int32Array counters;
    counters.SetSize(9);
    FX_CHAR decodedChar;
    FX_INT32 lastStart;
    do {
        RecordPattern(row, nextStart, &counters, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, "");
        FX_INT32 pattern = ToNarrowWidePattern(&counters);
        if (pattern < 0) {
            e = BCExceptionNotFound;
            return "";
        }
        decodedChar = PatternToChar(pattern, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, "");
        result += decodedChar;
        lastStart = nextStart;
        for (FX_INT32 i = 0; i < counters.GetSize(); i++) {
            nextStart += counters[i];
        }
        while (nextStart < end && !row->Get(nextStart)) {
            nextStart++;
        }
    } while (decodedChar != '*');
    result = result.Mid(0, result.GetLength() - 1);
    FX_INT32 lastPatternSize = 0;
    for (FX_INT32 j = 0; j < counters.GetSize(); j++) {
        lastPatternSize += counters[j];
    }
    FX_INT32 whiteSpaceAfterEnd = nextStart - lastStart - lastPatternSize;
    if(m_usingCheckDigit) {
        FX_INT32 max = result.GetLength() - 1;
        FX_INT32 total = 0;
        FX_INT32 len = (FX_INT32)strlen(ALPHABET_STRING);
        for (FX_INT32 k = 0; k < max; k++) {
            for (FX_INT32 j = 0; j < len; j++)
                if (ALPHABET_STRING[j] == result[k]) {
                    total += j;
                }
        }
        if (result[max] != (ALPHABET_STRING)[total % 43]) {
            e = BCExceptionChecksumException;
            return "";
        }
        result = result.Mid(0, result.GetLength() - 1);
    }
    if (result.GetLength() == 0) {
        e  = BCExceptionNotFound;
        return "";
    }
    if(m_extendedMode) {
        CFX_ByteString bytestr = DecodeExtended(result, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, "");
        return bytestr;
    } else {
        return result;
    }
}
CFX_Int32Array *CBC_OnedCode39Reader::FindAsteriskPattern(CBC_CommonBitArray *row, FX_INT32 &e)
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
    counters.SetSize(9);
    FX_INT32 patternStart = rowOffset;
    FX_BOOL isWhite = FALSE;
    FX_INT32 patternLength = counters.GetSize();
    for (FX_INT32 i = rowOffset; i < width; i++) {
        FX_BOOL pixel = row->Get(i);
        if (pixel ^ isWhite) {
            counters[counterPosition]++;
        } else {
            if (counterPosition == patternLength - 1) {
                if (ToNarrowWidePattern(&counters) == ASTERISK_ENCODING) {
                    FX_BOOL bT1 =  row->IsRange(FX_MAX(0, patternStart - (i - patternStart) / 2), patternStart, FALSE, e);
                    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
                    if (bT1) {
                        CFX_Int32Array *result = FX_NEW CFX_Int32Array;
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
FX_INT32 CBC_OnedCode39Reader::ToNarrowWidePattern(CFX_Int32Array *counters)
{
    FX_INT32 numCounters = counters->GetSize();
    FX_INT32 maxNarrowCounter = 0;
    FX_INT32 wideCounters;
    do {
#undef max
        FX_INT32 minCounter = FXSYS_IntMax;
        for (FX_INT32 i = 0; i < numCounters; i++) {
            FX_INT32 counter = (*counters)[i];
            if (counter < minCounter && counter > maxNarrowCounter) {
                minCounter = counter;
            }
        }
        maxNarrowCounter = minCounter;
        wideCounters = 0;
        FX_INT32 totalWideCountersWidth = 0;
        FX_INT32 pattern = 0;
        for (FX_INT32 j = 0; j < numCounters; j++) {
            FX_INT32 counter = (*counters)[j];
            if ((*counters)[j] > maxNarrowCounter) {
                pattern |= 1 << (numCounters - 1 - j);
                wideCounters++;
                totalWideCountersWidth += counter;
            }
        }
        if (wideCounters == 3) {
            for (FX_INT32 k = 0; k < numCounters && wideCounters > 0; k++) {
                FX_INT32 counter = (*counters)[k];
                if ((*counters)[k] > maxNarrowCounter) {
                    wideCounters--;
                    if ((counter << 1) >= totalWideCountersWidth) {
                        return -1;
                    }
                }
            }
            return pattern;
        }
    } while (wideCounters > 3);
    return -1;
}
FX_CHAR CBC_OnedCode39Reader::PatternToChar(FX_INT32 pattern, FX_INT32 &e)
{
    for (FX_INT32 i = 0; i < 44; i++) {
        if (CHARACTER_ENCODINGS[i] == pattern) {
            return (ALPHABET_STRING)[i];
        }
    }
    e = BCExceptionNotFound;
    return 0;
}
CFX_ByteString CBC_OnedCode39Reader::DecodeExtended(CFX_ByteString &encoded, FX_INT32 &e)
{
    FX_INT32 length = encoded.GetLength();
    CFX_ByteString decoded;
    FX_CHAR c, next;
    for(FX_INT32 i = 0; i < length; i++) {
        c = encoded[i];
        if(c == '+' || c == '$' || c == '%' || c == '/') {
            next = encoded[i + 1];
            FX_CHAR decodedChar = '\0';
            switch (c) {
                case '+':
                    if (next >= 'A' && next <= 'Z') {
                        decodedChar = (FX_CHAR) (next + 32);
                    } else {
                        e = BCExceptionFormatException;
                        return "";
                    }
                    break;
                case '$':
                    if (next >= 'A' && next <= 'Z') {
                        decodedChar = (FX_CHAR) (next - 64);
                    } else {
                        e = BCExceptionFormatException;
                        return "";
                    }
                    break;
                case '%':
                    if (next >= 'A' && next <= 'E') {
                        decodedChar = (FX_CHAR) (next - 38);
                    } else if (next >= 'F' && next <= 'J') {
                        decodedChar = (FX_CHAR) (next - 11);
                    } else if (next >= 'K' && next <= 'O' && next != 'M' && next != 'N') {
                        decodedChar = (FX_CHAR) (next + 16);
                    } else if (next >= 'P' && next <= 'S') {
                        decodedChar = (FX_CHAR) (next + 43);
                    } else if (next == 'U') {
                        decodedChar = (FX_CHAR) 0;
                    } else if (next == 'V') {
                        decodedChar = (FX_CHAR) 64;
                    } else if (next == 'W') {
                        decodedChar = (FX_CHAR) 96;
                    } else if (next == 'T' || next == 'X' || next == 'Y' || next == 'Z') {
                        decodedChar = (FX_CHAR) 127;
                    } else {
                        e = BCExceptionFormatException;
                        return "";
                    }
                    break;
                case '/':
                    if (next >= 'A' && next <= 'O') {
                        decodedChar = (FX_CHAR) (next - 32);
                    } else if (next == 'Z') {
                        decodedChar = ':';
                    } else {
                        e = BCExceptionFormatException;
                        return "";
                    }
                    break;
            }
            decoded += decodedChar;
            i++;
        } else {
            decoded += c;
        }
    }
    return decoded;
}
