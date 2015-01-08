// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2011 ZXing authors
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
#include "include/BC_CommonBitArray.h"
#include "include/BC_OneDimWriter.h"
#include "include/BC_OnedCodaBarReader.h"
#include "include/BC_OnedCodaBarWriter.h"
#include "include/BC_CommonBitMatrix.h"
const FX_CHAR CBC_OnedCodaBarWriter::START_END_CHARS[] = {'A', 'B', 'C', 'D', 'T', 'N', '*', 'E', 'a', 'b', 'c', 'd', 't', 'n', 'e'};
const FX_CHAR CBC_OnedCodaBarWriter::CONTENT_CHARS[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '$', '/', ':', '+', '.'};
CBC_OnedCodaBarWriter::CBC_OnedCodaBarWriter()
{
    m_chStart			= 'A';
    m_chEnd				= 'B';
    m_iWideNarrRatio	= 2;
}
CBC_OnedCodaBarWriter::~CBC_OnedCodaBarWriter()
{
}
FX_BOOL CBC_OnedCodaBarWriter::SetStartChar(FX_CHAR start)
{
    for (FX_INT32 i = 0; i < sizeof(START_END_CHARS) / sizeof(FX_CHAR); i++) {
        if (START_END_CHARS[i] == start) {
            m_chStart = start;
            return TRUE;
        }
    }
    return FALSE;
}
FX_BOOL CBC_OnedCodaBarWriter::SetEndChar(FX_CHAR end)
{
    for (FX_INT32 i = 0; i < sizeof(START_END_CHARS) / sizeof(FX_CHAR); i++) {
        if (START_END_CHARS[i] == end) {
            m_chEnd = end;
            return TRUE;
        }
    }
    return FALSE;
}
void CBC_OnedCodaBarWriter::SetDataLength(FX_INT32 length)
{
    m_iDataLenth = length + 2;
}
FX_BOOL CBC_OnedCodaBarWriter::SetTextLocation(BC_TEXT_LOC location)
{
    if ( location < BC_TEXT_LOC_NONE || location > BC_TEXT_LOC_BELOWEMBED) {
        return FALSE;
    }
    m_locTextLoc = location;
    return TRUE;
}
FX_BOOL CBC_OnedCodaBarWriter::SetWideNarrowRatio(FX_INT32 ratio)
{
    if(ratio < 2 || ratio > 3) {
        return FALSE;
    }
    m_iWideNarrRatio = ratio;
    return TRUE;
}
FX_BOOL CBC_OnedCodaBarWriter::FindChar(FX_WCHAR ch, FX_BOOL isContent)
{
    if(isContent) {
        for(FX_INT32 i = 0 ; i < sizeof(CONTENT_CHARS) / sizeof(FX_CHAR) ; i++) {
            if(ch == (FX_WCHAR)CONTENT_CHARS[i]) {
                return TRUE;
            }
        }
        for(FX_INT32 j = 0 ; j < sizeof(START_END_CHARS) / sizeof(FX_CHAR) ; j++) {
            if(ch == (FX_WCHAR)START_END_CHARS[j]) {
                return TRUE;
            }
        }
        return FALSE;
    } else {
        for(FX_INT32 i = 0 ; i < sizeof(CONTENT_CHARS) / sizeof(FX_CHAR) ; i++) {
            if(ch == (FX_WCHAR)CONTENT_CHARS[i]) {
                return TRUE;
            }
        }
        return FALSE;
    }
}
FX_BOOL CBC_OnedCodaBarWriter::CheckContentValidity(FX_WSTR contents)
{
    FX_WCHAR ch;
    FX_INT32 index = 0;
    for (index = 0; index < contents.GetLength(); index++) {
        ch = contents.GetAt(index);
        if (FindChar(ch, FALSE)) {
            continue;
        } else {
            return FALSE;
        }
    }
    return TRUE;
}
CFX_WideString CBC_OnedCodaBarWriter::FilterContents(FX_WSTR contents)
{
    CFX_WideString filtercontents;
    FX_WCHAR ch;
    for (FX_INT32 index = 0; index < contents.GetLength(); index ++) {
        ch = contents.GetAt(index);
        if(ch > 175) {
            index++;
            continue;
        }
        if (FindChar(ch, TRUE)) {
            filtercontents += ch;
        } else {
            continue;
        }
    }
    return filtercontents;
}
FX_BYTE *CBC_OnedCodaBarWriter::Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e)
{
    FX_BYTE *ret = Encode(contents, format, outWidth, outHeight, 0 , e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return ret;
}
FX_BYTE *CBC_OnedCodaBarWriter::Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 hints, FX_INT32 &e)
{
    if(format != BCFORMAT_CODABAR) {
        e = BCExceptionOnlyEncodeCODEBAR;
        return NULL;
    }
    FX_BYTE *ret = CBC_OneDimWriter::Encode(contents, format, outWidth, outHeight, hints, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return ret;
}
FX_BYTE* CBC_OnedCodaBarWriter::Encode(const CFX_ByteString &contents, FX_INT32 &outLength, FX_INT32 &e)
{
    CBC_OnedCodaBarReader CodaBarR;
    CFX_ByteString data = m_chStart + contents + m_chEnd;
    m_iContentLen = data.GetLength();
    FX_BYTE *result = FX_Alloc(FX_BYTE, m_iWideNarrRatio * 7 * data.GetLength());
    FX_CHAR ch;
    FX_INT32 position = 0;
    for (FX_INT32 index = 0; index < data.GetLength(); index++) {
        ch = data.GetAt(index);
        if (((ch >= 'a') && (ch <= 'z'))) {
            ch = ch - 32;
        }
        switch (ch) {
            case 'T':
                ch = 'A';
                break;
            case 'N':
                ch = 'B';
                break;
            case '*':
                ch = 'C';
                break;
            case 'E':
                ch = 'D';
                break;
            default:
                break;
        }
        FX_INT32 code = 0;
        FX_INT32 len =  (FX_INT32)strlen(CodaBarR.ALPHABET_STRING);
        for (FX_INT32 i = 0; i < len; i++) {
            if (ch == CodaBarR.ALPHABET_STRING[i]) {
                code = CodaBarR.CHARACTER_ENCODINGS[i];
                break;
            }
        }
        FX_BYTE color = 1;
        FX_INT32 counter = 0;
        FX_INT32 bit = 0;
        while (bit < 7) {
            result[position] = color;
            position++;
            if (((code >> (6 - bit)) & 1) == 0 || counter == m_iWideNarrRatio - 1) {
                color = !color;
                bit++;
                counter = 0;
            } else {
                counter++;
            }
        }
        if (index < data.GetLength() - 1) {
            result[position] = 0;
            position ++;
        }
    }
    outLength = position;
    return result;
}
CFX_WideString CBC_OnedCodaBarWriter::encodedContents(FX_WSTR contents)
{
    CFX_WideString strStart(m_chStart);
    CFX_WideString strEnd(m_chEnd);
    return strStart + contents + strEnd;
}
void CBC_OnedCodaBarWriter::RenderResult(FX_WSTR contents, FX_BYTE* code, FX_INT32 codeLength, FX_BOOL isDevice, FX_INT32 &e)
{
    CBC_OneDimWriter::RenderResult(encodedContents(contents), code, codeLength, isDevice, e);
}
