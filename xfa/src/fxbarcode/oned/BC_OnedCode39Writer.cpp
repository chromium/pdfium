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

#include "xfa/src/fxbarcode/barcode.h"
#include "xfa/src/fxbarcode/BC_Writer.h"
#include "xfa/src/fxbarcode/BC_Reader.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitMatrix.h"
#include "BC_OneDReader.h"
#include "BC_OneDimWriter.h"
#include "BC_OnedCode39Reader.h"
#include "BC_OnedCode39Writer.h"
CBC_OnedCode39Writer::CBC_OnedCode39Writer() {
  m_extendedMode = FALSE;
  m_iWideNarrRatio = 3;
}
CBC_OnedCode39Writer::CBC_OnedCode39Writer(FX_BOOL extendedMode) {
  m_iWideNarrRatio = 3;
  m_extendedMode = extendedMode;
}
CBC_OnedCode39Writer::~CBC_OnedCode39Writer() {}
FX_BOOL CBC_OnedCode39Writer::CheckContentValidity(
    const CFX_WideStringC& contents) {
  if (m_extendedMode) {
    return CheckExtendedContentValidity(contents);
  }
  for (int32_t i = 0; i < contents.GetLength(); i++) {
    FX_WCHAR ch = contents.GetAt(i);
    if ((ch >= (FX_WCHAR)'0' && ch <= (FX_WCHAR)'9') ||
        (ch >= (FX_WCHAR)'A' && ch <= (FX_WCHAR)'Z') || ch == (FX_WCHAR)'-' ||
        ch == (FX_WCHAR)'.' || ch == (FX_WCHAR)' ' || ch == (FX_WCHAR)'*' ||
        ch == (FX_WCHAR)'$' || ch == (FX_WCHAR)'/' || ch == (FX_WCHAR)'+' ||
        ch == (FX_WCHAR)'%') {
      continue;
    }
    return FALSE;
  }
  return TRUE;
}
FX_BOOL CBC_OnedCode39Writer::CheckExtendedContentValidity(
    const CFX_WideStringC& contents) {
  for (int32_t i = 0; i < contents.GetLength(); i++) {
    FX_WCHAR ch = contents.GetAt(i);
    if (ch > 127) {
      return FALSE;
    }
  }
  return TRUE;
}
CFX_WideString CBC_OnedCode39Writer::FilterContents(
    const CFX_WideStringC& contents) {
  if (m_extendedMode) {
    return FilterExtendedContents(contents);
  }
  CFX_WideString filtercontents;
  for (int32_t i = 0; i < contents.GetLength(); i++) {
    FX_WCHAR ch = contents.GetAt(i);
    if (ch == (FX_WCHAR)'*' && (i == 0 || i == contents.GetLength() - 1)) {
      continue;
    }
    if (ch > 175) {
      i++;
      continue;
    } else {
      ch = Upper(ch);
    }
    if ((ch >= (FX_WCHAR)'0' && ch <= (FX_WCHAR)'9') ||
        (ch >= (FX_WCHAR)'A' && ch <= (FX_WCHAR)'Z') || ch == (FX_WCHAR)'-' ||
        ch == (FX_WCHAR)'.' || ch == (FX_WCHAR)' ' || ch == (FX_WCHAR)'*' ||
        ch == (FX_WCHAR)'$' || ch == (FX_WCHAR)'/' || ch == (FX_WCHAR)'+' ||
        ch == (FX_WCHAR)'%') {
      filtercontents += ch;
    }
  }
  return filtercontents;
}
CFX_WideString CBC_OnedCode39Writer::FilterExtendedContents(
    const CFX_WideStringC& contents) {
  CFX_WideString filtercontents;
  for (int32_t i = 0; i < contents.GetLength(); i++) {
    FX_WCHAR ch = contents.GetAt(i);
    if (ch == (FX_WCHAR)'*' && (i == 0 || i == contents.GetLength() - 1)) {
      continue;
    }
    if (ch > 175) {
      i++;
      continue;
    }
    if (ch > 127 && ch < 176) {
      continue;
    }
    if (ch == 0) {
      filtercontents += '%';
      filtercontents += 'U';
    } else if (ch >= 1 && ch <= 26) {
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
CFX_WideString CBC_OnedCode39Writer::RenderTextContents(
    const CFX_WideStringC& contents) {
  if (m_extendedMode) {
    return RenderExtendedTextContents(contents);
  }
  CFX_WideString renderContents;
  for (int32_t i = 0; i < contents.GetLength(); i++) {
    FX_WCHAR ch = contents.GetAt(i);
    if (ch == (FX_WCHAR)'*' && (i == 0 || i == contents.GetLength() - 1)) {
      continue;
    }
    if (ch > 175) {
      i++;
      continue;
    }
    if ((ch >= (FX_WCHAR)'0' && ch <= (FX_WCHAR)'9') ||
        (ch >= (FX_WCHAR)'A' && ch <= (FX_WCHAR)'Z') ||
        (ch >= (FX_WCHAR)'a' && ch <= (FX_WCHAR)'z') || ch == (FX_WCHAR)'-' ||
        ch == (FX_WCHAR)'.' || ch == (FX_WCHAR)' ' || ch == (FX_WCHAR)'*' ||
        ch == (FX_WCHAR)'$' || ch == (FX_WCHAR)'/' || ch == (FX_WCHAR)'+' ||
        ch == (FX_WCHAR)'%') {
      renderContents += ch;
    }
  }
  return renderContents;
}
CFX_WideString CBC_OnedCode39Writer::RenderExtendedTextContents(
    const CFX_WideStringC& contents) {
  CFX_WideString renderContents;
  for (int32_t i = 0; i < contents.GetLength(); i++) {
    FX_WCHAR ch = contents.GetAt(i);
    if (ch == (FX_WCHAR)'*' && (i == 0 || i == contents.GetLength() - 1)) {
      continue;
    }
    if (ch > 175) {
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
FX_BOOL CBC_OnedCode39Writer::SetTextLocation(BC_TEXT_LOC location) {
  if (location < BC_TEXT_LOC_NONE || location > BC_TEXT_LOC_BELOWEMBED) {
    return FALSE;
  }
  m_locTextLoc = location;
  return TRUE;
}
FX_BOOL CBC_OnedCode39Writer::SetWideNarrowRatio(int32_t ratio) {
  if (ratio < 2 || ratio > 3) {
    return FALSE;
  }
  m_iWideNarrRatio = ratio;
  return TRUE;
}
uint8_t* CBC_OnedCode39Writer::Encode(const CFX_ByteString& contents,
                                      BCFORMAT format,
                                      int32_t& outWidth,
                                      int32_t& outHeight,
                                      int32_t& e) {
  uint8_t* ret = Encode(contents, format, outWidth, outHeight, 0, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return ret;
}
uint8_t* CBC_OnedCode39Writer::Encode(const CFX_ByteString& contents,
                                      BCFORMAT format,
                                      int32_t& outWidth,
                                      int32_t& outHeight,
                                      int32_t hints,
                                      int32_t& e) {
  if (format != BCFORMAT_CODE_39) {
    e = BCExceptionOnlyEncodeCODE_39;
    return NULL;
  }
  uint8_t* ret =
      CBC_OneDimWriter::Encode(contents, format, outWidth, outHeight, hints, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return ret;
}
void CBC_OnedCode39Writer::ToIntArray(int32_t a, int32_t* toReturn) {
  for (int32_t i = 0; i < 9; i++) {
    toReturn[i] = (a & (1 << i)) == 0 ? 1 : m_iWideNarrRatio;
  }
}
FX_CHAR CBC_OnedCode39Writer::CalcCheckSum(const CFX_ByteString& contents,
                                           int32_t& e) {
  int32_t length = contents.GetLength();
  if (length > 80) {
    e = BCExceptionContentsLengthShouldBetween1and80;
    return '*';
  }
  int32_t checksum = 0;
  int32_t len = (int32_t)strlen(CBC_OnedCode39Reader::ALPHABET_STRING);
  for (int32_t i = 0; i < contents.GetLength(); i++) {
    int32_t j = 0;
    for (; j < len; j++) {
      if (CBC_OnedCode39Reader::ALPHABET_STRING[j] == contents[i]) {
        if (contents[i] != '*') {
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
uint8_t* CBC_OnedCode39Writer::Encode(const CFX_ByteString& contents,
                                      int32_t& outlength,
                                      int32_t& e) {
  FX_CHAR checksum = CalcCheckSum(contents, e);
  if (checksum == '*') {
    return NULL;
  }
  int32_t widths[9] = {0};
  int32_t wideStrideNum = 3;
  int32_t narrStrideNum = 9 - wideStrideNum;
  CFX_ByteString encodedContents = contents;
  if (m_bCalcChecksum) {
    encodedContents += checksum;
  }
  m_iContentLen = encodedContents.GetLength();
  int32_t codeWidth = (wideStrideNum * m_iWideNarrRatio + narrStrideNum) * 2 +
                      1 + m_iContentLen;
  int32_t len = (int32_t)strlen(CBC_OnedCode39Reader::ALPHABET_STRING);
  for (int32_t j = 0; j < m_iContentLen; j++) {
    for (int32_t i = 0; i < len; i++) {
      if (CBC_OnedCode39Reader::ALPHABET_STRING[i] == encodedContents[j]) {
        ToIntArray(CBC_OnedCode39Reader::CHARACTER_ENCODINGS[i], widths);
        for (int32_t k = 0; k < 9; k++) {
          codeWidth += widths[k];
        }
      }
    }
  }
  outlength = codeWidth;
  uint8_t* result = FX_Alloc(uint8_t, codeWidth);
  ToIntArray(CBC_OnedCode39Reader::CHARACTER_ENCODINGS[39], widths);
  int32_t pos = AppendPattern(result, 0, widths, 9, 1, e);
  if (e != BCExceptionNO) {
    FX_Free(result);
    return NULL;
  }
  int32_t narrowWhite[] = {1};
  pos += AppendPattern(result, pos, narrowWhite, 1, 0, e);
  if (e != BCExceptionNO) {
    FX_Free(result);
    return NULL;
  }
  for (int32_t l = m_iContentLen - 1; l >= 0; l--) {
    for (int32_t i = 0; i < len; i++) {
      if (CBC_OnedCode39Reader::ALPHABET_STRING[i] == encodedContents[l]) {
        ToIntArray(CBC_OnedCode39Reader::CHARACTER_ENCODINGS[i], widths);
        pos += AppendPattern(result, pos, widths, 9, 1, e);
        if (e != BCExceptionNO) {
          FX_Free(result);
          return NULL;
        }
      }
    }
    pos += AppendPattern(result, pos, narrowWhite, 1, 0, e);
    if (e != BCExceptionNO) {
      FX_Free(result);
      return NULL;
    }
  }
  ToIntArray(CBC_OnedCode39Reader::CHARACTER_ENCODINGS[39], widths);
  pos += AppendPattern(result, pos, widths, 9, 1, e);
  if (e != BCExceptionNO) {
    FX_Free(result);
    return NULL;
  }
  for (int32_t i = 0; i < codeWidth / 2; i++) {
    result[i] ^= result[codeWidth - 1 - i];
    result[codeWidth - 1 - i] ^= result[i];
    result[i] ^= result[codeWidth - 1 - i];
  }
  return result;
}
CFX_WideString CBC_OnedCode39Writer::encodedContents(
    const CFX_WideStringC& contents,
    int32_t& e) {
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
void CBC_OnedCode39Writer::RenderResult(const CFX_WideStringC& contents,
                                        uint8_t* code,
                                        int32_t codeLength,
                                        FX_BOOL isDevice,
                                        int32_t& e) {
  CFX_WideString encodedCon = encodedContents(contents, e);
  BC_EXCEPTION_CHECK_ReturnVoid(e);
  CBC_OneDimWriter::RenderResult(encodedCon, code, codeLength, isDevice, e);
}
