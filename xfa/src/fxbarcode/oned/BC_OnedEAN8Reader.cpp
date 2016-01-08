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

#include "xfa/src/fxbarcode/barcode.h"
#include "xfa/src/fxbarcode/BC_Reader.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitArray.h"
#include "BC_OneDReader.h"
#include "BC_OneDimReader.h"
#include "BC_OnedEAN8Reader.h"
CBC_OnedEAN8Reader::CBC_OnedEAN8Reader() {}
CBC_OnedEAN8Reader::~CBC_OnedEAN8Reader() {}
int32_t CBC_OnedEAN8Reader::DecodeMiddle(CBC_CommonBitArray* row,
                                         CFX_Int32Array* startRange,
                                         CFX_ByteString& resultResult,
                                         int32_t& e) {
  CFX_Int32Array counters;
  counters.Add(0);
  counters.Add(0);
  counters.Add(0);
  counters.Add(0);
  int32_t end = row->GetSize();
  int32_t rowOffset = (*startRange)[1];
  int32_t rowOffsetLeft = rowOffset;
  for (int32_t x = 0; x < 4 && rowOffset < end; x++) {
    int32_t bestMatch =
        DecodeDigit(row, &counters, rowOffset,
                    &(CBC_OneDimReader::L_PATTERNS[0][0]), 10, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, 0);
    resultResult += (FX_CHAR)('0' + bestMatch);
    for (int32_t i = 0; i < counters.GetSize(); i++) {
      rowOffset += counters[i];
    }
  }
  int32_t RowOffsetLen = (rowOffset - rowOffsetLeft) / 4;
  CFX_Int32Array result;
  result.Add(CBC_OneDimReader::MIDDLE_PATTERN[0]);
  result.Add(CBC_OneDimReader::MIDDLE_PATTERN[1]);
  result.Add(CBC_OneDimReader::MIDDLE_PATTERN[2]);
  result.Add(CBC_OneDimReader::MIDDLE_PATTERN[3]);
  result.Add(CBC_OneDimReader::MIDDLE_PATTERN[4]);
  CFX_Int32Array* middleRange =
      FindGuardPattern(row, rowOffset, TRUE, &result, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, 0);
  int32_t rowOffsetMid = rowOffset;
  rowOffset = (*middleRange)[1];
  if ((rowOffset - rowOffsetMid) > RowOffsetLen) {
    e = BCExceptionNotFound;
    BC_EXCEPTION_CHECK_ReturnValue(e, 0);
  }
  if (middleRange != NULL) {
    delete middleRange;
    middleRange = NULL;
  }
  for (int32_t y = 0; y < 4 && rowOffset < end; y++) {
    int32_t bestMatch =
        DecodeDigit(row, &counters, rowOffset,
                    &(CBC_OneDimReader::L_PATTERNS[0][0]), 10, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, 0);
    resultResult += (FX_CHAR)('0' + bestMatch);
    for (int32_t i = 0; i < counters.GetSize(); i++) {
      rowOffset += counters[i];
    }
  }
  return rowOffset;
}
