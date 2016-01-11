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

#include <algorithm>

#include "xfa/src/fxbarcode/barcode.h"
#include "xfa/src/fxbarcode/BC_Reader.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitArray.h"
#include "BC_OneDReader.h"
#include "BC_OnedCode128Reader.h"
const int32_t CBC_OnedCode128Reader::CODE_PATTERNS[107][7] = {
    {2, 1, 2, 2, 2, 2, 0}, {2, 2, 2, 1, 2, 2, 0}, {2, 2, 2, 2, 2, 1, 0},
    {1, 2, 1, 2, 2, 3, 0}, {1, 2, 1, 3, 2, 2, 0}, {1, 3, 1, 2, 2, 2, 0},
    {1, 2, 2, 2, 1, 3, 0}, {1, 2, 2, 3, 1, 2, 0}, {1, 3, 2, 2, 1, 2, 0},
    {2, 2, 1, 2, 1, 3, 0}, {2, 2, 1, 3, 1, 2, 0}, {2, 3, 1, 2, 1, 2, 0},
    {1, 1, 2, 2, 3, 2, 0}, {1, 2, 2, 1, 3, 2, 0}, {1, 2, 2, 2, 3, 1, 0},
    {1, 1, 3, 2, 2, 2, 0}, {1, 2, 3, 1, 2, 2, 0}, {1, 2, 3, 2, 2, 1, 0},
    {2, 2, 3, 2, 1, 1, 0}, {2, 2, 1, 1, 3, 2, 0}, {2, 2, 1, 2, 3, 1, 0},
    {2, 1, 3, 2, 1, 2, 0}, {2, 2, 3, 1, 1, 2, 0}, {3, 1, 2, 1, 3, 1, 0},
    {3, 1, 1, 2, 2, 2, 0}, {3, 2, 1, 1, 2, 2, 0}, {3, 2, 1, 2, 2, 1, 0},
    {3, 1, 2, 2, 1, 2, 0}, {3, 2, 2, 1, 1, 2, 0}, {3, 2, 2, 2, 1, 1, 0},
    {2, 1, 2, 1, 2, 3, 0}, {2, 1, 2, 3, 2, 1, 0}, {2, 3, 2, 1, 2, 1, 0},
    {1, 1, 1, 3, 2, 3, 0}, {1, 3, 1, 1, 2, 3, 0}, {1, 3, 1, 3, 2, 1, 0},
    {1, 1, 2, 3, 1, 3, 0}, {1, 3, 2, 1, 1, 3, 0}, {1, 3, 2, 3, 1, 1, 0},
    {2, 1, 1, 3, 1, 3, 0}, {2, 3, 1, 1, 1, 3, 0}, {2, 3, 1, 3, 1, 1, 0},
    {1, 1, 2, 1, 3, 3, 0}, {1, 1, 2, 3, 3, 1, 0}, {1, 3, 2, 1, 3, 1, 0},
    {1, 1, 3, 1, 2, 3, 0}, {1, 1, 3, 3, 2, 1, 0}, {1, 3, 3, 1, 2, 1, 0},
    {3, 1, 3, 1, 2, 1, 0}, {2, 1, 1, 3, 3, 1, 0}, {2, 3, 1, 1, 3, 1, 0},
    {2, 1, 3, 1, 1, 3, 0}, {2, 1, 3, 3, 1, 1, 0}, {2, 1, 3, 1, 3, 1, 0},
    {3, 1, 1, 1, 2, 3, 0}, {3, 1, 1, 3, 2, 1, 0}, {3, 3, 1, 1, 2, 1, 0},
    {3, 1, 2, 1, 1, 3, 0}, {3, 1, 2, 3, 1, 1, 0}, {3, 3, 2, 1, 1, 1, 0},
    {3, 1, 4, 1, 1, 1, 0}, {2, 2, 1, 4, 1, 1, 0}, {4, 3, 1, 1, 1, 1, 0},
    {1, 1, 1, 2, 2, 4, 0}, {1, 1, 1, 4, 2, 2, 0}, {1, 2, 1, 1, 2, 4, 0},
    {1, 2, 1, 4, 2, 1, 0}, {1, 4, 1, 1, 2, 2, 0}, {1, 4, 1, 2, 2, 1, 0},
    {1, 1, 2, 2, 1, 4, 0}, {1, 1, 2, 4, 1, 2, 0}, {1, 2, 2, 1, 1, 4, 0},
    {1, 2, 2, 4, 1, 1, 0}, {1, 4, 2, 1, 1, 2, 0}, {1, 4, 2, 2, 1, 1, 0},
    {2, 4, 1, 2, 1, 1, 0}, {2, 2, 1, 1, 1, 4, 0}, {4, 1, 3, 1, 1, 1, 0},
    {2, 4, 1, 1, 1, 2, 0}, {1, 3, 4, 1, 1, 1, 0}, {1, 1, 1, 2, 4, 2, 0},
    {1, 2, 1, 1, 4, 2, 0}, {1, 2, 1, 2, 4, 1, 0}, {1, 1, 4, 2, 1, 2, 0},
    {1, 2, 4, 1, 1, 2, 0}, {1, 2, 4, 2, 1, 1, 0}, {4, 1, 1, 2, 1, 2, 0},
    {4, 2, 1, 1, 1, 2, 0}, {4, 2, 1, 2, 1, 1, 0}, {2, 1, 2, 1, 4, 1, 0},
    {2, 1, 4, 1, 2, 1, 0}, {4, 1, 2, 1, 2, 1, 0}, {1, 1, 1, 1, 4, 3, 0},
    {1, 1, 1, 3, 4, 1, 0}, {1, 3, 1, 1, 4, 1, 0}, {1, 1, 4, 1, 1, 3, 0},
    {1, 1, 4, 3, 1, 1, 0}, {4, 1, 1, 1, 1, 3, 0}, {4, 1, 1, 3, 1, 1, 0},
    {1, 1, 3, 1, 4, 1, 0}, {1, 1, 4, 1, 3, 1, 0}, {3, 1, 1, 1, 4, 1, 0},
    {4, 1, 1, 1, 3, 1, 0}, {2, 1, 1, 4, 1, 2, 0}, {2, 1, 1, 2, 1, 4, 0},
    {2, 1, 1, 2, 3, 2, 0}, {2, 3, 3, 1, 1, 1, 2}};
