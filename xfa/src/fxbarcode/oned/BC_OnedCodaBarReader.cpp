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
#include "xfa/src/fxbarcode/oned/BC_OneDReader.h"
#include "xfa/src/fxbarcode/oned/BC_OnedCode39Reader.h"
#include "xfa/src/fxbarcode/oned/BC_OnedCodaBarReader.h"
const FX_CHAR* CBC_OnedCodaBarReader::ALPHABET_STRING =
    "0123456789-$:/.+ABCDTN";
const int32_t CBC_OnedCodaBarReader::CHARACTER_ENCODINGS[22] = {
    0x003, 0x006, 0x009, 0x060, 0x012, 0x042, 0x021, 0x024,
    0x030, 0x048, 0x00c, 0x018, 0x045, 0x051, 0x054, 0x015,
    0x01A, 0x029, 0x00B, 0x00E, 0x01A, 0x029};
const int32_t CBC_OnedCodaBarReader::minCharacterLength = 3;
const FX_CHAR CBC_OnedCodaBarReader::STARTEND_ENCODING[8] = {
    'E', '*', 'A', 'B', 'C', 'D', 'T', 'N'};
CBC_OnedCodaBarReader::CBC_OnedCodaBarReader() {}
CBC_OnedCodaBarReader::~CBC_OnedCodaBarReader() {}
CFX_ByteString CBC_OnedCodaBarReader::DecodeRow(int32_t rowNumber,
                                                CBC_CommonBitArray* row,
                                                int32_t hints,
                                                int32_t& e) {
  CFX_Int32Array* int32Ptr = FindAsteriskPattern(row, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  CBC_AutoPtr<CFX_Int32Array> start(int32Ptr);
  (*start)[1] = 0;
  int32_t nextStart = (*start)[1];
  int32_t end = row->GetSize();
  while (nextStart < end && !row->Get(nextStart)) {
    nextStart++;
  }
  CFX_ByteString result;
  CFX_Int32Array counters;
  counters.SetSize(7);
  FX_CHAR decodedChar;
  int32_t lastStart;
  do {
    RecordPattern(row, nextStart, &counters, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, "");
    decodedChar = ToNarrowWidePattern(&counters);
    if (decodedChar == '!') {
      e = BCExceptionNotFound;
      return "";
    }
    result += decodedChar;
    lastStart = nextStart;
    for (int32_t i = 0; i < counters.GetSize(); i++) {
      nextStart += counters[i];
    }
    while (nextStart < end && !row->Get(nextStart)) {
      nextStart++;
    }
  } while (nextStart < end);
  int32_t lastPatternSize = 0;
  for (int32_t j = 0; j < counters.GetSize(); j++) {
    lastPatternSize += counters[j];
  }
  int32_t whiteSpaceAfterEnd = nextStart - lastStart - lastPatternSize;
  if (nextStart != end && (whiteSpaceAfterEnd / 2 < lastPatternSize)) {
    e = BCExceptionNotFound;
    return "";
  }
  if (result.GetLength() < 2) {
    e = BCExceptionNotFound;
    return "";
  }
  FX_CHAR startchar = result[0];
  if (!ArrayContains(STARTEND_ENCODING, startchar)) {
    e = BCExceptionNotFound;
    return "";
  }
  int32_t len = result.GetLength();
  CFX_ByteString temp = result;
  for (int32_t k = 1; k < result.GetLength(); k++) {
    if (ArrayContains(STARTEND_ENCODING, result[k])) {
      if ((k + 1) != result.GetLength()) {
        result.Delete(1, k);
        k = 1;
      }
    }
  }
  if (result.GetLength() < 5) {
    int32_t index = temp.Find(result.Mid(1, result.GetLength() - 1));
    if (index == len - (result.GetLength() - 1)) {
      e = BCExceptionNotFound;
      return "";
    }
  }
  if (result.GetLength() > minCharacterLength) {
    result = result.Mid(1, result.GetLength() - 2);
  } else {
    e = BCExceptionNotFound;
    return "";
  }
  return result;
}
CFX_Int32Array* CBC_OnedCodaBarReader::FindAsteriskPattern(
    CBC_CommonBitArray* row,
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
  counters.SetSize(7);
  int32_t patternStart = rowOffset;
  FX_BOOL isWhite = FALSE;
  int32_t patternLength = counters.GetSize();
  for (int32_t i = rowOffset; i < width; i++) {
    FX_BOOL pixel = row->Get(i);
    if (pixel ^ isWhite) {
      counters[counterPosition]++;
    } else {
      if (counterPosition == patternLength - 1) {
        if (ArrayContains(STARTEND_ENCODING, ToNarrowWidePattern(&counters))) {
          FX_BOOL btemp3 =
              row->IsRange(std::max(0, patternStart - (i - patternStart) / 2),
                           patternStart, FALSE, e);
          BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
          if (btemp3) {
            CFX_Int32Array* result = new CFX_Int32Array();
            result->SetSize(2);
            (*result)[0] = patternStart;
            (*result)[1] = i;
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
FX_BOOL CBC_OnedCodaBarReader::ArrayContains(const FX_CHAR array[],
                                             FX_CHAR key) {
  for (int32_t i = 0; i < 8; i++) {
    if (array[i] == key) {
      return TRUE;
    }
  }
  return FALSE;
}
FX_CHAR CBC_OnedCodaBarReader::ToNarrowWidePattern(CFX_Int32Array* counter) {
  int32_t numCounters = counter->GetSize();
  if (numCounters < 1) {
    return '!';
  }
  int32_t averageCounter = 0;
  int32_t totalCounters = 0;
  for (int32_t i = 0; i < numCounters; i++) {
    totalCounters += (*counter)[i];
  }
  averageCounter = totalCounters / numCounters;
  int32_t pattern = 0;
  int32_t wideCounters = 0;
  for (int32_t j = 0; j < numCounters; j++) {
    if ((*counter)[j] > averageCounter) {
      pattern |= 1 << (numCounters - 1 - j);
      wideCounters++;
    }
  }
  if ((wideCounters == 2) || (wideCounters == 3)) {
    for (int32_t k = 0; k < 22; k++) {
      if (CHARACTER_ENCODINGS[k] == pattern) {
        return (ALPHABET_STRING)[k];
      }
    }
  }
  return '!';
}
