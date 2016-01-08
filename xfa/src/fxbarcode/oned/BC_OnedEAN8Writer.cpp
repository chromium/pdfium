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

#include "xfa/src/fxbarcode/barcode.h"
#include "xfa/src/fxbarcode/BC_Writer.h"
#include "xfa/src/fxbarcode/BC_Reader.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitMatrix.h"
#include "BC_OneDReader.h"
#include "BC_OneDimWriter.h"
#include "BC_OneDimReader.h"
#include "BC_OnedEAN8Writer.h"
CBC_OnedEAN8Writer::CBC_OnedEAN8Writer() {
  m_iDataLenth = 8;
  m_codeWidth = 3 + (7 * 4) + 5 + (7 * 4) + 3;
}
CBC_OnedEAN8Writer::~CBC_OnedEAN8Writer() {}
void CBC_OnedEAN8Writer::SetDataLength(int32_t length) {
  m_iDataLenth = 8;
}
FX_BOOL CBC_OnedEAN8Writer::SetTextLocation(BC_TEXT_LOC location) {
  if (location == BC_TEXT_LOC_BELOWEMBED) {
    m_locTextLoc = location;
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CBC_OnedEAN8Writer::CheckContentValidity(
    const CFX_WideStringC& contents) {
  for (int32_t i = 0; i < contents.GetLength(); i++) {
    if (contents.GetAt(i) >= '0' && contents.GetAt(i) <= '9') {
      continue;
    } else {
      return FALSE;
    }
  }
  return TRUE;
}
CFX_WideString CBC_OnedEAN8Writer::FilterContents(
    const CFX_WideStringC& contents) {
  CFX_WideString filtercontents;
  FX_WCHAR ch;
  for (int32_t i = 0; i < contents.GetLength(); i++) {
    ch = contents.GetAt(i);
    if (ch > 175) {
      i++;
      continue;
    }
    if (ch >= '0' && ch <= '9') {
      filtercontents += ch;
    }
  }
  return filtercontents;
}
int32_t CBC_OnedEAN8Writer::CalcChecksum(const CFX_ByteString& contents) {
  int32_t odd = 0;
  int32_t even = 0;
  int32_t j = 1;
  for (int32_t i = contents.GetLength() - 1; i >= 0; i--) {
    if (j % 2) {
      odd += FXSYS_atoi(contents.Mid(i, 1));
    } else {
      even += FXSYS_atoi(contents.Mid(i, 1));
    }
    j++;
  }
  int32_t checksum = (odd * 3 + even) % 10;
  checksum = (10 - checksum) % 10;
  return (checksum);
}
uint8_t* CBC_OnedEAN8Writer::Encode(const CFX_ByteString& contents,
                                    BCFORMAT format,
                                    int32_t& outWidth,
                                    int32_t& outHeight,
                                    int32_t& e) {
  uint8_t* ret = Encode(contents, format, outWidth, outHeight, 0, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return ret;
}
uint8_t* CBC_OnedEAN8Writer::Encode(const CFX_ByteString& contents,
                                    BCFORMAT format,
                                    int32_t& outWidth,
                                    int32_t& outHeight,
                                    int32_t hints,
                                    int32_t& e) {
  if (format != BCFORMAT_EAN_8) {
    e = BCExceptionOnlyEncodeEAN_8;
    return NULL;
  }
  uint8_t* ret =
      CBC_OneDimWriter::Encode(contents, format, outWidth, outHeight, hints, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return ret;
}
uint8_t* CBC_OnedEAN8Writer::Encode(const CFX_ByteString& contents,
                                    int32_t& outLength,
                                    int32_t& e) {
  if (contents.GetLength() != 8) {
    e = BCExceptionDigitLengthMustBe8;
    return NULL;
  }
  outLength = m_codeWidth;
  uint8_t* result = FX_Alloc(uint8_t, m_codeWidth);
  int32_t pos = 0;
  pos +=
      AppendPattern(result, pos, CBC_OneDimReader::START_END_PATTERN, 3, 1, e);
  if (e != BCExceptionNO) {
    FX_Free(result);
    return NULL;
  }
  int32_t i = 0;
  for (i = 0; i <= 3; i++) {
    int32_t digit = FXSYS_atoi(contents.Mid(i, 1));
    pos += AppendPattern(result, pos, CBC_OneDimReader::L_PATTERNS[digit], 4, 0,
                         e);
    if (e != BCExceptionNO) {
      FX_Free(result);
      return NULL;
    }
  }
  pos += AppendPattern(result, pos, CBC_OneDimReader::MIDDLE_PATTERN, 5, 0, e);
  if (e != BCExceptionNO) {
    FX_Free(result);
    return NULL;
  }
  for (i = 4; i <= 7; i++) {
    int32_t digit = FXSYS_atoi(contents.Mid(i, 1));
    pos += AppendPattern(result, pos, CBC_OneDimReader::L_PATTERNS[digit], 4, 1,
                         e);
    if (e != BCExceptionNO) {
      FX_Free(result);
      return NULL;
    }
  }
  pos +=
      AppendPattern(result, pos, CBC_OneDimReader::START_END_PATTERN, 3, 1, e);
  if (e != BCExceptionNO) {
    FX_Free(result);
    return NULL;
  }
  return result;
}
void CBC_OnedEAN8Writer::ShowChars(const CFX_WideStringC& contents,
                                   CFX_DIBitmap* pOutBitmap,
                                   CFX_RenderDevice* device,
                                   const CFX_Matrix* matrix,
                                   int32_t barWidth,
                                   int32_t multiple,
                                   int32_t& e) {
  if (device == NULL && pOutBitmap == NULL) {
    e = BCExceptionIllegalArgument;
    return;
  }
  int32_t leftPosition = 3 * multiple;
  CFX_ByteString str = FX_UTF8Encode(contents);
  int32_t iLength = str.GetLength();
  FXTEXT_CHARPOS* pCharPos = FX_Alloc(FXTEXT_CHARPOS, iLength);
  FXSYS_memset(pCharPos, 0, sizeof(FXTEXT_CHARPOS) * iLength);
  CFX_ByteString tempStr = str.Mid(0, 4);
  int32_t iLen = tempStr.GetLength();
  int32_t strWidth = 7 * multiple * 4;
  FX_FLOAT blank = 0.0;
  CFX_FxgeDevice geBitmap;
  if (pOutBitmap != NULL) {
    geBitmap.Attach(pOutBitmap);
  }
  FX_FLOAT charsWidth = 0;
  int32_t iFontSize = (int32_t)fabs(m_fFontSize);
  int32_t iTextHeight = iFontSize + 1;
  if (pOutBitmap == NULL) {
    CFX_Matrix matr(m_outputHScale, 0.0, 0.0, 1.0, 0.0, 0.0);
    CFX_FloatRect rect(
        (FX_FLOAT)leftPosition, (FX_FLOAT)(m_Height - iTextHeight),
        (FX_FLOAT)(leftPosition + strWidth - 0.5), (FX_FLOAT)m_Height);
    matr.Concat(*matrix);
    matr.TransformRect(rect);
    FX_RECT re = rect.GetOutterRect();
    device->FillRect(&re, m_backgroundColor);
    CFX_Matrix matr1(m_outputHScale, 0.0, 0.0, 1.0, 0.0, 0.0);
    CFX_FloatRect rect1(
        (FX_FLOAT)(leftPosition + 33 * multiple),
        (FX_FLOAT)(m_Height - iTextHeight),
        (FX_FLOAT)(leftPosition + 33 * multiple + strWidth - 0.5),
        (FX_FLOAT)m_Height);
    matr1.Concat(*matrix);
    matr1.TransformRect(rect1);
    re = rect1.GetOutterRect();
    device->FillRect(&re, m_backgroundColor);
  }
  if (pOutBitmap == NULL) {
    strWidth = (int32_t)(strWidth * m_outputHScale);
  }
  CalcTextInfo(tempStr, pCharPos, m_pFont, (FX_FLOAT)strWidth, iFontSize,
               blank);
  CFX_Matrix affine_matrix(1.0, 0.0, 0.0, -1.0, 0.0, (FX_FLOAT)iFontSize);
  CFX_FxgeDevice ge;
  if (pOutBitmap != NULL) {
    delete ge.GetBitmap();
    ge.Create(strWidth, iTextHeight, FXDIB_Argb);
    ge.GetBitmap()->Clear(m_backgroundColor);
    ge.DrawNormalText(iLen, pCharPos, m_pFont,
                      CFX_GEModule::Get()->GetFontCache(), (FX_FLOAT)iFontSize,
                      (CFX_Matrix*)&affine_matrix, m_fontColor,
                      FXTEXT_CLEARTYPE);
    geBitmap.SetDIBits(ge.GetBitmap(), leftPosition, m_Height - iTextHeight);
  } else {
    CFX_Matrix affine_matrix1(1.0, 0.0, 0.0, -1.0,
                              (FX_FLOAT)leftPosition * m_outputHScale,
                              (FX_FLOAT)(m_Height - iTextHeight + iFontSize));
    affine_matrix1.Concat(*matrix);
    device->DrawNormalText(iLen, pCharPos, m_pFont,
                           CFX_GEModule::Get()->GetFontCache(),
                           (FX_FLOAT)iFontSize, (CFX_Matrix*)&affine_matrix1,
                           m_fontColor, FXTEXT_CLEARTYPE);
  }
  tempStr = str.Mid(4, 4);
  iLen = tempStr.GetLength();
  charsWidth = 0.0f;
  CalcTextInfo(tempStr, pCharPos + 4, m_pFont, (FX_FLOAT)strWidth, iFontSize,
               blank);
  if (pOutBitmap != NULL) {
    delete ge.GetBitmap();
    ge.Create(strWidth, iTextHeight, FXDIB_Argb);
    ge.GetBitmap()->Clear(m_backgroundColor);
    ge.DrawNormalText(iLen, pCharPos + 4, m_pFont,
                      CFX_GEModule::Get()->GetFontCache(), (FX_FLOAT)iFontSize,
                      (CFX_Matrix*)&affine_matrix, m_fontColor,
                      FXTEXT_CLEARTYPE);
    geBitmap.SetDIBits(ge.GetBitmap(), leftPosition + 33 * multiple,
                       m_Height - iTextHeight);
  } else {
    CFX_Matrix affine_matrix1(
        1.0, 0.0, 0.0, -1.0,
        (FX_FLOAT)(leftPosition + 33 * multiple) * m_outputHScale,
        (FX_FLOAT)(m_Height - iTextHeight + iFontSize));
    if (matrix != NULL) {
      affine_matrix1.Concat(*matrix);
    }
    device->DrawNormalText(iLen, pCharPos + 4, m_pFont,
                           CFX_GEModule::Get()->GetFontCache(),
                           (FX_FLOAT)iFontSize, (CFX_Matrix*)&affine_matrix1,
                           m_fontColor, FXTEXT_CLEARTYPE);
  }
  FX_Free(pCharPos);
}
void CBC_OnedEAN8Writer::RenderResult(const CFX_WideStringC& contents,
                                      uint8_t* code,
                                      int32_t codeLength,
                                      FX_BOOL isDevice,
                                      int32_t& e) {
  CBC_OneDimWriter::RenderResult(contents, code, codeLength, isDevice, e);
}
