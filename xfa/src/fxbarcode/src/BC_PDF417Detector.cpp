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

#include "barcode.h"
#include "include/BC_ResultPoint.h"
#include "include/BC_PDF417DetectorResult.h"
#include "include/BC_BinaryBitmap.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_CommonBitArray.h"
#include "include/BC_PDF417Detector.h"
#define  INTERGER_MAX     2147483647
FX_INT32 CBC_Detector::INDEXES_START_PATTERN[] = {0, 4, 1, 5};
FX_INT32 CBC_Detector::INDEXES_STOP_PATTERN[] = {6, 2, 7, 3};
FX_INT32 CBC_Detector::INTEGER_MATH_SHIFT = 8;
FX_INT32 CBC_Detector::PATTERN_MATCH_RESULT_SCALE_FACTOR = 1 << INTEGER_MATH_SHIFT;
FX_INT32 CBC_Detector::MAX_AVG_VARIANCE = (FX_INT32) (PATTERN_MATCH_RESULT_SCALE_FACTOR * 0.42f);
FX_INT32 CBC_Detector::MAX_INDIVIDUAL_VARIANCE = (FX_INT32) (PATTERN_MATCH_RESULT_SCALE_FACTOR * 0.8f);
FX_INT32 CBC_Detector::START_PATTERN[] = {8, 1, 1, 1, 1, 1, 1, 3};
FX_INT32 CBC_Detector::STOP_PATTERN[] = {7, 1, 1, 3, 1, 1, 1, 2, 1};
FX_INT32 CBC_Detector::MAX_PIXEL_DRIFT = 3;
FX_INT32 CBC_Detector::MAX_PATTERN_DRIFT = 5;
FX_INT32 CBC_Detector::SKIPPED_ROW_COUNT_MAX = 25;
FX_INT32 CBC_Detector::ROW_STEP = 5;
FX_INT32 CBC_Detector::BARCODE_MIN_HEIGHT = 10;
CBC_Detector::CBC_Detector()
{
}
CBC_Detector::~CBC_Detector()
{
}
CBC_PDF417DetectorResult* CBC_Detector::detect(CBC_BinaryBitmap* image, FX_INT32 hints, FX_BOOL multiple, FX_INT32 &e)
{
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
    CBC_PDF417DetectorResult* detectorResult = FX_NEW CBC_PDF417DetectorResult(bitMatrix, barcodeCoordinates);
    return detectorResult;
}
void CBC_Detector::rotate180(CBC_CommonBitMatrix* bitMatrix)
{
    FX_INT32 width = bitMatrix->GetWidth();
    FX_INT32 height = bitMatrix->GetHeight();
    CBC_CommonBitArray* firstRowBitArray = FX_NEW CBC_CommonBitArray(width);
    CBC_CommonBitArray* secondRowBitArray = FX_NEW CBC_CommonBitArray(width);
    CBC_CommonBitArray* tmpBitArray = FX_NEW CBC_CommonBitArray(width);
    for (FX_INT32 y = 0; y < (height + 1) >> 1; y++) {
        CBC_CommonBitArray* temp = bitMatrix->GetRow(height - 1 - y, secondRowBitArray);
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
CBC_CommonBitArray* CBC_Detector::mirror(CBC_CommonBitArray* input, CBC_CommonBitArray* result)
{
    CBC_CommonBitArray* array = FX_NEW CBC_CommonBitArray(result->GetSize());
    array->Clear();
    FX_INT32 size = input->GetSize();
    for (FX_INT32 i = 0; i < size; i++) {
        if (input->Get(i)) {
            array->Set(size - 1 - i);
        }
    }
    return array;
}
CFX_PtrArray* CBC_Detector::detect(FX_BOOL multiple, CBC_CommonBitMatrix* bitMatrix)
{
    CFX_PtrArray* barcodeCoordinates = FX_NEW CFX_PtrArray;
    FX_INT32 row = 0;
    FX_INT32 column = 0;
    FX_BOOL foundBarcodeInRow = FALSE;
    while (row < bitMatrix->GetHeight()) {
        CFX_PtrArray* vertices = findVertices(bitMatrix, row, column);
        if (vertices->GetAt(0) == NULL && vertices->GetAt(3) == NULL) {
            if (!foundBarcodeInRow) {
                if (vertices) {
                    delete(vertices);
                }
                break;
            }
            foundBarcodeInRow = FALSE;
            column = 0;
            for (FX_INT32 i = 0; i < barcodeCoordinates->GetSize(); i++) {
                CFX_PtrArray* barcodeCoordinate = (CFX_PtrArray*)barcodeCoordinates->GetAt(i);
                if (barcodeCoordinate->GetAt(1) != NULL) {
                    row = row > ((CBC_ResultPoint*)barcodeCoordinate->GetAt(1))->GetY();
                }
                if (barcodeCoordinate->GetAt(3) != NULL) {
                    row = row > ((CBC_ResultPoint*)barcodeCoordinate->GetAt(3))->GetY();
                }
            }
            row += ROW_STEP;
            if (vertices) {
                delete(vertices);
            }
            continue;
        }
        foundBarcodeInRow = TRUE;
        barcodeCoordinates->Add(vertices);
        if (!multiple) {
            break;
        }
        if (vertices->GetAt(2) != NULL) {
            column = (FX_INT32) ((CBC_ResultPoint*)vertices->GetAt(2))->GetX();
            row = (FX_INT32) ((CBC_ResultPoint*)vertices->GetAt(2))->GetY();
        } else {
            column = (FX_INT32) ((CBC_ResultPoint*)vertices->GetAt(4))->GetX();
            row = (FX_INT32) ((CBC_ResultPoint*)vertices->GetAt(4))->GetY();
        }
    }
    return barcodeCoordinates;
}
CFX_PtrArray* CBC_Detector::findVertices(CBC_CommonBitMatrix* matrix, FX_INT32 startRow, FX_INT32 startColumn)
{
    FX_INT32 height = matrix->GetHeight();
    FX_INT32 width = matrix->GetWidth();
    CFX_PtrArray* result = FX_NEW CFX_PtrArray;
    result->SetSize(8);
    CFX_PtrArray* startptr =  findRowsWithPattern(matrix, height, width, startRow, startColumn, START_PATTERN, sizeof(START_PATTERN) / sizeof(START_PATTERN[0]));
    copyToResult(result, startptr, INDEXES_START_PATTERN, sizeof(INDEXES_START_PATTERN) / sizeof(INDEXES_START_PATTERN[0]));
    startptr->RemoveAll();
    delete startptr;
    if (result->GetAt(4) != NULL) {
        startColumn = (FX_INT32) ((CBC_ResultPoint*)result->GetAt(4))->GetX();
        startRow = (FX_INT32) ((CBC_ResultPoint*)result->GetAt(4))->GetY();
    }
    CFX_PtrArray*  stopptr = findRowsWithPattern(matrix, height, width, startRow, startColumn, STOP_PATTERN, sizeof(STOP_PATTERN) / sizeof(STOP_PATTERN[0]));
    copyToResult(result, stopptr, INDEXES_STOP_PATTERN, sizeof(INDEXES_STOP_PATTERN) / sizeof(INDEXES_STOP_PATTERN[0]));
    stopptr->RemoveAll();
    delete stopptr;
    return result;
}
void CBC_Detector::copyToResult(CFX_PtrArray *result, CFX_PtrArray* tmpResult, FX_INT32* destinationIndexes, FX_INT32 destinationLength)
{
    for (FX_INT32 i = 0; i < destinationLength; i++) {
        result->SetAt(destinationIndexes[i], tmpResult->GetAt(i));
    }
}
CFX_PtrArray* CBC_Detector::findRowsWithPattern(CBC_CommonBitMatrix* matrix, FX_INT32 height, FX_INT32 width, FX_INT32 startRow, FX_INT32 startColumn, FX_INT32* pattern, FX_INT32 patternLength)
{
    CFX_PtrArray* result = FX_NEW CFX_PtrArray;
    result->SetSize(4);
    FX_BOOL found = FALSE;
    CFX_Int32Array counters;
    counters.SetSize(patternLength);
    for (; startRow < height; startRow += ROW_STEP) {
        CFX_Int32Array* loc = findGuardPattern(matrix, startColumn, startRow, width, FALSE, pattern, patternLength, counters);
        if (loc != NULL) {
            while (startRow > 0) {
                CFX_Int32Array* previousRowLoc = findGuardPattern(matrix, startColumn, --startRow, width, FALSE, pattern, patternLength, counters);
                if (previousRowLoc != NULL) {
                    delete loc;
                    loc = previousRowLoc;
                } else {
                    startRow++;
                    break;
                }
            }
            result->SetAt(0, FX_NEW CBC_ResultPoint((FX_FLOAT)loc->GetAt(0), (FX_FLOAT)startRow));
            result->SetAt(1, FX_NEW CBC_ResultPoint((FX_FLOAT)loc->GetAt(1), (FX_FLOAT)startRow));
            found = TRUE;
            delete loc;
            break;
        }
    }
    FX_INT32 stopRow = startRow + 1;
    if (found) {
        FX_INT32 skippedRowCount = 0;
        CFX_Int32Array previousRowLoc;
        previousRowLoc.Add((FX_INT32)((CBC_ResultPoint*)result->GetAt(0))->GetX());
        previousRowLoc.Add((FX_INT32)((CBC_ResultPoint*)result->GetAt(1))->GetX());
        for (; stopRow < height; stopRow++) {
            CFX_Int32Array* loc = findGuardPattern(matrix, previousRowLoc[0], stopRow, width, FALSE, pattern, patternLength, counters);
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
        result->SetAt(2, FX_NEW CBC_ResultPoint((FX_FLOAT)previousRowLoc.GetAt(0), (FX_FLOAT)stopRow));
        result->SetAt(3, FX_NEW CBC_ResultPoint((FX_FLOAT)previousRowLoc.GetAt(1), (FX_FLOAT)stopRow));
    }
    if (stopRow - startRow < BARCODE_MIN_HEIGHT) {
        for (FX_INT32 i = 0; i < result->GetSize(); i++) {
            result->SetAt(i, NULL);
        }
    }
    return result;
}
CFX_Int32Array* CBC_Detector::findGuardPattern(CBC_CommonBitMatrix* matrix, FX_INT32 column, FX_INT32 row, FX_INT32 width, FX_BOOL whiteFirst, FX_INT32* pattern, FX_INT32 patternLength, CFX_Int32Array &counters)
{
    for (FX_INT32 i = 0; i < counters.GetSize(); i++) {
        counters.SetAt(i, 0);
    }
    FX_BOOL isWhite = whiteFirst;
    FX_INT32 patternStart = column;
    FX_INT32 pixelDrift = 0;
    CFX_Int32Array* intarray = FX_NEW CFX_Int32Array;
    while (matrix->Get(patternStart, row) && patternStart > 0 && pixelDrift++ < MAX_PIXEL_DRIFT) {
        patternStart--;
    }
    FX_INT32 x = patternStart;
    FX_INT32 counterPosition = 0;
    for (; x < width; x++) {
        FX_BOOL pixel = matrix->Get(x, row);
        if (pixel ^ isWhite) {
            counters[counterPosition]++;
        } else {
            if (counterPosition == patternLength - 1) {
                if (patternMatchVariance(counters, pattern, MAX_INDIVIDUAL_VARIANCE) < MAX_AVG_VARIANCE) {
                    intarray->Add(patternStart);
                    intarray->Add(x);
                    return intarray;
                }
                patternStart += counters[0] + counters[1];
                for (FX_INT32 l = 2, k = 0; l < patternLength; l++, k++) {
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
        if (patternMatchVariance(counters, pattern, MAX_INDIVIDUAL_VARIANCE) < MAX_AVG_VARIANCE) {
            intarray->Add(patternStart);
            intarray->Add(x - 1);
            return intarray;
        }
    }
    delete intarray;
    return NULL;
}
FX_INT32 CBC_Detector::patternMatchVariance(CFX_Int32Array &counters, FX_INT32* pattern, FX_INT32 maxIndividualVariance)
{
    FX_INT32 numCounters = counters.GetSize();
    FX_INT32 total = 0;
    FX_INT32 patternLength = 0;
    for (FX_INT32 i = 0; i < numCounters; i++) {
        total += counters[i];
        patternLength += pattern[i];
    }
    if (total < patternLength) {
        return INTERGER_MAX;
    }
    FX_INT32 unitBarWidth = (total << INTEGER_MATH_SHIFT) / patternLength;
    maxIndividualVariance = (maxIndividualVariance * unitBarWidth) >> INTEGER_MATH_SHIFT;
    FX_INT32 totalVariance = 0;
    for (FX_INT32 x = 0; x < numCounters; x++) {
        FX_INT32 counter = counters[x] << INTEGER_MATH_SHIFT;
        FX_INT32 scaledPattern = pattern[x] * unitBarWidth;
        FX_INT32 variance = counter > scaledPattern ? counter - scaledPattern : scaledPattern - counter;
        if (variance > maxIndividualVariance) {
            return INTERGER_MAX;
        }
        totalVariance += variance;
    }
    return totalVariance / total;
}

