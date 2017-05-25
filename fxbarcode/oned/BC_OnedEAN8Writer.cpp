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

#include "fxbarcode/oned/BC_OnedEAN8Writer.h"

#include <algorithm>
#include <cwctype>
#include <memory>
#include <vector>

#include "core/fxge/cfx_defaultrenderdevice.h"
#include "fxbarcode/BC_Writer.h"
#include "fxbarcode/common/BC_CommonBitMatrix.h"
#include "fxbarcode/oned/BC_OneDimWriter.h"

namespace {

const int8_t START_END_PATTERN[3] = {1, 1, 1};
const int8_t MIDDLE_PATTERN[5] = {1, 1, 1, 1, 1};
const int8_t L_PATTERNS[10][4] = {
    {3, 2, 1, 1}, {2, 2, 2, 1}, {2, 1, 2, 2}, {1, 4, 1, 1}, {1, 1, 3, 2},
    {1, 2, 3, 1}, {1, 1, 1, 4}, {1, 3, 1, 2}, {1, 2, 1, 3}, {3, 1, 1, 2}};

}  // namespace

CBC_OnedEAN8Writer::CBC_OnedEAN8Writer() {
  m_iDataLenth = 8;
  m_codeWidth = 3 + (7 * 4) + 5 + (7 * 4) + 3;
}

CBC_OnedEAN8Writer::~CBC_OnedEAN8Writer() {}

void CBC_OnedEAN8Writer::SetDataLength(int32_t length) {
  m_iDataLenth = 8;
}

bool CBC_OnedEAN8Writer::SetTextLocation(BC_TEXT_LOC location) {
  if (location == BC_TEXT_LOC_BELOWEMBED) {
    m_locTextLoc = location;
    return true;
  }
  return false;
}

bool CBC_OnedEAN8Writer::CheckContentValidity(const CFX_WideStringC& contents) {
  return std::all_of(contents.begin(), contents.end(), std::iswdigit);
}

CFX_WideString CBC_OnedEAN8Writer::FilterContents(
    const CFX_WideStringC& contents) {
  CFX_WideString filtercontents;
  wchar_t ch;
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
      odd += FXSYS_atoi(contents.Mid(i, 1).c_str());
    } else {
      even += FXSYS_atoi(contents.Mid(i, 1).c_str());
    }
    j++;
  }
  int32_t checksum = (odd * 3 + even) % 10;
  checksum = (10 - checksum) % 10;
  return (checksum);
}

uint8_t* CBC_OnedEAN8Writer::EncodeWithHint(const CFX_ByteString& contents,
                                            BCFORMAT format,
                                            int32_t& outWidth,
                                            int32_t& outHeight,
                                            int32_t hints) {
  if (format != BCFORMAT_EAN_8)
    return nullptr;
  return CBC_OneDimWriter::EncodeWithHint(contents, format, outWidth, outHeight,
                                          hints);
}

uint8_t* CBC_OnedEAN8Writer::EncodeImpl(const CFX_ByteString& contents,
                                        int32_t& outLength) {
  if (contents.GetLength() != 8)
    return nullptr;

  outLength = m_codeWidth;
  std::unique_ptr<uint8_t, FxFreeDeleter> result(
      FX_Alloc(uint8_t, m_codeWidth));
  int32_t pos = 0;
  int32_t e = BCExceptionNO;
  pos += AppendPattern(result.get(), pos, START_END_PATTERN, 3, 1, e);
  if (e != BCExceptionNO)
    return nullptr;

  int32_t i = 0;
  for (i = 0; i <= 3; i++) {
    int32_t digit = FXSYS_atoi(contents.Mid(i, 1).c_str());
    pos += AppendPattern(result.get(), pos, L_PATTERNS[digit], 4, 0, e);
    if (e != BCExceptionNO)
      return nullptr;
  }
  pos += AppendPattern(result.get(), pos, MIDDLE_PATTERN, 5, 0, e);
  if (e != BCExceptionNO)
    return nullptr;

  for (i = 4; i <= 7; i++) {
    int32_t digit = FXSYS_atoi(contents.Mid(i, 1).c_str());
    pos += AppendPattern(result.get(), pos, L_PATTERNS[digit], 4, 1, e);
    if (e != BCExceptionNO)
      return nullptr;
  }
  pos += AppendPattern(result.get(), pos, START_END_PATTERN, 3, 1, e);
  if (e != BCExceptionNO)
    return nullptr;
  return result.release();
}

bool CBC_OnedEAN8Writer::ShowChars(const CFX_WideStringC& contents,
                                   CFX_RenderDevice* device,
                                   const CFX_Matrix* matrix,
                                   int32_t barWidth,
                                   int32_t multiple) {
  if (!device)
    return false;

  int32_t leftPosition = 3 * multiple;
  CFX_ByteString str = FX_UTF8Encode(contents);
  int32_t iLength = str.GetLength();
  std::vector<FXTEXT_CHARPOS> charpos(iLength);
  CFX_ByteString tempStr = str.Mid(0, 4);
  int32_t iLen = tempStr.GetLength();
  int32_t strWidth = 7 * multiple * 4;
  float blank = 0.0;

  int32_t iFontSize = (int32_t)fabs(m_fFontSize);
  int32_t iTextHeight = iFontSize + 1;

  CFX_Matrix matr(m_outputHScale, 0.0, 0.0, 1.0, 0.0, 0.0);
  CFX_FloatRect rect((float)leftPosition, (float)(m_Height - iTextHeight),
                     (float)(leftPosition + strWidth - 0.5), (float)m_Height);
  matr.Concat(*matrix);
  matr.TransformRect(rect);
  FX_RECT re = rect.GetOuterRect();
  device->FillRect(&re, m_backgroundColor);
  CFX_Matrix matr1(m_outputHScale, 0.0, 0.0, 1.0, 0.0, 0.0);
  CFX_FloatRect rect1(
      (float)(leftPosition + 33 * multiple), (float)(m_Height - iTextHeight),
      (float)(leftPosition + 33 * multiple + strWidth - 0.5), (float)m_Height);
  matr1.Concat(*matrix);
  matr1.TransformRect(rect1);
  re = rect1.GetOuterRect();
  device->FillRect(&re, m_backgroundColor);
  strWidth = (int32_t)(strWidth * m_outputHScale);

  CalcTextInfo(tempStr, charpos.data(), m_pFont.Get(), (float)strWidth,
               iFontSize, blank);
  {
    CFX_Matrix affine_matrix1(1.0, 0.0, 0.0, -1.0,
                              (float)leftPosition * m_outputHScale,
                              (float)(m_Height - iTextHeight + iFontSize));
    affine_matrix1.Concat(*matrix);
    device->DrawNormalText(iLen, charpos.data(), m_pFont.Get(),
                           static_cast<float>(iFontSize), &affine_matrix1,
                           m_fontColor, FXTEXT_CLEARTYPE);
  }
  tempStr = str.Mid(4, 4);
  iLen = tempStr.GetLength();
  CalcTextInfo(tempStr, &charpos[4], m_pFont.Get(), (float)strWidth, iFontSize,
               blank);
  {
    CFX_Matrix affine_matrix1(
        1.0, 0.0, 0.0, -1.0,
        (float)(leftPosition + 33 * multiple) * m_outputHScale,
        (float)(m_Height - iTextHeight + iFontSize));
    if (matrix)
      affine_matrix1.Concat(*matrix);
    device->DrawNormalText(iLen, &charpos[4], m_pFont.Get(),
                           static_cast<float>(iFontSize), &affine_matrix1,
                           m_fontColor, FXTEXT_CLEARTYPE);
  }
  return true;
}
