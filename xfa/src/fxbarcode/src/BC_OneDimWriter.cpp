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
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_OneDimWriter.h"
CBC_OneDimWriter::CBC_OneDimWriter()
{
    m_locTextLoc			= BC_TEXT_LOC_BELOWEMBED;
    m_bPrintChecksum		= TRUE;
    m_iDataLenth			= 0;
    m_bCalcChecksum			= FALSE;
    m_pFont					= NULL;
    m_fFontSize				= 10;                                                                                                                                                                             ;
    m_iFontStyle			= 0;
    m_fontColor				= 0xff000000;
    m_iContentLen			= 0;
    m_bLeftPadding			= FALSE;
    m_bRightPadding			= FALSE;
    m_output                = NULL;
}
CBC_OneDimWriter::~CBC_OneDimWriter()
{
    if (m_output != NULL) {
        delete m_output;
        m_output = NULL;
    }
}
void CBC_OneDimWriter::SetPrintChecksum(FX_BOOL checksum)
{
    m_bPrintChecksum = checksum;
}
void CBC_OneDimWriter::SetDataLength(FX_INT32 length)
{
    m_iDataLenth = length;
}
void CBC_OneDimWriter::SetCalcChecksum(FX_INT32 state)
{
    m_bCalcChecksum = state;
}
FX_BOOL	CBC_OneDimWriter::SetFont(CFX_Font * cFont)
{
    if (cFont == NULL) {
        return FALSE;
    }
    m_pFont = cFont;
    return TRUE;
}
void CBC_OneDimWriter::SetFontSize(FX_FLOAT size)
{
    m_fFontSize = size;
}
void CBC_OneDimWriter::SetFontStyle(FX_INT32 style)
{
    m_iFontStyle = style;
}
void CBC_OneDimWriter::SetFontColor(FX_ARGB color)
{
    m_fontColor = color;
}
FX_WCHAR CBC_OneDimWriter::Upper(FX_WCHAR ch)
{
    if(ch >= 'a' && ch <= 'z') {
        ch = ch - ('a' - 'A');
    }
    return ch;
}
FX_BYTE* CBC_OneDimWriter::Encode(const CFX_ByteString &contents, BCFORMAT format,
                                  FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 hints, FX_INT32 &e)
{
    FX_BYTE *ret = NULL;
    outHeight = 1;
    if (m_Width >= 20) {
        ret = Encode(contents, outWidth, e);
    } else {
        ret = Encode(contents, outWidth, e);
    }
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return ret;
}
FX_BYTE *CBC_OneDimWriter::Encode(const CFX_ByteString &contents, BCFORMAT format,
                                  FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e)
{
    FX_BYTE *ret = Encode(contents, format, outWidth, outHeight, 0, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return ret;
}
FX_INT32 CBC_OneDimWriter::AppendPattern(FX_BYTE* target, FX_INT32 pos, const FX_INT32* pattern , FX_INT32 patternLength, FX_INT32 startColor, FX_INT32 &e)
{
    if (startColor != 0 && startColor != 1) {
        e = BCExceptionValueMustBeEither0or1;
        return 0;
    }
    FX_BYTE color = (FX_BYTE) startColor;
    FX_INT32 numAdded = 0;
    for (FX_INT32 i = 0; i < patternLength; i++) {
        for (FX_INT32 j = 0; j < pattern[i]; j++) {
            target[pos] = color;
            pos += 1;
            numAdded += 1;
        }
        color ^= 1;
    }
    return numAdded;
}
void CBC_OneDimWriter::CalcTextInfo(const CFX_ByteString &text, FXTEXT_CHARPOS *charPos, CFX_Font *cFont, FX_FLOAT geWidth, FX_INT32 fontSize, FX_FLOAT &charsLen)
{
#ifdef FXFM_ENCODING_NONE
    IFX_FontEncodingEx* encoding = FX_CreateFontEncodingEx(cFont);
#else
    IFX_FontEncoding * encoding = FXGE_CreateUnicodeEncoding(cFont);
#endif
    FX_INT32 length = text.GetLength();
    FX_DWORD *pCharCode = FX_Alloc(FX_DWORD, text.GetLength());
    FX_FLOAT charWidth = 0;
    for (FX_INT32 j = 0; j < text.GetLength(); j++) {
        pCharCode[j] = encoding->CharCodeFromUnicode(text[j]);
        FX_INT32 glyp_code = encoding->GlyphFromCharCode(pCharCode[j]);
        FX_INT32 glyp_value = cFont->GetGlyphWidth(glyp_code);
        FX_FLOAT temp = (FX_FLOAT)((glyp_value) * fontSize / 1000.0);
        charWidth += temp;
    }
    charsLen = charWidth;
    FX_FLOAT leftPositon = (FX_FLOAT)(geWidth - charsLen) / 2.0f;
    if (leftPositon < 0 && geWidth == 0) {
        leftPositon = 0;
    }
    FX_FLOAT penX = 0.0;
    FX_FLOAT penY = (FX_FLOAT)FXSYS_abs(cFont->GetDescent()) * (FX_FLOAT)fontSize / 1000.0f;
    FX_FLOAT left = leftPositon;
    FX_FLOAT top  = 0.0;
    charPos[0].m_OriginX = penX + left;
    charPos[0].m_OriginY = penY + top;
    charPos[0].m_GlyphIndex = encoding->GlyphFromCharCode(pCharCode[0]);
    charPos[0].m_FontCharWidth = cFont->GetGlyphWidth(charPos[0].m_GlyphIndex);
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
    charPos[0].m_ExtGID = charPos[0].m_GlyphIndex;
#endif
    penX += (FX_FLOAT)(charPos[0].m_FontCharWidth) * (FX_FLOAT)fontSize / 1000.0f;
    for (FX_INT32 i = 1; i < length; i++) {
        charPos[i].m_OriginX = penX + left;
        charPos[i].m_OriginY = penY + top;
        charPos[i].m_GlyphIndex = encoding->GlyphFromCharCode(pCharCode[i]);
        charPos[i].m_FontCharWidth = cFont->GetGlyphWidth(charPos[i].m_GlyphIndex);
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
        charPos[i].m_ExtGID = charPos[i].m_GlyphIndex;
#endif
        penX += (FX_FLOAT)(charPos[i].m_FontCharWidth) * (FX_FLOAT)fontSize / 1000.0f;
    }
    FX_Free (pCharCode);
    delete encoding;
    encoding = NULL;
}
void CBC_OneDimWriter::ShowDeviceChars(CFX_RenderDevice *device, const CFX_Matrix* matrix, const CFX_ByteString str, FX_FLOAT geWidth, FXTEXT_CHARPOS* pCharPos, FX_FLOAT locX, FX_FLOAT locY,  FX_INT32 barWidth)
{
    FX_INT32 iFontSize = (FX_INT32)fabs(m_fFontSize);
    FX_INT32 iTextHeight = iFontSize + 1;
    CFX_FloatRect rect((FX_FLOAT)locX, (FX_FLOAT)locY, (FX_FLOAT)(locX + geWidth), (FX_FLOAT)(locY + iTextHeight));
    if (geWidth != m_Width) {
        rect.right -= 1;
    }
    matrix->TransformRect(rect);
    FX_RECT re = rect.GetOutterRect();
    device->FillRect(&re, m_backgroundColor);
    CFX_Matrix affine_matrix(1.0, 0.0, 0.0, -1.0, (FX_FLOAT)locX, (FX_FLOAT)(locY + iFontSize));
    if (matrix != NULL) {
        affine_matrix.Concat(*matrix);
    }
    FX_BOOL ret = device->DrawNormalText(str.GetLength(),
                                         pCharPos,
                                         m_pFont,
                                         CFX_GEModule::Get()->GetFontCache(),
                                         (FX_FLOAT)iFontSize,
                                         (CFX_AffineMatrix *) &affine_matrix,
                                         m_fontColor, FXTEXT_CLEARTYPE);
}
void CBC_OneDimWriter::ShowBitmapChars(CFX_DIBitmap *pOutBitmap, const CFX_ByteString str, FX_FLOAT geWidth, FXTEXT_CHARPOS* pCharPos, FX_FLOAT locX, FX_FLOAT locY, FX_INT32 barWidth)
{
    FX_INT32 iFontSize = (FX_INT32)fabs(m_fFontSize);
    FX_INT32 iTextHeight = iFontSize + 1;
    CFX_FxgeDevice ge;
    ge.Create((int)geWidth, iTextHeight , m_colorSpace);
    FX_RECT geRect(0, 0, (int)geWidth, iTextHeight);
    ge.FillRect(&geRect, m_backgroundColor);
    CFX_AffineMatrix affine_matrix(1.0, 0.0, 0.0, -1.0, 0.0, (FX_FLOAT)iFontSize);
    FX_BOOL ret = ge.DrawNormalText(str.GetLength(),
                                    pCharPos,
                                    m_pFont,
                                    CFX_GEModule::Get()->GetFontCache(),
                                    (FX_FLOAT)iFontSize,
                                    (CFX_AffineMatrix *) &affine_matrix,
                                    m_fontColor, FXTEXT_CLEARTYPE);
    CFX_FxgeDevice geBitmap;
    geBitmap.Attach(pOutBitmap);
    geBitmap.SetDIBits(ge.GetBitmap(), (int)locX, (int)locY);
}
void CBC_OneDimWriter::ShowChars(FX_WSTR contents, CFX_DIBitmap *pOutBitmap, CFX_RenderDevice *device, const CFX_Matrix* matrix, FX_INT32 barWidth, FX_INT32 multiple, FX_INT32 &e)
{
    if (device == NULL && pOutBitmap == NULL) {
        e = BCExceptionIllegalArgument;
        return;
    }
    if (m_pFont == NULL) {
        e = BCExceptionNullPointer;
        return;
    }
    CFX_ByteString str = FX_UTF8Encode(contents);
    FX_INT32 iLen = str.GetLength();
    FXTEXT_CHARPOS* pCharPos = FX_Alloc(FXTEXT_CHARPOS, iLen);
    if (!pCharPos) {
        return;
    }
    FXSYS_memset32(pCharPos, 0, sizeof(FXTEXT_CHARPOS) * iLen);
    FX_FLOAT charsLen = 0;
    FX_FLOAT geWidth = 0;
    if ( m_locTextLoc == BC_TEXT_LOC_ABOVEEMBED ||
            m_locTextLoc == BC_TEXT_LOC_BELOWEMBED ) {
        geWidth = 0;
    } else if ( m_locTextLoc == BC_TEXT_LOC_ABOVE ||
                m_locTextLoc == BC_TEXT_LOC_BELOW ) {
        geWidth = (FX_FLOAT)barWidth;
    }
    FX_INT32 iFontSize = (FX_INT32)fabs(m_fFontSize);
    FX_INT32 iTextHeight = iFontSize + 1;
    CalcTextInfo(str, pCharPos, m_pFont, geWidth, iFontSize, charsLen);
    if (charsLen < 1) {
        return;
    }
    FX_INT32 locX = 0;
    FX_INT32 locY = 0;
    switch (m_locTextLoc) {
        case BC_TEXT_LOC_ABOVEEMBED:
            locX = (FX_INT32)(barWidth - charsLen) / 2;
            locY = 0;
            geWidth = charsLen;
            break;
        case BC_TEXT_LOC_ABOVE:
            locX = 0;
            locY = 0;
            geWidth = (FX_FLOAT)barWidth;
            break;
        case BC_TEXT_LOC_BELOWEMBED:
            locX = (FX_INT32)(barWidth - charsLen) / 2;
            locY = m_Height - iTextHeight;
            geWidth = charsLen;
            break;
        case BC_TEXT_LOC_BELOW:
        default:
            locX = 0;
            locY = m_Height - iTextHeight;
            geWidth = (FX_FLOAT)barWidth;
            break;
    }
    if (device != NULL) {
        ShowDeviceChars(device, matrix, str, geWidth, pCharPos, (FX_FLOAT)locX, (FX_FLOAT)locY, barWidth);
    } else {
        ShowBitmapChars(pOutBitmap, str, geWidth, pCharPos, (FX_FLOAT)locX, (FX_FLOAT)locY, barWidth);
    }
    FX_Free(pCharPos);
}
void CBC_OneDimWriter::RenderBitmapResult(CFX_DIBitmap *&pOutBitmap, FX_WSTR contents, FX_INT32 &e)
{
    if (m_output == NULL) {
        BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
    pOutBitmap = CreateDIBitmap(m_output->GetWidth(), m_output->GetHeight());
    pOutBitmap->Clear(m_backgroundColor);
    if (!pOutBitmap) {
        e = BCExceptionFailToCreateBitmap;
        return;
    }
    for (FX_INT32 x = 0; x < m_output->GetWidth(); x++) {
        for (FX_INT32 y = 0; y < m_output->GetHeight(); y++) {
            if (m_output->Get(x, y)) {
                pOutBitmap->SetPixel(x, y, m_barColor);
            }
        }
    }
    FX_INT32 i = 0;
    for (; i < contents.GetLength(); i++)
        if (contents.GetAt(i) != ' ') {
            break;
        }
    if (m_locTextLoc != BC_TEXT_LOC_NONE && i < contents.GetLength()) {
        ShowChars(contents, pOutBitmap, NULL, NULL, m_barWidth, m_multiple, e);
        BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
    CFX_DIBitmap * pStretchBitmap = pOutBitmap->StretchTo(m_Width, m_Height);
    if (pOutBitmap) {
        delete pOutBitmap;
    }
    pOutBitmap = pStretchBitmap;
}
void CBC_OneDimWriter::RenderDeviceResult(CFX_RenderDevice* device, const CFX_Matrix* matrix, FX_WSTR contents, FX_INT32 &e)
{
    if (m_output == NULL) {
        BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
    CFX_GraphStateData stateData;
    CFX_PathData path;
    path.AppendRect(0, 0, (FX_FLOAT)m_Width, (FX_FLOAT)m_Height);
    device->DrawPath(&path, matrix, &stateData, m_backgroundColor, m_backgroundColor, FXFILL_ALTERNATE);
    CFX_Matrix matri(m_outputHScale, 0.0, 0.0, (FX_FLOAT)m_Height, 0.0, 0.0);
    matri.Concat(*matrix);
    for (FX_INT32 x = 0; x < m_output->GetWidth(); x++) {
        for (FX_INT32 y = 0; y < m_output->GetHeight(); y++) {
            CFX_PathData rect;
            rect.AppendRect((FX_FLOAT)x, (FX_FLOAT)y, (FX_FLOAT)(x + 1), (FX_FLOAT)(y + 1));
            CFX_GraphStateData stateData;
            if (m_output->Get(x, y)) {
                device->DrawPath(&rect, &matri, &stateData, m_barColor, 0, FXFILL_WINDING);
            }
        }
    }
    FX_INT32 i = 0;
    for (; i < contents.GetLength(); i++)
        if (contents.GetAt(i) != ' ') {
            break;
        }
    if (m_locTextLoc != BC_TEXT_LOC_NONE && i < contents.GetLength()) {
        ShowChars(contents, NULL, device, matrix, m_barWidth, m_multiple, e);
        BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
}
void CBC_OneDimWriter::RenderResult(FX_WSTR contents, FX_BYTE* code, FX_INT32 codeLength,  FX_BOOL isDevice, FX_INT32 &e)
{
    if (codeLength < 1) {
        BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
    if (m_ModuleHeight < 20.0) {
        m_ModuleHeight = 20;
    }
    FX_INT32 codeOldLength = codeLength;
    FX_INT32 leftPadding = 0;
    FX_INT32 rightPadding = 0;
    if (m_bLeftPadding) {
        leftPadding = 7;
    }
    if (m_bRightPadding) {
        rightPadding = 7;
    }
    codeLength += leftPadding;
    codeLength += rightPadding;
    m_outputHScale = 1.0;
    if (m_Width > 0) {
        m_outputHScale = (FX_FLOAT)m_Width / (FX_FLOAT)codeLength;
    }
    if (!isDevice) {
        m_outputHScale = FX_MAX(m_outputHScale, m_ModuleWidth);
    }
    FX_FLOAT dataLengthScale = 1.0;
    if (m_iDataLenth > 0 && contents.GetLength() != 0) {
        dataLengthScale = FX_FLOAT(contents.GetLength()) / FX_FLOAT(m_iDataLenth);
    }
    if (m_iDataLenth > 0 && contents.GetLength() == 0) {
        dataLengthScale = FX_FLOAT(1) / FX_FLOAT(m_iDataLenth);
    }
    m_multiple = 1;
    if (!isDevice) {
        m_multiple = (FX_INT32)ceil(m_outputHScale * dataLengthScale);
    }
    FX_INT32 outputHeight = 1;
    if (!isDevice) {
        if (m_Height == 0) {
            outputHeight = FX_MAX(20, m_ModuleHeight);
        } else {
            outputHeight = m_Height;
        }
    }
    FX_INT32 outputWidth = codeLength;
    if (!isDevice) {
        outputWidth = (FX_INT32)(codeLength * m_multiple / dataLengthScale);
    }
    m_barWidth = m_Width;
    if (!isDevice) {
        m_barWidth = codeLength * m_multiple;
    }
    m_output = FX_NEW CBC_CommonBitMatrix;
    m_output->Init(outputWidth, outputHeight);
    FX_INT32 outputX = leftPadding * m_multiple;
    for (FX_INT32 inputX = 0; inputX < codeOldLength; inputX++) {
        if (code[inputX] == 1) {
            if (outputX >= outputWidth ) {
                break;
            }
            if (outputX + m_multiple > outputWidth && outputWidth - outputX > 0) {
                m_output->SetRegion(outputX, 0, outputWidth - outputX , outputHeight, e);
                break;
            }
            m_output->SetRegion(outputX, 0, m_multiple, outputHeight, e);
            BC_EXCEPTION_CHECK_ReturnVoid(e);
        }
        outputX += m_multiple;
    }
}
