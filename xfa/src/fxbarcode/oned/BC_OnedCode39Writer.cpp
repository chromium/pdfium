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

#include "../barcode.h"
#include "../BC_Writer.h"
#include "../BC_Reader.h"
#include "../common/BC_CommonBitMatrix.h"
#include "BC_OneDReader.h"
#include "BC_OneDimWriter.h"
#include "BC_OnedCode39Reader.h"
#include "BC_OnedCode39Writer.h"
CBC_OnedCode39Writer::CBC_OnedCode39Writer()
{
    m_extendedMode = FALSE;
    m_iWideNarrRatio = 3;
}
CBC_OnedCode39Writer::CBC_OnedCode39Writer(FX_BOOL extendedMode)
{
    m_iWideNarrRatio = 3;
    m_extendedMode = extendedMode;
}
CBC_OnedCode39Writer::~CBC_OnedCode39Writer()
{
}
FX_BOOL	CBC_OnedCode39Writer::CheckContentValidity(FX_WSTR contents)
{
    if (m_extendedMode) {
        return CheckExtendedContentValidity(contents);
    }
    for(FX_INT32 i = 0; i < contents.GetLength(); i++) {
        FX_WCHAR ch = contents.GetAt(i);
        if ((ch >= (FX_WCHAR)'0' && ch <= (FX_WCHAR)'9') || (ch >= (FX_WCHAR)'A' && ch <= (FX_WCHAR)'Z')
                || ch == (FX_WCHAR)'-' || ch == (FX_WCHAR)'.' || ch == (FX_WCHAR)' ' || ch == (FX_WCHAR)'*'
                || ch == (FX_WCHAR)'$' || ch == (FX_WCHAR)'/' || ch == (FX_WCHAR)'+' || ch == (FX_WCHAR)'%') {
            continue;
        }
        return FALSE;
    }
    return TRUE;
}
FX_BOOL	CBC_OnedCode39Writer::CheckExtendedContentValidity(FX_WSTR contents)
{
    for(FX_INT32 i = 0; i < contents.GetLength(); i++) {
        FX_WCHAR ch = contents.GetAt(i);
        if (ch > 127) {
            return FALSE;
        }
    }
    return TRUE;
}
CFX_WideString CBC_OnedCode39Writer::FilterContents(FX_WSTR contents)
{
    if (m_extendedMode) {
        return FilterExtendedContents(contents);
    }
    CFX_WideString filtercontents;
    for(FX_INT32 i = 0; i < contents.GetLength(); i++) {
        FX_WCHAR ch = contents.GetAt(i);
        if ( ch == (FX_WCHAR)'*' && (i == 0 || i == contents.GetLength() - 1) ) {
            continue;
        }
        if(ch > 175) {
            i++;
            continue;
        } else {
            ch = Upper(ch);
        }
        if ((ch >= (FX_WCHAR)'0' && ch <= (FX_WCHAR)'9') || (ch >= (FX_WCHAR)'A' && ch <= (FX_WCHAR)'Z')
                || ch == (FX_WCHAR)'-' || ch == (FX_WCHAR)'.' || ch == (FX_WCHAR)' ' || ch == (FX_WCHAR)'*'
                || ch == (FX_WCHAR)'$' || ch == (FX_WCHAR)'/' || ch == (FX_WCHAR)'+' || ch == (FX_WCHAR)'%') {
            filtercontents += ch;
        }
    }
    return filtercontents;
}
CFX_WideString CBC_OnedCode39Writer::FilterExtendedContents(FX_WSTR contents)
{
    CFX_WideString filtercontents;
    for(FX_INT32 i = 0; i < contents.GetLength(); i++) {
        FX_WCHAR ch = contents.GetAt(i);
        if ( ch == (FX_WCHAR)'*' && (i == 0 || i == contents.GetLength() - 1) ) {
            continue;
        }
        if(ch > 175) {
            i++;
            continue;
        }
        if (ch > 127 && ch < 176) {
            continue;
        }
        if (ch == 0) {
            filtercontents += '%';
            filtercontents += 'U';
        } else if(ch >= 1 && ch <= 26) {
            filtercontents += '$';
            filtercontents += (ch + 64);
        } else if (ch >= 27 && ch <= 31) {
            filtercontents += '%';
            filtercontents += (ch + 38);
        } else if (ch >= 33 && ch <= 47 && ch != 45 && ch != 46) {
            filtercontents += '/';
            filtercontents += (ch + 32);
        } else if (ch == 58) {
            filtercontents += '/';
            filtercontents += 'Z';
        } else if (ch >= 59 && ch <= 63) {
            filtercontents += '%';
            filtercontents += ch + 11;
        } else if (ch == 64) {
            filtercontents += '%';
            filtercontents += 'V';
        } else if (ch >= 91 && ch <= 95) {
            filtercontents += '%';
            filtercontents += ch - 16;
        } else if (ch == 96) {
            filtercontents += '%';
            filtercontents += 'W';
        } else if (ch >= 97 && ch <= 122) {
            filtercontents += '+';
            filtercontents += ch - 32;
        } else if (ch >= 123 && ch <= 126) {
            filtercontents += '%';
            filtercontents += ch - 43;
        } else if (ch == 127) {
            filtercontents += '%';
            filtercontents += 'T';
        } else {
            filtercontents += ch;
        }
    }
    return filtercontents;
}
CFX_WideString CBC_OnedCode39Writer::RenderTextContents(FX_WSTR contents)
{
    if (m_extendedMode) {
        return RenderExtendedTextContents(contents);
    }
    CFX_WideString renderContents;
    for(FX_INT32 i = 0; i < contents.GetLength(); i++) {
        FX_WCHAR ch = contents.GetAt(i);
        if ( ch == (FX_WCHAR)'*' && (i == 0 || i == contents.GetLength() - 1) ) {
            continue;
        }
        if(ch > 175) {
            i++;
            continue;
        }
        if ((ch >= (FX_WCHAR)'0' && ch <= (FX_WCHAR)'9') || (ch >= (FX_WCHAR)'A' && ch <= (FX_WCHAR)'Z')
                || (ch >= (FX_WCHAR)'a' && ch <= (FX_WCHAR)'z') || ch == (FX_WCHAR)'-' || ch == (FX_WCHAR)'.'
                || ch == (FX_WCHAR)' ' || ch == (FX_WCHAR)'*' || ch == (FX_WCHAR)'$' || ch == (FX_WCHAR)'/'
                || ch == (FX_WCHAR)'+' || ch == (FX_WCHAR)'%') {
            renderContents += ch;
        }
    }
    return renderContents;
}
CFX_WideString CBC_OnedCode39Writer::RenderExtendedTextContents(FX_WSTR contents)
{
    CFX_WideString renderContents;
    for(FX_INT32 i = 0; i < contents.GetLength(); i++) {
        FX_WCHAR ch = contents.GetAt(i);
        if ( ch == (FX_WCHAR)'*' && (i == 0 || i == contents.GetLength() - 1) ) {
            continue;
        }
        if(ch > 175) {
            i++;
            continue;
        }
        if (ch > 127 && ch < 176) {
            continue;
        }
        renderContents += ch;
    }
    return renderContents;
}
FX_BOOL CBC_OnedCode39Writer::SetTextLocation(BC_TEXT_LOC location)
{
    if ( location < BC_TEXT_LOC_NONE || location > BC_TEXT_LOC_BELOWEMBED) {
        return FALSE;
    }
    m_locTextLoc = location;
    return TRUE;
}
FX_BOOL CBC_OnedCode39Writer::SetWideNarrowRatio(FX_INT32 ratio)
{
    if ( ratio < 2 || ratio > 3) {
        return FALSE;
    }
    m_iWideNarrRatio = ratio;
    return TRUE;
}
FX_BYTE *CBC_OnedCode39Writer::Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e)
{
    FX_BYTE *ret = Encode(contents, format, outWidth, outHeight, 0 , e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return ret;
}
FX_BYTE *CBC_OnedCode39Writer::Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 hints, FX_INT32 &e)
{
    if(format != BCFORMAT_CODE_39) {
        e = BCExceptionOnlyEncodeCODE_39;
        return NULL;
    }
    FX_BYTE *ret = CBC_OneDimWriter::Encode(contents, format, outWidth, outHeight, hints, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return ret;
}
void CBC_OnedCode39Writer::ToIntArray(FX_INT32 a, FX_INT32 *toReturn)
{
    for(FX_INT32 i = 0; i < 9; i++) {
        toReturn[i] = (a & (1 << i) ) == 0 ? 1 : m_iWideNarrRatio;
    }
}
FX_CHAR CBC_OnedCode39Writer::CalcCheckSum(const CFX_ByteString &contents, FX_INT32 &e)
{
    FX_INT32 length = contents.GetLength();
    if (length > 80) {
        e = BCExceptionContentsLengthShouldBetween1and80;
        return '*';
    }
    FX_INT32 checksum = 0;
    FX_INT32 len = (FX_INT32)strlen(CBC_OnedCode39Reader::ALPHABET_STRING);
    for(FX_INT32 i = 0; i < contents.GetLength(); i++) {
        FX_INT32 j = 0;
        for (; j < len; j++) {
            if (CBC_OnedCode39Reader::ALPHABET_STRING[j] == contents[i]) {
                if(contents[i] != '*') {
                    checksum += j;
                    break;
                } else {
                    break;
                }
            }
        }
        if (j >= len) {
            e = BCExceptionUnSupportedString;
            return '*';
        }
    }
    checksum = checksum % 43;
    return CBC_OnedCode39Reader::CHECKSUM_STRING[checksum];
}
FX_BYTE *CBC_OnedCode39Writer::Encode(const CFX_ByteString &contents, FX_INT32 &outlength , FX_INT32 &e)
{
    FX_CHAR checksum = CalcCheckSum(contents, e);
    if (checksum == '*') {
        return NULL;
    }
    FX_INT32 widths[9] = {0};
    FX_INT32 wideStrideNum = 3;
    FX_INT32 narrStrideNum = 9 - wideStrideNum;
    CFX_ByteString encodedContents = contents;
    if ( m_bCalcChecksum ) {
        encodedContents += checksum;
    }
    m_iContentLen = encodedContents.GetLength();
    FX_INT32 codeWidth = (wideStrideNum * m_iWideNarrRatio + narrStrideNum) * 2 + 1 + m_iContentLen;
    FX_INT32 len = (FX_INT32)strlen(CBC_OnedCode39Reader::ALPHABET_STRING);
    for (FX_INT32 j = 0; j < m_iContentLen; j++) {
        for (FX_INT32 i = 0; i < len; i++) {
            if (CBC_OnedCode39Reader::ALPHABET_STRING[i] == encodedContents[j]) {
                ToIntArray(CBC_OnedCode39Reader::CHARACTER_ENCODINGS[i], widths);
                for(FX_INT32 k = 0; k < 9; k++) {
                    codeWidth += widths[k];
                }
            }
        }
    }
    outlength = codeWidth;
    FX_BYTE *result = FX_Alloc(FX_BYTE, codeWidth);
    ToIntArray(CBC_OnedCode39Reader::CHARACTER_ENCODINGS[39], widths);
    FX_INT32 pos = AppendPattern(result, 0, widths, 9, 1 , e);
    if (e != BCExceptionNO) {
        FX_Free (result);
        return NULL;
    }
    FX_INT32 narrowWhite[] = {1};
    pos += AppendPattern(result, pos, narrowWhite, 1, 0, e);
    if (e != BCExceptionNO) {
        FX_Free (result);
        return NULL;
    }
    for(FX_INT32 l = m_iContentLen - 1; l >= 0; l--) {
        for (FX_INT32 i = 0; i < len; i++) {
            if (CBC_OnedCode39Reader::ALPHABET_STRING[i] == encodedContents[l]) {
                ToIntArray(CBC_OnedCode39Reader::CHARACTER_ENCODINGS[i], widths);
                pos += AppendPattern(result, pos, widths, 9, 1, e);
                if (e != BCExceptionNO) {
                    FX_Free (result);
                    return NULL;
                }
            }
        }
        pos += AppendPattern(result, pos, narrowWhite, 1, 0, e);
        if (e != BCExceptionNO) {
            FX_Free (result);
            return NULL;
        }
    }
    ToIntArray(CBC_OnedCode39Reader::CHARACTER_ENCODINGS[39], widths);
    pos += AppendPattern(result, pos, widths, 9, 1, e);
    if (e != BCExceptionNO) {
        FX_Free (result);
        return NULL;
    }
    for (FX_INT32 i = 0; i < codeWidth / 2; i++) {
        result[i] ^= result[codeWidth - 1 - i];
        result[codeWidth - 1 - i] ^= result[i];
        result[i] ^= result[codeWidth - 1 - i];
    }
    return result;
}
CFX_WideString CBC_OnedCode39Writer::encodedContents(FX_WSTR contents, FX_INT32 &e)
{
    CFX_WideString encodedContents = contents;
    if (m_bCalcChecksum && m_bPrintChecksum) {
        CFX_WideString checksumContent = FilterContents(contents);
        CFX_ByteString str = checksumContent.UTF8Encode();
        FX_CHAR checksum;
        checksum = CalcCheckSum(str, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, FX_WSTRC(L""));
        str += checksum;
        encodedContents += checksum;
    }
    return encodedContents;
}
void CBC_OnedCode39Writer::RenderResult(FX_WSTR contents, FX_BYTE* code, FX_INT32 codeLength, FX_BOOL isDevice, FX_INT32 &e)
{
    CFX_WideString encodedCon = encodedContents(contents, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    CBC_OneDimWriter::RenderResult(encodedCon, code, codeLength, isDevice, e);
}
