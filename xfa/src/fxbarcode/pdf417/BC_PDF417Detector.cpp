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
#include "xfa/src/fxbarcode/BC_ResultPoint.h"
#include "xfa/src/fxbarcode/BC_BinaryBitmap.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitMatrix.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitArray.h"
#include "BC_PDF417DetectorResult.h"
#include "BC_PDF417Detector.h"
#define INTERGER_MAX 2147483647
int32_t CBC_Detector::INDEXES_START_PATTERN[] = {0, 4, 1, 5};
int32_t CBC_Detector::INDEXES_STOP_PATTERN[] = {6, 2, 7, 3};
int32_t CBC_Detector::INTEGER_MATH_SHIFT = 8;
int32_t CBC_Detector::PATTERN_MATCH_RESULT_SCALE_FACTOR = 1
                                                          << INTEGER_MATH_SHIFT;
int32_t CBC_Detector::MAX_AVG_VARIANCE =
    (int32_t)(PATTERN_MATCH_RESULT_SCALE_FACTOR * 0.42f);
int32_t CBC_Detector::MAX_INDIVIDUAL_VARIANCE =
    (int32_t)(PATTERN_MATCH_RESULT_SCALE_FACTOR * 0.8f);
int32_t CBC_Detector::START_PATTERN[] = {8, 1, 1, 1, 1, 1, 1, 3};
int32_t CBC_Detector::STOP_PATTERN[] = {7, 1, 1, 3, 1, 1, 1, 2, 1};
int32_t CBC_Detector::MAX_PIXEL_DRIFT = 3;
int32_t CBC_Detector::MAX_PATTERN_DRIFT = 5;
int32_t CBC_Detector::SKIPPED_ROW_COUNT_MAX = 25;
int32_t CBC_Detector::ROW_STEP = 5;
int32_t CBC_Detector::BARCODE_MIN_HEIGHT = 10;
CBC_Detector::CBC_Detector() {}
CBC_Detector::~CBC_Detector() {}
CBC_PDF417DetectorResult* CBC_Detector::detect(CBC_BinaryBitmap* image,
                                               int32_t hints,
                                               FX_BOOL multiple,
                                               int32_t& e) {
  CBC_CommonBitMatrix* bitMatrix = image->GetBlackMatrix(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  CFX_PtrArray* barcodeCoordinates = detect(multiple, bitMatrix);
  if (barcodeCoordinates->GetSize() == 0) {
    rotate180(bitMatrix);
    barcodeCoordinates = detect(multiple, bitMatrix);
  }
  if (barcodeCoordinates->GetSize() == 0) {
    e = BCExceptionUnSupportedBarcode;
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  }
  CBC_PDF417DetectorResult* detectorResult =
      new CBC_PDF417DetectorResult(bitMatrix, barcodeCoordinates);
  return detectorResult;
}
void CBC_Detector::rotate180(CBC_CommonBitMatrix* bitMatrix) {
  int32_t width = bitMatrix->GetWidth();
  int32_t height = bitMatrix->GetHeight();
  CBC_CommonBitArray* firstRowBitArray = new CBC_CommonBitArray(width);
  CBC_CommonBitArray* secondRowBitArray = new CBC_CommonBitArray(width);
  CBC_CommonBitArray* tmpBitArray = new CBC_CommonBitArray(width);
  for (int32_t y = 0; y<(height + 1)>> 1; y++) {
    CBC_CommonBitArray* temp =
        bitMatrix->GetRow(height - 1 - y, secondRowBitArray);
    CBC_CommonBitArray* tempfirstRow = firstRowBitArray;
    firstRowBitArray = bitMatrix->GetRow(y, tempfirstRow);
    delete tempfirstRow;
    CBC_CommonBitArray* row = mirror(temp, tmpBitArray);
    delete temp;
    bitMatrix->SetRow(y, row);
    delete row;
    CBC_CommonBitArray* rowfirstRow = mirror(firstRowBitArray, tmpBitArray);
    bitMatrix->SetRow(height - 1 - y, rowfirstRow);
    delete rowfirstRow;
  }
  delete tmpBitArray;
  delete firstRowBitArray;
  delete secondRowBitArray;
}
CBC_CommonBitArray* CBC_Detector::mirror(CBC_CommonBitArray* input,
                                         CBC_CommonBitArray* result) {
  CBC_CommonBitArray* array = new CBC_CommonBitArray(result->GetSize());
  array->Clear();
  int32_t size = input->GetSize();
  for (int32_t i = 0; i < size; i++) {
    if (input->Get(i)) {
      array->Set(size - 1 - i);
    }
  }
  return array;
}
CFX_PtrArray* CBC_Detector::detect(FX_BOOL multiple,
                                   CBC_CommonBitMatrix* bitMatrix) {
  CFX_PtrArray* barcodeCoordinates = new CFX_PtrArray;
  int32_t row = 0;
  int32_t column = 0;
  FX_BOOL foundBarcodeInRow = FALSE;
  while (row < bitMatrix->GetHeight()) {
    CFX_PtrArray* vertices = findVertices(bitMatrix, row, column);
    if (vertices->GetAt(0) == NULL && vertices->GetAt(3) == NULL) {
      if (!foundBarcodeInRow) {
        if (vertices) {
          delete (vertices);
        }
        break;
      }
      foundBarcodeInRow = FALSE;
      column = 0;
      for (int32_t i = 0; i < barcodeCoordinates->GetSize(); i++) {
        CFX_PtrArray* barcodeCoordinate =
            (CFX_PtrArray*)barcodeCoordinates->GetAt(i);
        if (barcodeCoordinate->GetAt(1) != NULL) {
          row = row > ((CBC_ResultPoint*)barcodeCoordinate->GetAt(1))->GetY();
        }
        if (barcodeCoordinate->GetAt(3) != NULL) {
          row = row > ((CBC_ResultPoint*)barcodeCoordinate->GetAt(3))->GetY();
        }
      }
      row += ROW_STEP;
      if (vertices) {
        delete (vertices);
      }
      continue;
    }
    foundBarcodeInRow = TRUE;
    barcodeCoordinates->Add(vertices);
    if (!multiple) {
      break;
    }
    if (vertices->GetAt(2) != NULL) {
      column = (int32_t)((CBC_ResultPoint*)vertices->GetAt(2))->GetX();
      row = (int32_t)((CBC_ResultPoint*)vertices->GetAt(2))->GetY();
    } else {
      column = (int32_t)((CBC_ResultPoint*)vertices->GetAt(4))->GetX();
      row = (int32_t)((CBC_ResultPoint*)vertices->GetAt(4))->GetY();
    }
  }
  return barcodeCoordinates;
}
CFX_PtrArray* CBC_Detector::findVertices(CBC_CommonBitMatrix* matrix,
                                         int32_t startRow,
                                         int32_t startColumn) {
  int32_t height = matrix->GetHeight();
  int32_t width = matrix->GetWidth();
  CFX_PtrArray* result = new CFX_PtrArray;
  result->SetSize(8);
  CFX_PtrArray* startptr = findRowsWithPattern(
      matrix, height, width, startRow, startColumn, START_PATTERN,
      sizeof(START_PATTERN) / sizeof(START_PATTERN[0]));
  copyToResult(
      result, startptr, INDEXES_START_PATTERN,
      sizeof(INDEXES_START_PATTERN) / sizeof(INDEXES_START_PATTERN[0]));
  startptr->RemoveAll();
  delete startptr;
  if (result->GetAt(4) != NULL) {
    startColumn = (int32_t)((CBC_ResultPoint*)result->GetAt(4))->GetX();
    startRow = (int32_t)((CBC_ResultPoint*)result->GetAt(4))->GetY();
  }
  CFX_PtrArray* stopptr = findRowsWithPattern(
      matrix, height, width, startRow, startColumn, STOP_PATTERN,
      sizeof(STOP_PATTERN) / sizeof(STOP_PATTERN[0]));
  copyToResult(result, stopptr, INDEXES_STOP_PATTERN,
               sizeof(INDEXES_STOP_PATTERN) / sizeof(INDEXES_STOP_PATTERN[0]));
  stopptr->RemoveAll();
  delete stopptr;
  return result;
}
void CBC_Detector::copyToResult(CFX_PtrArray* result,
                                CFX_PtrArray* tmpResult,
                                int32_t* destinationIndexes,
                                int32_t destinationLength) {
  for (int32_t i = 0; i < destinationLength; i++) {
    result->SetAt(destinationIndexes[i], tmpResult->GetAt(i));
  }
}
CFX_PtrArray* CBC_Detector::findRowsWithPattern(CBC_CommonBitMatrix* matrix,
                                                int32_t height,
                                                int32_t width,
                                                int32_t startRow,
                                                int32_t startColumn,
                                                int32_t* pattern,
                                                int32_t patternLength) {
  CFX_PtrArray* result = new CFX_PtrArray;
  result->SetSize(4);
  FX_BOOL found = FALSE;
  CFX_Int32Array counters;
  counters.SetSize(patternLength);
  for (; startRow < height; startRow += ROW_STEP) {
    CFX_Int32Array* loc =
        findGuardPattern(matrix, startColumn, startRow, width, FALSE, pattern,
                         patternLength, counters);
    if (loc != NULL) {
      while (startRow > 0) {
        CFX_Int32Array* previousRowLoc =
            findGuardPattern(matrix, startColumn, --startRow, width, FALSE,
                             pattern, patternLength, counters);
        if (previousRowLoc != NULL) {
          delete loc;
          loc = previousRowLoc;
        } else {
          startRow++;
          break;
        }
      }
      result->SetAt(
          0, new CBC_ResultPoint((FX_FLOAT)loc->GetAt(0), (FX_FLOAT)startRow));
      result->SetAt(
          1, new CBC_ResultPoint((FX_FLOAT)loc->GetAt(1), (FX_FLOAT)startRow));
      found = TRUE;
      delete loc;
      break;
    }
  }
  int32_t stopRow = startRow + 1;
  if (found) {
    int32_t skippedRowCount = 0;
    CFX_Int32Array previousRowLoc;
    previousRowLoc.Add((int32_t)((CBC_ResultPoint*)result->GetAt(0))->GetX());
    previousRowLoc.Add((int32_t)((CBC_ResultPoint*)result->GetAt(1))->GetX());
    for (; stopRow < height; stopRow++) {
      CFX_Int32Array* loc =
          findGuardPattern(matrix, previousRowLoc[0], stopRow, width, FALSE,
                           pattern, patternLength, counters);
      if (loc != NULL &&
          abs(previousRowLoc[0] - loc->GetAt(0)) < MAX_PATTERN_DRIFT &&
          abs(previousRowLoc[1] - loc->GetAt(1)) < MAX_PATTERN_DRIFT) {
        previousRowLoc.Copy(*loc);
        skippedRowCount = 0;
      } else {
        if (skippedRowCount > SKIPPED_ROW_COUNT_MAX) {
          delete loc;
          break;
        } else {
          skippedRowCount++;
        }
      }
      delete loc;
    }
    stopRow -= skippedRowCount + 1;
    result->SetAt(2, new CBC_ResultPoint((FX_FLOAT)previousRowLoc.GetAt(0),
                                         (FX_FLOAT)stopRow));
    result->SetAt(3, new CBC_ResultPoint((FX_FLOAT)previousRowLoc.GetAt(1),
                                         (FX_FLOAT)stopRow));
  }
  if (stopRow - startRow < BARCODE_MIN_HEIGHT) {
    for (int32_t i = 0; i < result->GetSize(); i++) {
      result->SetAt(i, NULL);
    }
  }
  return result;
}
CFX_Int32Array* CBC_Detector::findGuardPattern(CBC_CommonBitMatrix* matrix,
                                               int32_t column,
                                               int32_t row,
                                               int32_t width,
                                               FX_BOOL whiteFirst,
                                               int32_t* pattern,
                                               int32_t patternLength,
                                               CFX_Int32Array& counters) {
  for (int32_t i = 0; i < counters.GetSize(); i++) {
    counters.SetAt(i, 0);
  }
  FX_BOOL isWhite = whiteFirst;
  int32_t patternStart = column;
  int32_t pixelDrift = 0;
  CFX_Int32Array* intarray = new CFX_Int32Array;
  while (matrix->Get(patternStart, row) && patternStart > 0 &&
         pixelDrift++ < MAX_PIXEL_DRIFT) {
    patternStart--;
  }
  int32_t x = patternStart;
  int32_t counterPosition = 0;
  for (; x < width; x++) {
    FX_BOOL pixel = matrix->Get(x, row);
    if (pixel ^ isWhite) {
      counters[counterPosition]++;
    } else {
      if (counterPosition == patternLength - 1) {
        if (patternMatchVariance(counters, pattern, MAX_INDIVIDUAL_VARIANCE) <
            MAX_AVG_VARIANCE) {
          intarray->Add(patternStart);
          intarray->Add(x);
          return intarray;
        }
        patternStart += counters[0] + counters[1];
        for (int32_t l = 2, k = 0; l < patternLength; l++, k++) {
          counters.SetAt(k, counters.GetAt(l));
        }
        counters.SetAt(patternLength - 2, 0);
        counters.SetAt(patternLength - 1, 0);
        counterPosition--;
      } else {
        counterPosition++;
      }
      counters[counterPosition] = 1;
      isWhite = !isWhite;
    }
  }
  if (counterPosition == patternLength - 1) {
    if (patternMatchVariance(counters, pattern, MAX_INDIVIDUAL_VARIANCE) <
        MAX_AVG_VARIANCE) {
      intarray->Add(patternStart);
      intarray->Add(x - 1);
      return intarray;
    }
  }
  delete intarray;
  return NULL;
}
int32_t CBC_Detector::patternMatchVariance(CFX_Int32Array& counters,
                                           int32_t* pattern,
                                           int32_t maxIndividualVariance) {
  int32_t numCounters = counters.GetSize();
  int32_t total = 0;
  int32_t patternLength = 0;
  for (int32_t i = 0; i < numCounters; i++) {
    total += counters[i];
    patternLength += pattern[i];
  }
  if (total < patternLength) {
    return INTERGER_MAX;
  }
  int32_t unitBarWidth = (total << INTEGER_MATH_SHIFT) / patternLength;
  maxIndividualVariance =
      (maxIndividualVariance * unitBarWidth) >> INTEGER_MATH_SHIFT;
  int32_t totalVariance = 0;
  for (int32_t x = 0; x < numCounters; x++) {
    int32_t counter = counters[x] << INTEGER_MATH_SHIFT;
    int32_t scaledPattern = pattern[x] * unitBarWidth;
    int32_t variance = counter > scaledPattern ? counter - scaledPattern
                                               : scaledPattern - counter;
    if (variance > maxIndividualVariance) {
      return INTERGER_MAX;
    }
    totalVariance += variance;
  }
  return totalVariance / total;
}
