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

#include "fxbarcode/oned/BC_OnedCodaBarWriter.h"

#include "fxbarcode/BC_Writer.h"
#include "fxbarcode/common/BC_CommonBitArray.h"
#include "fxbarcode/common/BC_CommonBitMatrix.h"
#include "fxbarcode/oned/BC_OneDimWriter.h"
#include "third_party/base/stl_util.h"

namespace {

const char ALPHABET_STRING[] = "0123456789-$:/.+ABCDTN";

const int32_t CHARACTER_ENCODINGS[22] = {
    0x003, 0x006, 0x009, 0x060, 0x012, 0x042, 0x021, 0x024,
    0x030, 0x048, 0x00c, 0x018, 0x045, 0x051, 0x054, 0x015,
    0x01A, 0x029, 0x00B, 0x00E, 0x01A, 0x029};

const char START_END_CHARS[] = {'A', 'B', 'C', 'D', 'T', 'N', '*', 'E',
                                'a', 'b', 'c', 'd', 't', 'n', 'e'};
const char CONTENT_CHARS[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                              '8', '9', '-', '$', '/', ':', '+', '.'};

}  // namespace

CBC_OnedCodaBarWriter::CBC_OnedCodaBarWriter()
    : m_chStart('A'), m_chEnd('B'), m_iWideNarrRatio(2) {}

CBC_OnedCodaBarWriter::~CBC_OnedCodaBarWriter() {}

bool CBC_OnedCodaBarWriter::SetStartChar(char start) {
  if (!pdfium::ContainsValue(START_END_CHARS, start))
    return false;

  m_chStart = start;
  return true;
}

bool CBC_OnedCodaBarWriter::SetEndChar(char end) {
  if (!pdfium::ContainsValue(START_END_CHARS, end))
    return false;

  m_chEnd = end;
  return true;
}

void CBC_OnedCodaBarWriter::SetDataLength(int32_t length) {
  m_iDataLenth = length + 2;
}

bool CBC_OnedCodaBarWriter::SetTextLocation(BC_TEXT_LOC location) {
  if (location < BC_TEXT_LOC_NONE || location > BC_TEXT_LOC_BELOWEMBED) {
    return false;
  }
  m_locTextLoc = location;
  return true;
}

bool CBC_OnedCodaBarWriter::SetWideNarrowRatio(int8_t ratio) {
  if (ratio < 2 || ratio > 3)
    return false;

  m_iWideNarrRatio = ratio;
  return true;
}

bool CBC_OnedCodaBarWriter::FindChar(wchar_t ch, bool isContent) {
  if (ch > 0x7F)
    return false;

  char narrow_ch = static_cast<char>(ch);
  return pdfium::ContainsValue(CONTENT_CHARS, narrow_ch) ||
         (isContent && pdfium::ContainsValue(START_END_CHARS, narrow_ch));
}

bool CBC_OnedCodaBarWriter::CheckContentValidity(
    const CFX_WideStringC& contents) {
  return std::all_of(
      contents.begin(), contents.end(),
      [this](const wchar_t& ch) { return this->FindChar(ch, false); });
}

CFX_WideString CBC_OnedCodaBarWriter::FilterContents(
    const CFX_WideStringC& contents) {
  CFX_WideString filtercontents;
  wchar_t ch;
  for (int32_t index = 0; index < contents.GetLength(); index++) {
    ch = contents.GetAt(index);
    if (ch > 175) {
      index++;
      continue;
    }
    if (!FindChar(ch, true))
      continue;
    filtercontents += ch;
  }
  return filtercontents;
}

uint8_t* CBC_OnedCodaBarWriter::EncodeWithHint(const CFX_ByteString& contents,
                                               BCFORMAT format,
                                               int32_t& outWidth,
                                               int32_t& outHeight,
                                               int32_t hints) {
  if (format != BCFORMAT_CODABAR)
    return nullptr;
  return CBC_OneDimWriter::EncodeWithHint(contents, format, outWidth, outHeight,
                                          hints);
}

uint8_t* CBC_OnedCodaBarWriter::EncodeImpl(const CFX_ByteString& contents,
                                           int32_t& outLength) {
  CFX_ByteString data = m_chStart + contents + m_chEnd;
  m_iContentLen = data.GetLength();
  uint8_t* result = FX_Alloc2D(uint8_t, m_iWideNarrRatio * 7, data.GetLength());
  char ch;
  int32_t position = 0;
  for (int32_t index = 0; index < data.GetLength(); index++) {
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
    int32_t code = 0;
    size_t len = strlen(ALPHABET_STRING);
    for (size_t i = 0; i < len; i++) {
      if (ch == ALPHABET_STRING[i]) {
        code = CHARACTER_ENCODINGS[i];
        break;
      }
    }
    uint8_t color = 1;
    int32_t counter = 0;
    int32_t bit = 0;
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
      position++;
    }
  }
  outLength = position;
  return result;
}

CFX_WideString CBC_OnedCodaBarWriter::encodedContents(
    const CFX_WideStringC& contents) {
  CFX_WideString strStart(static_cast<wchar_t>(m_chStart));
  CFX_WideString strEnd(static_cast<wchar_t>(m_chEnd));
  return strStart + contents + strEnd;
}

bool CBC_OnedCodaBarWriter::RenderResult(const CFX_WideStringC& contents,
                                         uint8_t* code,
                                         int32_t codeLength,
                                         bool isDevice) {
  return CBC_OneDimWriter::RenderResult(encodedContents(contents).AsStringC(),
                                        code, codeLength, isDevice);
}
