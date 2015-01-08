// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2010 ZXing authors
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
#include "include/BC_Writer.h"
#include "include/BC_Reader.h"
#include "include/BC_OneDReader.h"
#include "include/BC_OneDimWriter.h"
#include "include/BC_OnedCode128Reader.h"
#include "include/BC_OnedCode128Writer.h"
const FX_INT32 CBC_OnedCode128Writer::CODE_CODE_B = 100;
const FX_INT32 CBC_OnedCode128Writer::CODE_CODE_C = 99;
const FX_INT32 CBC_OnedCode128Writer::CODE_START_B = 104;
const FX_INT32 CBC_OnedCode128Writer::CODE_START_C = 105;
const FX_INT32 CBC_OnedCode128Writer::CODE_STOP = 106;
CBC_OnedCode128Writer::CBC_OnedCode128Writer()
{
    m_codeFormat = BC_CODE128_B;
}
CBC_OnedCode128Writer::CBC_OnedCode128Writer(BC_TYPE type)
{
    m_codeFormat = type;
}
CBC_OnedCode128Writer::~CBC_OnedCode128Writer()
{
}
BC_TYPE CBC_OnedCode128Writer::GetType()
{
    return m_codeFormat;
}
FX_BOOL	CBC_OnedCode128Writer::CheckContentValidity(FX_WSTR contents)
{
    FX_BOOL ret = TRUE;
    FX_INT32 position = 0;
    FX_INT32 patternIndex = -1;
    if (m_codeFormat == BC_CODE128_B || m_codeFormat == BC_CODE128_C) {
        while (position < contents.GetLength()) {
            patternIndex = (FX_INT32)contents.GetAt(position);
            if (patternIndex >= 32 && patternIndex <= 126 && patternIndex != 34) {
                position++;
                continue;
            } else {
                ret = FALSE;
                break;
            }
            position ++;
        }
    } else {
        ret = FALSE;
    }
    return ret;
}
CFX_WideString CBC_OnedCode128Writer::FilterContents(FX_WSTR contents)
{
    CFX_WideString filterChineseChar;
    FX_WCHAR ch;
    for (FX_INT32 i = 0; i < contents.GetLength(); i++) {
        ch = contents.GetAt(i);
        if(ch > 175) {
            i++;
            continue;
        }
        filterChineseChar += ch;
    }
    CFX_WideString filtercontents;
    if (m_codeFormat == BC_CODE128_B) {
        for (FX_INT32 i = 0; i < filterChineseChar.GetLength(); i++) {
            ch = filterChineseChar.GetAt(i);
            if (ch >= 32 && ch <= 126) {
                filtercontents += ch;
            } else {
                continue;
            }
        }
    } else if (m_codeFormat == BC_CODE128_C) {
        for (FX_INT32 i = 0; i < filterChineseChar.GetLength(); i++) {
            ch = filterChineseChar.GetAt(i);
            if (ch >= 32 && ch <= 106) {
                filtercontents += ch;
            } else {
                continue;
            }
        }
    } else {
        filtercontents = contents;
    }
    return filtercontents;
}
FX_BOOL CBC_OnedCode128Writer::SetTextLocation(BC_TEXT_LOC location)
{
    if ( location < BC_TEXT_LOC_NONE || location > BC_TEXT_LOC_BELOWEMBED) {
        return FALSE;
    }
    m_locTextLoc = location;
    return TRUE;
}
FX_BYTE *CBC_OnedCode128Writer::Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 hints, FX_INT32 &e)
{
    if(format != BCFORMAT_CODE_128) {
        e = BCExceptionOnlyEncodeCODE_128;
        return NULL;
    }
    FX_BYTE *ret = CBC_OneDimWriter::Encode(contents, format, outWidth, outHeight, hints, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return ret;
}
FX_BYTE *CBC_OnedCode128Writer::Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e)
{
    FX_BYTE *ret =  Encode(contents, format, outWidth, outHeight, 0, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return ret;
}
FX_BOOL CBC_OnedCode128Writer::IsDigits(const CFX_ByteString &contents, FX_INT32 start, FX_INT32 length)
{
    FX_INT32 end = start + length;
    for (FX_INT32 i = start; i < end; i++) {
        if (contents[i] < '0' || contents[i] > '9') {
            return FALSE;
        }
    }
    return TRUE;
}
FX_BYTE *CBC_OnedCode128Writer::Encode(const CFX_ByteString &contents, FX_INT32 &outLength, FX_INT32 &e)
{
    FX_INT32 length = contents.GetLength();
    if(contents.GetLength() < 1 || contents.GetLength() > 80) {
        e = BCExceptionContentsLengthShouldBetween1and80;
        return NULL;
    }
    CFX_PtrArray patterns;
    FX_INT32 checkSum = 0;
    if (m_codeFormat == BC_CODE128_B) {
        checkSum = Encode128B(contents, patterns);
    } else if (m_codeFormat == BC_CODE128_C) 	{
        checkSum = Encode128C(contents, patterns);
    } else {
        e = BCExceptionFormatException;
        return NULL;
    }
    checkSum %= 103;
    patterns.Add((FX_INT32*)CBC_OnedCode128Reader::CODE_PATTERNS[checkSum]);
    patterns.Add((FX_INT32*)CBC_OnedCode128Reader::CODE_PATTERNS[CODE_STOP]);
    m_iContentLen = contents.GetLength() + 3;
    FX_INT32 codeWidth = 0;
    for(FX_INT32 k = 0; k < patterns.GetSize(); k++) {
        FX_INT32 *pattern = (FX_INT32*)patterns[k];
        for(FX_INT32 j = 0; j < 7; j++) {
            codeWidth += pattern[j];
        }
    }
    outLength = codeWidth;
    FX_BYTE *result = FX_Alloc(FX_BYTE, outLength);
    FX_INT32 pos = 0;
    for(FX_INT32 j = 0; j < patterns.GetSize(); j++) {
        FX_INT32* pattern = (FX_INT32*)patterns[j];
        pos += AppendPattern(result, pos, pattern, 7, 1, e);
        if (e != BCExceptionNO) {
            FX_Free (result);
            return NULL;
        }
    }
    return result;
}
FX_INT32 CBC_OnedCode128Writer::Encode128B(const CFX_ByteString &contents,  CFX_PtrArray &patterns)
{
    FX_INT32 checkSum = 0;
    FX_INT32 checkWeight = 1;
    FX_INT32 position = 0;
    patterns.Add((FX_INT32*)CBC_OnedCode128Reader::CODE_PATTERNS[CODE_START_B]);
    checkSum += CODE_START_B * checkWeight;
    while (position < contents.GetLength()) {
        FX_INT32 patternIndex = 0;
        patternIndex = contents[position] - ' ';
        position += 1;
        patterns.Add((FX_INT32*)CBC_OnedCode128Reader::CODE_PATTERNS[patternIndex]);
        checkSum += patternIndex * checkWeight;
        if (position != 0) {
            checkWeight++;
        }
    }
    return checkSum;
}
FX_INT32 CBC_OnedCode128Writer::Encode128C(const CFX_ByteString &contents,  CFX_PtrArray &patterns)
{
    FX_INT32 checkSum = 0;
    FX_INT32 checkWeight = 1;
    FX_INT32 position = 0;
    patterns.Add((FX_INT32*)CBC_OnedCode128Reader::CODE_PATTERNS[CODE_START_C]);
    checkSum += CODE_START_C * checkWeight;
    while (position < contents.GetLength()) {
        FX_INT32 patternIndex = 0;
        FX_CHAR ch = contents.GetAt(position);
        if (ch < '0' || ch > '9') {
            patternIndex = (FX_INT32)ch;
            position++;
        } else {
            patternIndex = FXSYS_atoi(contents.Mid(position, 2));
            if (contents.GetAt(position + 1) < '0' || contents.GetAt(position + 1) > '9') {
                position += 1;
            } else {
                position += 2;
            }
        }
        patterns.Add((FX_INT32*)CBC_OnedCode128Reader::CODE_PATTERNS[patternIndex]);
        checkSum += patternIndex * checkWeight;
        if (position != 0) {
            checkWeight++;
        }
    }
    return checkSum;
}
