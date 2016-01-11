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
#include "xfa/src/fxbarcode/BC_BinaryBitmap.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitArray.h"
#include "BC_OneDReader.h"
const int32_t CBC_OneDReader::INTEGER_MATH_SHIFT = 8;
const int32_t CBC_OneDReader::PATTERN_MATCH_RESULT_SCALE_FACTOR = 1 << 8;
CBC_OneDReader::CBC_OneDReader() {}
CBC_OneDReader::~CBC_OneDReader() {}
CFX_ByteString CBC_OneDReader::Decode(CBC_BinaryBitmap* image, int32_t& e) {
  CFX_ByteString strtemp = Decode(image, 0, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  return strtemp;
}
CFX_ByteString CBC_OneDReader::Decode(CBC_BinaryBitmap* image,
                                      int32_t hints,
                                      int32_t& e) {
  CFX_ByteString strtemp = DeDecode(image, hints, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  return strtemp;
}
CFX_ByteString CBC_OneDReader::DeDecode(CBC_BinaryBitmap* image,
                                        int32_t hints,
                                        int32_t& e) {
  int32_t height = image->GetHeight();
  CBC_CommonBitArray* row = NULL;
  int32_t middle = height >> 1;
  FX_BOOL tryHarder = FALSE;
  int32_t rowStep = std::max(1, height >> (tryHarder ? 8 : 5));
  int32_t maxLines;
  if (tryHarder) {
    maxLines = height;
  } else {
    maxLines = 15;
  }
  for (int32_t x = 0; x < maxLines; x++) {
    int32_t rowStepsAboveOrBelow = (x + 1) >> 1;
    FX_BOOL isAbove = (x & 0x01) == 0;
    int32_t rowNumber =
        middle +
        rowStep * (isAbove ? rowStepsAboveOrBelow : -rowStepsAboveOrBelow);
    if (rowNumber < 0 || rowNumber >= height) {
      break;
    }
    row = image->GetBlackRow(rowNumber, NULL, e);
    if (e != BCExceptionNO) {
      e = BCExceptionNO;
      if (row != NULL) {
        delete row;
        row = NULL;
      }
      continue;
    }
    for (int32_t attempt = 0; attempt < 2; attempt++) {
      if (attempt == 1) {
        row->Reverse();
      }
      CFX_ByteString result = DecodeRow(rowNumber, row, hints, e);
      if (e != BCExceptionNO) {
        e = BCExceptionNO;
        continue;
      }
      if (row != NULL) {
        delete row;
        row = NULL;
      }
      return result;
    }
    if (row != NULL) {
      delete row;
      row = NULL;
    }
  }
  e = BCExceptionNotFound;
  return "";
}
void CBC_OneDReader::RecordPattern(CBC_CommonBitArray* row,
                                   int32_t start,
                                   CFX_Int32Array* counters,
                                   int32_t& e) {
  int32_t numCounters = counters->GetSize();
  for (int32_t i = 0; i < numCounters; i++) {
    (*counters)[i] = 0;
  }
  int32_t end = row->GetSize();
  if (start >= end) {
    e = BCExceptionNotFound;
    return;
  }
  FX_BOOL isWhite = !row->Get(start);
  int32_t counterPosition = 0;
  int32_t j = start;
  while (j < end) {
    FX_BOOL pixel = row->Get(j);
    if (pixel ^ isWhite) {
      (*counters)[counterPosition]++;
    } else {
      counterPosition++;
      if (counterPosition == numCounters) {
        break;
      } else {
        (*counters)[counterPosition] = 1;
        isWhite = !isWhite;
      }
    }
    j++;
  }
  if (!(counterPosition == numCounters ||
        (counterPosition == numCounters - 1 && j == end))) {
    e = BCExceptionNotFound;
    return;
  }
}
void CBC_OneDReader::RecordPatternInReverse(CBC_CommonBitArray* row,
                                            int32_t start,
                                            CFX_Int32Array* counters,
                                            int32_t& e) {
  int32_t numTransitionsLeft = counters->GetSize();
  FX_BOOL last = row->Get(start);
  while (start > 0 && numTransitionsLeft >= 0) {
    if (row->Get(--start) != last) {
      numTransitionsLeft--;
      last = !last;
    }
  }
  if (numTransitionsLeft >= 0) {
    e = BCExceptionNotFound;
    return;
  }
  RecordPattern(row, start + 1, counters, e);
  BC_EXCEPTION_CHECK_ReturnVoid(e);
}
int32_t CBC_OneDReader::PatternMatchVariance(CFX_Int32Array* counters,
                                             const int32_t* pattern,
                                             int32_t maxIndividualVariance) {
  int32_t numCounters = counters->GetSize();
  int32_t total = 0;
  int32_t patternLength = 0;
  for (int32_t i = 0; i < numCounters; i++) {
    total += (*counters)[i];
    patternLength += pattern[i];
  }
  if (total < patternLength) {
#undef max
    return FXSYS_IntMax;
  }
  int32_t unitBarWidth = (total << INTEGER_MATH_SHIFT) / patternLength;
  maxIndividualVariance =
      (maxIndividualVariance * unitBarWidth) >> INTEGER_MATH_SHIFT;
  int32_t totalVariance = 0;
  for (int32_t x = 0; x < numCounters; x++) {
    int32_t counter = (*counters)[x] << INTEGER_MATH_SHIFT;
    int32_t scaledPattern = pattern[x] * unitBarWidth;
    int32_t variance = counter > scaledPattern ? counter - scaledPattern
                                               : scaledPattern - counter;
    if (variance > maxIndividualVariance) {
#undef max
      return FXSYS_IntMax;
    }
    totalVariance += variance;
  }
  return totalVariance / total;
}