const int32_t CBC_OnedCode128Reader::MAX_AVG_VARIANCE = (int32_t)(256 * 0.25f);
const int32_t CBC_OnedCode128Reader::MAX_INDIVIDUAL_VARIANCE =
    (int32_t)(256 * 0.7f);
const int32_t CBC_OnedCode128Reader::CODE_SHIFT = 98;
const int32_t CBC_OnedCode128Reader::CODE_CODE_C = 99;
const int32_t CBC_OnedCode128Reader::CODE_CODE_B = 100;
const int32_t CBC_OnedCode128Reader::CODE_CODE_A = 101;
const int32_t CBC_OnedCode128Reader::CODE_FNC_1 = 102;
const int32_t CBC_OnedCode128Reader::CODE_FNC_2 = 97;
const int32_t CBC_OnedCode128Reader::CODE_FNC_3 = 96;
const int32_t CBC_OnedCode128Reader::CODE_FNC_4_A = 101;
const int32_t CBC_OnedCode128Reader::CODE_FNC_4_B = 100;
const int32_t CBC_OnedCode128Reader::CODE_START_A = 103;
const int32_t CBC_OnedCode128Reader::CODE_START_B = 104;
const int32_t CBC_OnedCode128Reader::CODE_START_C = 105;
const int32_t CBC_OnedCode128Reader::CODE_STOP = 106;
CBC_OnedCode128Reader::CBC_OnedCode128Reader() {}
CBC_OnedCode128Reader::~CBC_OnedCode128Reader() {}
CFX_Int32Array* CBC_OnedCode128Reader::FindStartPattern(CBC_CommonBitArray* row,
                                                        int32_t& e) {
  int32_t width = row->GetSize();
  int32_t rowOffset = 0;
  while (rowOffset < width) {
    if (row->Get(rowOffset)) {
      break;
    }
    rowOffset++;
  }
  int32_t counterPosition = 0;
  CFX_Int32Array counters;
  counters.SetSize(6);
  int32_t patternStart = rowOffset;
  FX_BOOL isWhite = FALSE;
  int32_t patternLength = counters.GetSize();
  for (int32_t i = rowOffset; i < width; i++) {
    FX_BOOL pixel = row->Get(i);
    if (pixel ^ isWhite) {
      counters[counterPosition]++;
    } else {
      if (counterPosition == patternLength - 1) {
        int32_t bestVariance = MAX_AVG_VARIANCE;
        int32_t bestMatch = -1;
        for (int32_t startCode = CODE_START_A; startCode <= CODE_START_C;
             startCode++) {
          int32_t variance = PatternMatchVariance(
              &counters, &CODE_PATTERNS[startCode][0], MAX_INDIVIDUAL_VARIANCE);
          if (variance < bestVariance) {
            bestVariance = variance;
            bestMatch = startCode;
          }
        }
        if (bestMatch >= 0) {
          FX_BOOL btemp2 =
              row->IsRange(std::max(0, patternStart - (i - patternStart) / 2),
                           patternStart, FALSE, e);
          BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
          if (btemp2) {
            CFX_Int32Array* result = new CFX_Int32Array;
            result->SetSize(3);
            (*result)[0] = patternStart;
            (*result)[1] = i;
            (*result)[2] = bestMatch;
            return result;
          }
        }
        patternStart += counters[0] + counters[1];
        for (int32_t y = 2; y < patternLength; y++) {
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
int32_t CBC_OnedCode128Reader::DecodeCode(CBC_CommonBitArray* row,
                                          CFX_Int32Array* counters,
                                          int32_t rowOffset,
                                          int32_t& e) {
  RecordPattern(row, rowOffset, counters, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, 0);
  int32_t bestVariance = MAX_AVG_VARIANCE;
  int32_t bestMatch = -1;
  for (int32_t d = 0; d < 107; d++) {
    int32_t variance = PatternMatchVariance(counters, &CODE_PATTERNS[d][0],
                                            MAX_INDIVIDUAL_VARIANCE);
    if (variance < bestVariance) {
      bestVariance = variance;
      bestMatch = d;
    }
  }
  if (bestMatch >= 0) {
    return bestMatch;
  } else {
    e = BCExceptionNotFound;
    return 0;
  }
  return 0;
}
CFX_ByteString CBC_OnedCode128Reader::DecodeRow(int32_t rowNumber,
                                                CBC_CommonBitArray* row,
                                                int32_t hints,
                                                int32_t& e) {
  CFX_Int32Array* startPatternInfo = FindStartPattern(row, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  int32_t startCode = (*startPatternInfo)[2];
  int32_t codeSet;
  switch (startCode) {
    case 103:
      codeSet = CODE_CODE_A;
      break;
    case 104:
      codeSet = CODE_CODE_B;
      break;
    case 105:
      codeSet = CODE_CODE_C;
      break;
    default:
      if (startPatternInfo != NULL) {
        startPatternInfo->RemoveAll();
        delete startPatternInfo;
        startPatternInfo = NULL;
      }
      e = BCExceptionFormatException;
      return "";
  }
  FX_BOOL done = FALSE;
  FX_BOOL isNextShifted = FALSE;
  CFX_ByteString result;
  int32_t lastStart = (*startPatternInfo)[0];
  int32_t nextStart = (*startPatternInfo)[1];
  if (startPatternInfo != NULL) {
    startPatternInfo->RemoveAll();
    delete startPatternInfo;
    startPatternInfo = NULL;
  }
  CFX_Int32Array counters;
  counters.SetSize(6);
  int32_t lastCode = 0;
  int32_t code = 0;
  int32_t checksumTotal = startCode;
  int32_t multiplier = 0;
  FX_BOOL lastCharacterWasPrintable = TRUE;
  while (!done) {
    FX_BOOL unshift = isNextShifted;
    isNextShifted = FALSE;
    lastCode = code;
    code = DecodeCode(row, &counters, nextStart, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    if (code != CODE_STOP) {
      lastCharacterWasPrintable = TRUE;
    }
    if (code != CODE_STOP) {
      multiplier++;
      checksumTotal += multiplier * code;
    }
    lastStart = nextStart;
    for (int32_t i = 0; i < counters.GetSize(); i++) {
      nextStart += counters[i];
    }
    switch (code) {
      case 103:
      case 104:
      case 105:
        e = BCExceptionFormatException;
        return "";
    }
    switch (codeSet) {
      case 101:
        if (code < 64) {
          result += (FX_CHAR)(' ' + code);
        } else if (code < 96) {
          result += (FX_CHAR)(code - 64);
        } else {
          if (code != CODE_STOP) {
            lastCharacterWasPrintable = FALSE;
          }
          switch (code) {
            case 102:
            case 97:
            case 96:
            case 101:
              break;
            case 98:
              isNextShifted = TRUE;
              codeSet = CODE_CODE_B;
              break;
            case 100:
              codeSet = CODE_CODE_B;
              break;
            case 99:
              codeSet = CODE_CODE_C;
              break;
            case 106:
              done = TRUE;
              break;
          }
        }
        break;
      case 100:
        if (code < 96) {
          result += (FX_CHAR)(' ' + code);
        } else {
          if (code != CODE_STOP) {
            lastCharacterWasPrintable = FALSE;
          }
          switch (code) {
            case 102:
            case 97:
            case 96:
            case 100:
              break;
            case 98:
              isNextShifted = TRUE;
              codeSet = CODE_CODE_A;
              break;
            case 101:
              codeSet = CODE_CODE_A;
              break;
            case 99:
              codeSet = CODE_CODE_C;
              break;
            case 106:
              done = TRUE;
              break;
          }
        }
        break;
      case 99:
        if (code < 100) {
          if (code < 10) {
            result += '0';
          }
          FX_CHAR temp[128];
#if defined(_FX_WINAPI_PARTITION_APP_)
          sprintf_s(temp, 128, "%d", code);
#else
          sprintf(temp, "%d", code);
#endif
          result += temp;
        } else {
          if (code != CODE_STOP) {
            lastCharacterWasPrintable = FALSE;
          }
          switch (code) {
            case 102:
              break;
            case 101:
              codeSet = CODE_CODE_A;
              break;
            case 100:
              codeSet = CODE_CODE_B;
              break;
            case 106:
              done = TRUE;
              break;
          }
        }
        break;
    }
    if (unshift) {
      codeSet = codeSet == CODE_CODE_A ? CODE_CODE_B : CODE_CODE_A;
    }
  }
  int32_t width = row->GetSize();
  while (nextStart < width && row->Get(nextStart)) {
    nextStart++;
  }
  FX_BOOL boolT1 = row->IsRange(
      nextStart, std::min(width, nextStart + (nextStart - lastStart) / 2),
      FALSE, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  if (!boolT1) {
    e = BCExceptionNotFound;
    return "";
  }
  checksumTotal -= multiplier * lastCode;
  if (checksumTotal % 103 != lastCode) {
    e = BCExceptionChecksumException;
    return "";
  }
  int32_t resultLength = result.GetLength();
  if (resultLength > 0 && lastCharacterWasPrintable) {
    if (codeSet == CODE_CODE_C) {
      result = result.Mid(0, result.GetLength() - 2);
    } else {
      result = result.Mid(0, result.GetLength() - 1);
    }
  }
  if (result.GetLength() == 0) {
    e = BCExceptionFormatException;
    return "";
  }
  return result;
}
