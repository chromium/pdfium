// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2009 ZXing authors
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
#include "include/BC_OneDimReader.h"
#include "include/BC_OneDimWriter.h"
#include "include/BC_OnedEAN13Reader.h"
#include "include/BC_OnedEAN13Writer.h"
CBC_OnedEAN13Writer::CBC_OnedEAN13Writer()
{
    m_bLeftPadding = TRUE;
    m_codeWidth = 3 +
                  (7 * 6) +
                  5 +
                  (7 * 6) +
                  3;
}
CBC_OnedEAN13Writer::~CBC_OnedEAN13Writer()
{
}
FX_BOOL	CBC_OnedEAN13Writer::CheckContentValidity(FX_WSTR contents)
{
    for (FX_INT32 i = 0; i < contents.GetLength(); i++) {
        if (contents.GetAt(i) >= '0' && contents.GetAt(i) <= '9') {
            continue;
        } else {
            return FALSE;
        }
    }
    return TRUE;
}
CFX_WideString	CBC_OnedEAN13Writer::FilterContents(FX_WSTR contents)
{
    CFX_WideString filtercontents;
    FX_WCHAR ch;
    for (FX_INT32 i = 0; i < contents.GetLength(); i++) {
        ch = contents.GetAt(i);
        if(ch > 175) {
            i++;
            continue;
        }
        if (ch >= '0' && ch <= '9') {
            filtercontents += ch;
        }
    }
    return filtercontents;
}
FX_INT32 CBC_OnedEAN13Writer::CalcChecksum(const CFX_ByteString &contents)
{
    FX_INT32 odd = 0;
    FX_INT32 even = 0;
    FX_INT32 j = 1;
    for(FX_INT32 i = contents.GetLength() - 1; i >= 0; i--) {
        if(j % 2) {
            odd += FXSYS_atoi(contents.Mid(i, 1));
        } else {
            even += FXSYS_atoi(contents.Mid(i, 1));
        }
        j++;
    }
    FX_INT32 checksum = (odd * 3 + even) % 10;
    checksum = (10 - checksum) % 10;
    return (checksum);
}
FX_BYTE *CBC_OnedEAN13Writer::Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e)
{
    FX_BYTE *ret = Encode(contents, format, outWidth, outHeight, 0, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return ret;
}
FX_BYTE *CBC_OnedEAN13Writer::Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 hints, FX_INT32 &e)
{
    if(format != BCFORMAT_EAN_13) {
        e = BCExceptionOnlyEncodeEAN_13;
    }
    FX_BYTE *ret = CBC_OneDimWriter::Encode(contents, format, outWidth, outHeight, hints, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return ret;
}
FX_BYTE *CBC_OnedEAN13Writer::Encode(const CFX_ByteString &contents, FX_INT32 &outLength, FX_INT32 &e)
{
    if (contents.GetLength() != 13) {
        e = BCExceptionDigitLengthShould13;
        return NULL;
    }
    m_iDataLenth = 13;
    FX_INT32 firstDigit = FXSYS_atoi(contents.Mid(0, 1));
    FX_INT32 parities = CBC_OnedEAN13Reader::FIRST_DIGIT_ENCODINGS[firstDigit];
    outLength = m_codeWidth;
    FX_BYTE *result = FX_Alloc(FX_BYTE, m_codeWidth);
    FX_INT32 pos = 0;
    pos += AppendPattern(result, pos, CBC_OneDimReader::START_END_PATTERN, 3, 1, e);
    if (e != BCExceptionNO) {
        FX_Free (result);
        return NULL;
    }
    FX_INT32 i = 0;
    for ( i = 1; i <= 6; i++) {
        FX_INT32 digit = FXSYS_atoi(contents.Mid(i, 1));
        if ((parities >> (6 - i) & 1) == 1) {
            digit += 10;
        }
        pos += AppendPattern(result, pos, CBC_OneDimReader::L_AND_G_PATTERNS[digit], 4, 0, e);
        if (e != BCExceptionNO) {
            FX_Free (result);
            return NULL;
        }
    }
    pos += AppendPattern(result, pos, CBC_OneDimReader::MIDDLE_PATTERN, 5, 0, e);
    if (e != BCExceptionNO) {
        FX_Free (result);
        return NULL;
    }
    for (i = 7; i <= 12; i++) {
        FX_INT32 digit = FXSYS_atoi(contents.Mid(i, 1));
        pos += AppendPattern(result, pos, CBC_OneDimReader::L_PATTERNS[digit], 4, 1, e);
        if (e != BCExceptionNO) {
            FX_Free (result);
            return NULL;
        }
    }
    pos += AppendPattern(result, pos, CBC_OneDimReader::START_END_PATTERN, 3, 1, e);
    if (e != BCExceptionNO) {
        FX_Free (result);
        return NULL;
    }
    return result;
}
void CBC_OnedEAN13Writer::ShowChars(FX_WSTR contents, CFX_DIBitmap *pOutBitmap, CFX_RenderDevice* device, const CFX_Matrix* matrix, FX_INT32 barWidth, FX_INT32 multiple, FX_INT32 &e)
{
    if (device == NULL && pOutBitmap == NULL) {
        e = BCExceptionIllegalArgument;
        return;
    }
    FX_INT32 leftPadding = 7 * multiple;
    FX_INT32 leftPosition = 3 * multiple + leftPadding;
    CFX_ByteString str = FX_UTF8Encode(contents);
    FX_INT32 iLen = str.GetLength();
    FXTEXT_CHARPOS* pCharPos = FX_Alloc(FXTEXT_CHARPOS, iLen);
    if (!pCharPos) {
        return;
    }
    FXSYS_memset32(pCharPos, 0, sizeof(FXTEXT_CHARPOS) * iLen);
    CFX_FxgeDevice geBitmap;
    if (pOutBitmap != NULL) {
        geBitmap.Attach(pOutBitmap);
    }
    FX_INT32 iFontSize = (FX_INT32)fabs(m_fFontSize);
    FX_INT32 iTextHeight = iFontSize + 1;
    CFX_ByteString tempStr = str.Mid(1, 6);
    FX_INT32 strWidth = multiple * 42;
    if (pOutBitmap == NULL) {
        CFX_Matrix matr(m_outputHScale, 0.0, 0.0, 1.0, 0.0, 0.0);
        CFX_FloatRect rect((FX_FLOAT)leftPosition, (FX_FLOAT)(m_Height - iTextHeight), (FX_FLOAT)(leftPosition + strWidth - 0.5), (FX_FLOAT)m_Height);
        matr.Concat(*matrix);
        matr.TransformRect(rect);
        FX_RECT re = rect.GetOutterRect();
        device->FillRect(&re, m_backgroundColor);
        CFX_FloatRect rect1((FX_FLOAT)(leftPosition + 47 * multiple), (FX_FLOAT)(m_Height - iTextHeight), (FX_FLOAT)(leftPosition + 47 * multiple + strWidth - 0.5), (FX_FLOAT)m_Height);
        CFX_Matrix matr1(m_outputHScale, 0.0, 0.0, 1.0, 0.0, 0.0);
        matr1.Concat(*matrix);
        matr1.TransformRect(rect1);
        re = rect1.GetOutterRect();
        device->FillRect(&re, m_backgroundColor);
        FX_INT32 strWidth1 = multiple * 7;
        CFX_Matrix matr2(m_outputHScale, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
        CFX_FloatRect rect2(0.0f, (FX_FLOAT)(m_Height - iTextHeight), (FX_FLOAT)strWidth1 - 0.5f, (FX_FLOAT)m_Height);
        matr2.Concat(*matrix);
        matr2.TransformRect(rect2);
        re = rect2.GetOutterRect();
        device->FillRect(&re, m_backgroundColor);
    }
    FX_FLOAT blank = 0.0;
    FX_FLOAT charsWidth = 0;
    iLen = tempStr.GetLength();
    if (pOutBitmap == NULL) {
        strWidth = (FX_INT32)(strWidth * m_outputHScale);
    }
    CalcTextInfo(tempStr, pCharPos + 1, m_pFont, (FX_FLOAT)strWidth, iFontSize, blank);
    CFX_AffineMatrix affine_matrix(1.0, 0.0, 0.0, -1.0, 0.0, (FX_FLOAT)iFontSize);
    CFX_FxgeDevice ge;
    if (pOutBitmap != NULL) {
        ge.Create(strWidth, iTextHeight, FXDIB_Argb);
        FX_RECT rect(0, 0, strWidth, iTextHeight);
        ge.FillRect(&rect, m_backgroundColor);
        ge.DrawNormalText(iLen,
                          pCharPos + 1,
                          m_pFont,
                          CFX_GEModule::Get()->GetFontCache(),
                          (FX_FLOAT)iFontSize ,
                          (CFX_AffineMatrix *) &affine_matrix,
                          m_fontColor, FXTEXT_CLEARTYPE);
        geBitmap.SetDIBits(ge.GetBitmap(), leftPosition, m_Height - iTextHeight);
    } else {
        CFX_AffineMatrix affine_matrix1(1.0, 0.0, 0.0, -1.0, (FX_FLOAT)leftPosition * m_outputHScale, (FX_FLOAT)(m_Height - iTextHeight) + iFontSize);
        if (matrix != NULL) {
            affine_matrix1.Concat(*matrix);
        }
        device->DrawNormalText(iLen,
                               pCharPos + 1,
                               m_pFont,
                               CFX_GEModule::Get()->GetFontCache(),
                               (FX_FLOAT)iFontSize ,
                               (CFX_AffineMatrix *) &affine_matrix1,
                               m_fontColor, FXTEXT_CLEARTYPE);
    }
    tempStr = str.Mid(7, 6);
    iLen = tempStr.GetLength();
    charsWidth = 0.0f;
    CalcTextInfo(tempStr, pCharPos + 7, m_pFont, (FX_FLOAT)strWidth, iFontSize, blank);
    if(pOutBitmap != NULL) {
        FX_RECT rect1(0, 0, strWidth, iTextHeight);
        ge.FillRect(&rect1, m_backgroundColor);
        ge.DrawNormalText(iLen,
                          pCharPos + 7,
                          m_pFont,
                          CFX_GEModule::Get()->GetFontCache(),
                          (FX_FLOAT)iFontSize ,
                          (CFX_AffineMatrix *) &affine_matrix,
                          m_fontColor, FXTEXT_CLEARTYPE);
        geBitmap.SetDIBits(ge.GetBitmap(), leftPosition + 47 * multiple, m_Height - iTextHeight);
    } else {
        CFX_AffineMatrix affine_matrix1(1.0, 0.0, 0.0, -1.0, (FX_FLOAT)(leftPosition + 47 * multiple) * m_outputHScale, (FX_FLOAT)(m_Height - iTextHeight + iFontSize));
        if (matrix != NULL) {
            affine_matrix1.Concat(*matrix);
        }
        device->DrawNormalText(iLen,
                               pCharPos + 7,
                               m_pFont,
                               CFX_GEModule::Get()->GetFontCache(),
                               (FX_FLOAT)iFontSize ,
                               (CFX_AffineMatrix *) &affine_matrix1,
                               m_fontColor, FXTEXT_CLEARTYPE);
    }
    tempStr = str.Mid(0, 1);
    iLen = tempStr.GetLength();
    strWidth = multiple * 7;
    if (pOutBitmap == NULL) {
        strWidth = (FX_INT32)(strWidth * m_outputHScale);
    }
    CalcTextInfo(tempStr, pCharPos, m_pFont, (FX_FLOAT)strWidth, iFontSize, blank);
    if(pOutBitmap != NULL) {
        delete ge.GetBitmap();
        ge.Create(strWidth, iTextHeight, FXDIB_Argb);
        ge.GetBitmap()->Clear(m_backgroundColor);
        ge.DrawNormalText(iLen,
                          pCharPos,
                          m_pFont,
                          CFX_GEModule::Get()->GetFontCache(),
                          (FX_FLOAT)iFontSize ,
                          (CFX_AffineMatrix *) &affine_matrix,
                          m_fontColor, FXTEXT_CLEARTYPE);
        geBitmap.SetDIBits(ge.GetBitmap(), 0, m_Height - iTextHeight);
    } else {
        CFX_AffineMatrix affine_matrix1(1.0, 0.0, 0.0, -1.0, 0.0, (FX_FLOAT)(m_Height - iTextHeight + iFontSize));
        if (matrix != NULL) {
            affine_matrix1.Concat(*matrix);
        }
        device->DrawNormalText(iLen,
                               pCharPos,
                               m_pFont,
                               CFX_GEModule::Get()->GetFontCache(),
                               (FX_FLOAT)iFontSize ,
                               (CFX_AffineMatrix *) &affine_matrix1,
                               m_fontColor, FXTEXT_CLEARTYPE);
    }
    FX_Free(pCharPos);
}
void CBC_OnedEAN13Writer::RenderResult(FX_WSTR contents, FX_BYTE* code, FX_INT32 codeLength, FX_BOOL isDevice, FX_INT32 &e)
{
    CBC_OneDimWriter::RenderResult(contents, code, codeLength, isDevice, e);
}
