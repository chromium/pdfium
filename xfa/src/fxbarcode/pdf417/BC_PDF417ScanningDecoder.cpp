// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2013 ZXing authors
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

#include "../barcode.h"
#include "../BC_DecoderResult.h"
#include "../BC_ResultPoint.h"
#include "../common/BC_CommonBitMatrix.h"
#include "BC_PDF417Codeword.h"
#include "BC_PDF417Common.h"
#include "BC_PDF417BarcodeValue.h"
#include "BC_PDF417BarcodeMetadata.h"
#include "BC_PDF417BoundingBox.h"
#include "BC_PDF417DetectionResultColumn.h"
#include "BC_PDF417DetectionResultRowIndicatorColumn.h"
#include "BC_PDF417DetectionResult.h"
#include "BC_PDF417DecodedBitStreamParser.h"
#include "BC_PDF417CodewordDecoder.h"
#include "BC_PDF417DecodedBitStreamParser.h"
#include "BC_PDF417ECModulusPoly.h"
#include "BC_PDF417ECModulusGF.h"
#include "BC_PDF417ECErrorCorrection.h"
#include "BC_PDF417DecodedBitStreamParser.h"
#include "BC_PDF417ScanningDecoder.h"
FX_INT32 CBC_PDF417ScanningDecoder::CODEWORD_SKEW_SIZE = 2;
FX_INT32 CBC_PDF417ScanningDecoder::MAX_ERRORS = 3;
FX_INT32 CBC_PDF417ScanningDecoder::MAX_EC_CODEWORDS = 512;
CBC_PDF417ECErrorCorrection* CBC_PDF417ScanningDecoder::errorCorrection = NULL;
CBC_PDF417ScanningDecoder::CBC_PDF417ScanningDecoder()
{
}
CBC_PDF417ScanningDecoder::~CBC_PDF417ScanningDecoder()
{
}
void CBC_PDF417ScanningDecoder::Initialize()
{
    errorCorrection = FX_NEW CBC_PDF417ECErrorCorrection;
}
void CBC_PDF417ScanningDecoder::Finalize()
{
    delete errorCorrection;
}
CBC_CommonDecoderResult* CBC_PDF417ScanningDecoder::decode(CBC_CommonBitMatrix* image, CBC_ResultPoint* imageTopLeft, CBC_ResultPoint* imageBottomLeft, CBC_ResultPoint* imageTopRight,
        CBC_ResultPoint* imageBottomRight, FX_INT32 minCodewordWidth, FX_INT32 maxCodewordWidth, FX_INT32 &e)
{
    CBC_BoundingBox* boundingBox = FX_NEW CBC_BoundingBox(image, imageTopLeft, imageBottomLeft, imageTopRight, imageBottomRight, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    CBC_DetectionResultRowIndicatorColumn* leftRowIndicatorColumn = NULL;
    CBC_DetectionResultRowIndicatorColumn* rightRowIndicatorColumn = NULL;
    CBC_DetectionResult* detectionResult = NULL;
    for (FX_INT32 i = 0; i < 2; i++) {
        if (imageTopLeft != NULL) {
            leftRowIndicatorColumn = getRowIndicatorColumn(image, boundingBox, *imageTopLeft, TRUE, minCodewordWidth, maxCodewordWidth);
        }
        if (imageTopRight != NULL) {
            rightRowIndicatorColumn = getRowIndicatorColumn(image, boundingBox, *imageTopRight, FALSE, minCodewordWidth, maxCodewordWidth);
        }
        detectionResult = merge(leftRowIndicatorColumn, rightRowIndicatorColumn, e);
        if (e != BCExceptionNO) {
            e = BCExceptiontNotFoundInstance;
            delete leftRowIndicatorColumn;
            delete rightRowIndicatorColumn;
            delete boundingBox;
            return NULL;
        }
        if (i == 0 && (detectionResult->getBoundingBox()->getMinY() < boundingBox->getMinY() || detectionResult->getBoundingBox()->getMaxY() > boundingBox->getMaxY())) {
            delete boundingBox;
            boundingBox = detectionResult->getBoundingBox();
        } else {
            detectionResult->setBoundingBox(boundingBox);
            break;
        }
    }
    FX_INT32 maxBarcodeColumn = detectionResult->getBarcodeColumnCount() + 1;
    detectionResult->setDetectionResultColumn(0, leftRowIndicatorColumn);
    detectionResult->setDetectionResultColumn(maxBarcodeColumn, rightRowIndicatorColumn);
    FX_BOOL leftToRight = leftRowIndicatorColumn != NULL;
    for (FX_INT32 barcodeColumnCount = 1; barcodeColumnCount <= maxBarcodeColumn; barcodeColumnCount++) {
        FX_INT32 barcodeColumn = leftToRight ? barcodeColumnCount : maxBarcodeColumn - barcodeColumnCount;
        if (detectionResult->getDetectionResultColumn(barcodeColumn) != NULL) {
            continue;
        }
        CBC_DetectionResultColumn* detectionResultColumn = NULL;
        if (barcodeColumn == 0 || barcodeColumn == maxBarcodeColumn) {
            detectionResultColumn = FX_NEW CBC_DetectionResultRowIndicatorColumn(boundingBox, barcodeColumn == 0);
        } else {
            detectionResultColumn = FX_NEW CBC_DetectionResultColumn(boundingBox);
        }
        detectionResult->setDetectionResultColumn(barcodeColumn, detectionResultColumn);
        FX_INT32 startColumn = -1;
        FX_INT32 previousStartColumn = startColumn;
        for (FX_INT32 imageRow = boundingBox->getMinY(); imageRow <= boundingBox->getMaxY(); imageRow++) {
            startColumn = getStartColumn(detectionResult, barcodeColumn, imageRow, leftToRight);
            if (startColumn < 0 || startColumn > boundingBox->getMaxX()) {
                if (previousStartColumn == -1) {
                    continue;
                }
                startColumn = previousStartColumn;
            }
            CBC_Codeword* codeword = detectCodeword(image, boundingBox->getMinX(), boundingBox->getMaxX(), leftToRight, startColumn, imageRow, minCodewordWidth, maxCodewordWidth);
            if (codeword != NULL) {
                detectionResultColumn->setCodeword(imageRow, codeword);
                previousStartColumn = startColumn;
                minCodewordWidth = minCodewordWidth < codeword->getWidth() ? minCodewordWidth : codeword->getWidth();
                maxCodewordWidth = maxCodewordWidth > codeword->getWidth() ? maxCodewordWidth : codeword->getWidth();
            }
        }
    }
    CBC_CommonDecoderResult* decoderresult = createDecoderResult(detectionResult, e);
    if (e != BCExceptionNO) {
        delete detectionResult;
        return NULL;
    }
    return decoderresult;
}
CFX_ByteString CBC_PDF417ScanningDecoder::toString(CFX_PtrArray* barcodeMatrix)
{
    CFX_ByteString result;
    for (FX_INT32 row = 0; row < barcodeMatrix->GetSize(); row++) {
        result += row;
        FX_INT32 l = 0;
        for (; l < ((CFX_PtrArray*)barcodeMatrix->GetAt(row))->GetSize(); l++) {
            CBC_BarcodeValue* barcodeValue = (CBC_BarcodeValue*)((CFX_PtrArray*)barcodeMatrix->GetAt(row))->GetAt(l);
            if (barcodeValue->getValue()->GetSize() == 0) {
                result +=  "";
            } else {
                result += barcodeValue->getValue()->GetAt(0);
                result += barcodeValue->getConfidence(barcodeValue->getValue()->GetAt(0));
            }
        }
    }
    return result;
}
CBC_DetectionResult* CBC_PDF417ScanningDecoder::merge(CBC_DetectionResultRowIndicatorColumn* leftRowIndicatorColumn, CBC_DetectionResultRowIndicatorColumn* rightRowIndicatorColumn, FX_INT32 &e)
{
    if (leftRowIndicatorColumn == NULL && rightRowIndicatorColumn == NULL) {
        e = BCExceptionIllegalArgument;
        return NULL;
    }
    CBC_BarcodeMetadata* barcodeMetadata = getBarcodeMetadata(leftRowIndicatorColumn, rightRowIndicatorColumn);
    if (barcodeMetadata == NULL) {
        e = BCExceptionCannotMetadata;
        return NULL;
    }
    CBC_BoundingBox* leftboundingBox = adjustBoundingBox(leftRowIndicatorColumn, e);
    if (e != BCExceptionNO) {
        delete barcodeMetadata;
        return NULL;
    }
    CBC_BoundingBox* rightboundingBox = adjustBoundingBox(rightRowIndicatorColumn, e);
    if (e != BCExceptionNO) {
        delete barcodeMetadata;
        return NULL;
    }
    CBC_BoundingBox* boundingBox = CBC_BoundingBox::merge(leftboundingBox, rightboundingBox, e);
    if (e != BCExceptionNO) {
        delete barcodeMetadata;
        return NULL;
    }
    CBC_DetectionResult* detectionresult = FX_NEW CBC_DetectionResult(barcodeMetadata, boundingBox);
    return detectionresult;
}
CBC_BoundingBox* CBC_PDF417ScanningDecoder::adjustBoundingBox(CBC_DetectionResultRowIndicatorColumn* rowIndicatorColumn, FX_INT32 &e)
{
    if (rowIndicatorColumn == NULL) {
        return NULL;
    }
    CFX_Int32Array* rowHeights = rowIndicatorColumn->getRowHeights(e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    FX_INT32 maxRowHeight = getMax(*rowHeights);
    FX_INT32 missingStartRows = 0;
    for (FX_INT32 i = 0; i < rowHeights->GetSize(); i++) {
        FX_INT32 rowHeight = rowHeights->GetAt(i);
        missingStartRows += maxRowHeight - rowHeight;
        if (rowHeight > 0) {
            break;
        }
    }
    CFX_PtrArray* codewords = rowIndicatorColumn->getCodewords();
    for (FX_INT32 row = 0; missingStartRows > 0 && codewords->GetAt(row) == NULL; row++) {
        missingStartRows--;
    }
    FX_INT32 missingEndRows = 0;
    for (FX_INT32 row1 = rowHeights->GetSize() - 1; row1 >= 0; row1--) {
        missingEndRows += maxRowHeight - rowHeights->GetAt(row1);
        if (rowHeights->GetAt(row1) > 0) {
            break;
        }
    }
    for (FX_INT32 row2 = codewords->GetSize() - 1; missingEndRows > 0 && codewords->GetAt(row2) == NULL; row2--) {
        missingEndRows--;
    }
    CBC_BoundingBox* boundingBox = rowIndicatorColumn->getBoundingBox()->addMissingRows(missingStartRows, missingEndRows, rowIndicatorColumn->isLeft(), e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return boundingBox;
}
FX_INT32 CBC_PDF417ScanningDecoder::getMax(CFX_Int32Array& values)
{
    FX_INT32 maxValue = -1;
    for (FX_INT32 i = 0; i < values.GetSize(); i++) {
        FX_INT32 value = values.GetAt(i);
        maxValue = maxValue > value ? maxValue : value;
    }
    return maxValue;
}
CBC_BarcodeMetadata* CBC_PDF417ScanningDecoder::getBarcodeMetadata(CBC_DetectionResultRowIndicatorColumn* leftRowIndicatorColumn,
        CBC_DetectionResultRowIndicatorColumn* rightRowIndicatorColumn)
{
    CBC_BarcodeMetadata* leftBarcodeMetadata = NULL;
    CBC_BarcodeMetadata* rightBarcodeMetadata = NULL;
    if (leftRowIndicatorColumn == NULL || (leftBarcodeMetadata = leftRowIndicatorColumn->getBarcodeMetadata()) == NULL) {
        return rightRowIndicatorColumn == NULL ? NULL : rightRowIndicatorColumn->getBarcodeMetadata();
    }
    if (rightRowIndicatorColumn == NULL || (rightBarcodeMetadata = rightRowIndicatorColumn->getBarcodeMetadata()) == NULL) {
        return leftRowIndicatorColumn == NULL ? NULL : leftRowIndicatorColumn->getBarcodeMetadata();
    }
    if (leftBarcodeMetadata->getColumnCount() != rightBarcodeMetadata->getColumnCount() &&
            leftBarcodeMetadata->getErrorCorrectionLevel() != rightBarcodeMetadata->getErrorCorrectionLevel() &&
            leftBarcodeMetadata->getRowCount() != rightBarcodeMetadata->getRowCount()) {
        delete leftBarcodeMetadata;
        delete rightBarcodeMetadata;
        return NULL;
    }
    delete rightBarcodeMetadata;
    return leftBarcodeMetadata;
}
CBC_DetectionResultRowIndicatorColumn* CBC_PDF417ScanningDecoder::getRowIndicatorColumn(CBC_CommonBitMatrix* image, CBC_BoundingBox* boundingBox, CBC_ResultPoint startPoint,
        FX_BOOL leftToRight, FX_INT32 minCodewordWidth, FX_INT32 maxCodewordWidth)
{
    CBC_DetectionResultRowIndicatorColumn* rowIndicatorColumn = FX_NEW CBC_DetectionResultRowIndicatorColumn(boundingBox, leftToRight);
    for (FX_INT32 i = 0; i < 2; i++) {
        FX_INT32 increment = i == 0 ? 1 : -1;
        FX_INT32 startColumn = (FX_INT32) startPoint.GetX();
        for (FX_INT32 imageRow = (FX_INT32) startPoint.GetY(); imageRow <= boundingBox->getMaxY() && imageRow >= boundingBox->getMinY(); imageRow += increment) {
            CBC_Codeword* codeword = detectCodeword(image, 0, image->GetWidth(), leftToRight, startColumn, imageRow, minCodewordWidth, maxCodewordWidth);
            if (codeword != NULL) {
                rowIndicatorColumn->setCodeword(imageRow, codeword);
                if (leftToRight) {
                    startColumn = codeword->getStartX();
                } else {
                    startColumn = codeword->getEndX();
                }
            }
        }
    }
    return rowIndicatorColumn;
}
void CBC_PDF417ScanningDecoder::adjustCodewordCount(CBC_DetectionResult* detectionResult, CFX_PtrArray* barcodeMatrix, FX_INT32 &e)
{
    CFX_Int32Array* numberOfCodewords = ((CBC_BarcodeValue*)((CFX_PtrArray*)barcodeMatrix->GetAt(0))->GetAt(1))->getValue();
    FX_INT32 calculatedNumberOfCodewords = detectionResult->getBarcodeColumnCount() * detectionResult->getBarcodeRowCount() - getNumberOfECCodeWords(detectionResult->getBarcodeECLevel());
    if (numberOfCodewords->GetSize() == 0) {
        if (calculatedNumberOfCodewords < 1 || calculatedNumberOfCodewords > CBC_PDF417Common::MAX_CODEWORDS_IN_BARCODE) {
            e = BCExceptiontNotFoundInstance;
            delete numberOfCodewords;
            BC_EXCEPTION_CHECK_ReturnVoid(e);
        }
        ((CBC_BarcodeValue*)((CFX_PtrArray*)barcodeMatrix->GetAt(0))->GetAt(1))->setValue(calculatedNumberOfCodewords);
    } else if (numberOfCodewords->GetAt(0) != calculatedNumberOfCodewords) {
        ((CBC_BarcodeValue*)((CFX_PtrArray*)barcodeMatrix->GetAt(0))->GetAt(1))->setValue(calculatedNumberOfCodewords);
    }
    delete numberOfCodewords;
}
CBC_CommonDecoderResult* CBC_PDF417ScanningDecoder::createDecoderResult(CBC_DetectionResult* detectionResult, FX_INT32 &e)
{
    CFX_PtrArray* barcodeMatrix = createBarcodeMatrix(detectionResult);
    adjustCodewordCount(detectionResult, barcodeMatrix, e);
    if (e != BCExceptionNO) {
        for (FX_INT32 i = 0; i < barcodeMatrix->GetSize(); i++) {
            CFX_PtrArray* temp = (CFX_PtrArray*)barcodeMatrix->GetAt(i);
            for (FX_INT32 j = 0; j < temp->GetSize(); j++) {
                delete (CBC_BarcodeValue*)temp->GetAt(j);
            }
            temp->RemoveAll();
            delete temp;
        }
        barcodeMatrix->RemoveAll();
        delete barcodeMatrix;
        return NULL;
    }
    CFX_Int32Array erasures;
    CFX_Int32Array codewords;
    codewords.SetSize(detectionResult->getBarcodeRowCount() * detectionResult->getBarcodeColumnCount());
    CFX_PtrArray ambiguousIndexValuesList;
    CFX_Int32Array ambiguousIndexesList;
    for (FX_INT32 row = 0; row < detectionResult->getBarcodeRowCount(); row++) {
        for (FX_INT32 l = 0; l < detectionResult->getBarcodeColumnCount(); l++) {
            CFX_Int32Array* values = ((CBC_BarcodeValue*)((CFX_PtrArray*)barcodeMatrix->GetAt(row))->GetAt(l + 1))->getValue();
            FX_INT32 codewordIndex = row * detectionResult->getBarcodeColumnCount() + l;
            if (values->GetSize() == 0) {
                erasures.Add(codewordIndex);
            } else if (values->GetSize() == 1) {
                codewords[codewordIndex] = values->GetAt(0);
            } else {
                ambiguousIndexesList.Add(codewordIndex);
                ambiguousIndexValuesList.Add(values);
            }
        }
    }
    CFX_PtrArray ambiguousIndexValues;
    ambiguousIndexValues.SetSize(ambiguousIndexValuesList.GetSize());
    for (FX_INT32 i = 0; i < ambiguousIndexValues.GetSize(); i++) {
        ambiguousIndexValues.SetAt(i, ambiguousIndexValuesList.GetAt(i));
    }
    for (FX_INT32 l = 0; l < barcodeMatrix->GetSize(); l++) {
        CFX_PtrArray* temp = (CFX_PtrArray*)barcodeMatrix->GetAt(l);
        for (FX_INT32 j = 0; j < temp->GetSize(); j++) {
            delete (CBC_BarcodeValue*)temp->GetAt(j);
        }
        temp->RemoveAll();
        delete temp;
    }
    barcodeMatrix->RemoveAll();
    delete barcodeMatrix;
    CBC_CommonDecoderResult* decoderResult = createDecoderResultFromAmbiguousValues(detectionResult->getBarcodeECLevel(), codewords, erasures, ambiguousIndexesList, ambiguousIndexValues, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return decoderResult;
}
CBC_CommonDecoderResult* CBC_PDF417ScanningDecoder::createDecoderResultFromAmbiguousValues(FX_INT32 ecLevel, CFX_Int32Array &codewords, CFX_Int32Array &erasureArray, CFX_Int32Array &ambiguousIndexes,
        CFX_PtrArray& ambiguousIndexValues, FX_INT32 &e)
{
    CFX_Int32Array ambiguousIndexCount;
    ambiguousIndexCount.SetSize(ambiguousIndexes.GetSize());
    FX_INT32 tries = 100;
    while (tries-- > 0) {
        for (FX_INT32 l = 0; l < ambiguousIndexCount.GetSize(); l++) {
            codewords[ambiguousIndexes[l]] = ((CFX_Int32Array*)ambiguousIndexValues.GetAt(l))->GetAt(ambiguousIndexCount[l]);
        }
        CBC_CommonDecoderResult* decoderResult = decodeCodewords(codewords, ecLevel, erasureArray, e);
        if (e != BCExceptionNO) {
            e = BCExceptionNO;
            continue;
        } else {
            return decoderResult;
        }
        if (ambiguousIndexCount.GetSize() == 0) {
            e = BCExceptionChecksumInstance;
            return NULL;
        }
        for (FX_INT32 i = 0; i < ambiguousIndexCount.GetSize(); i++) {
            if (ambiguousIndexCount[i] < ((CFX_Int32Array*)(ambiguousIndexValues.GetAt(i)))->GetSize() - 1) {
                ambiguousIndexCount[i]++;
                break;
            } else {
                ambiguousIndexCount[i] = 0;
                if (i == ambiguousIndexCount.GetSize() - 1) {
                    e = BCExceptionChecksumInstance;
                    return NULL;
                }
            }
        }
    }
    e = BCExceptionChecksumInstance;
    return NULL;
}
CFX_PtrArray* CBC_PDF417ScanningDecoder::createBarcodeMatrix(CBC_DetectionResult* detectionResult)
{
    CFX_PtrArray* barcodeMatrix = FX_NEW CFX_PtrArray;
    barcodeMatrix->SetSize(detectionResult->getBarcodeRowCount());
    CFX_PtrArray* temp = NULL;
    FX_INT32 colume = 0;
    for (FX_INT32 row = 0; row < barcodeMatrix->GetSize(); row++) {
        temp = FX_NEW CFX_PtrArray;
        temp->SetSize(detectionResult->getBarcodeColumnCount() + 2);
        for (colume = 0; colume < detectionResult->getBarcodeColumnCount() + 2; colume++) {
            temp->SetAt(colume, FX_NEW CBC_BarcodeValue());
        }
        barcodeMatrix->SetAt(row, temp);
    }
    colume = -1;
    for (FX_INT32 i = 0; i < detectionResult->getDetectionResultColumns().GetSize(); i++) {
        CBC_DetectionResultColumn* detectionResultColumn = (CBC_DetectionResultColumn*) detectionResult->getDetectionResultColumns().GetAt(i);
        colume++;
        if (detectionResultColumn == NULL) {
            continue;
        }
        CFX_PtrArray* temp = detectionResultColumn->getCodewords();
        for (FX_INT32 l = 0; l < temp->GetSize(); l++) {
            CBC_Codeword* codeword = (CBC_Codeword*) temp->GetAt(l);
            if (codeword == NULL || codeword->getRowNumber() == -1) {
                continue;
            }
            ((CBC_BarcodeValue*)((CFX_PtrArray*)barcodeMatrix->GetAt(codeword->getRowNumber()))->GetAt(colume))->setValue(codeword->getValue());
        }
    }
    return barcodeMatrix;
}
FX_BOOL CBC_PDF417ScanningDecoder::isValidBarcodeColumn(CBC_DetectionResult* detectionResult, FX_INT32 barcodeColumn)
{
    return barcodeColumn >= 0 && barcodeColumn <= detectionResult->getBarcodeColumnCount() + 1;
}
FX_INT32 CBC_PDF417ScanningDecoder::getStartColumn(CBC_DetectionResult* detectionResult, FX_INT32 barcodeColumn, FX_INT32 imageRow, FX_BOOL leftToRight)
{
    FX_INT32 offset = leftToRight ? 1 : -1;
    CBC_Codeword* codeword = NULL;
    if (isValidBarcodeColumn(detectionResult, barcodeColumn - offset)) {
        codeword = detectionResult->getDetectionResultColumn(barcodeColumn - offset)->getCodeword(imageRow);
    }
    if (codeword != NULL) {
        return leftToRight ? codeword->getEndX() : codeword->getStartX();
    }
    codeword = detectionResult->getDetectionResultColumn(barcodeColumn)->getCodewordNearby(imageRow);
    if (codeword != NULL) {
        return leftToRight ? codeword->getStartX() : codeword->getEndX();
    }
    if (isValidBarcodeColumn(detectionResult, barcodeColumn - offset)) {
        codeword = detectionResult->getDetectionResultColumn(barcodeColumn - offset)->getCodewordNearby(imageRow);
    }
    if (codeword != NULL) {
        return leftToRight ? codeword->getEndX() : codeword->getStartX();
    }
    FX_INT32 skippedColumns = 0;
    while (isValidBarcodeColumn(detectionResult, barcodeColumn - offset)) {
        barcodeColumn -= offset;
        for (FX_INT32 i = 0; i < detectionResult->getDetectionResultColumn(barcodeColumn)->getCodewords()->GetSize(); i++) {
            CBC_Codeword* previousRowCodeword = (CBC_Codeword*) detectionResult->getDetectionResultColumn(barcodeColumn)->getCodewords()->GetAt(i);
            if (previousRowCodeword != NULL) {
                return (leftToRight ? previousRowCodeword->getEndX() : previousRowCodeword->getStartX()) +
                       offset * skippedColumns * (previousRowCodeword->getEndX() - previousRowCodeword->getStartX());
            }
        }
        skippedColumns++;
    }
    return leftToRight ? detectionResult->getBoundingBox()->getMinX() : detectionResult->getBoundingBox()->getMaxX();
}
CBC_Codeword* CBC_PDF417ScanningDecoder::detectCodeword(CBC_CommonBitMatrix* image, FX_INT32 minColumn, FX_INT32 maxColumn, FX_BOOL leftToRight, FX_INT32 startColumn,
        FX_INT32 imageRow, FX_INT32 minCodewordWidth, FX_INT32 maxCodewordWidth)
{
    startColumn = adjustCodewordStartColumn(image, minColumn, maxColumn, leftToRight, startColumn, imageRow);
    CFX_Int32Array* moduleBitCount = getModuleBitCount(image, minColumn, maxColumn, leftToRight, startColumn, imageRow);
    if (moduleBitCount == NULL) {
        return NULL;
    }
    FX_INT32 endColumn;
    FX_INT32 codewordBitCount = CBC_PDF417Common::getBitCountSum(*moduleBitCount);
    if (leftToRight) {
        endColumn = startColumn + codewordBitCount;
    } else {
        for (FX_INT32 i = 0; i < moduleBitCount->GetSize() >> 1; i++) {
            FX_INT32 tmpCount = moduleBitCount->GetAt(i);
            moduleBitCount->SetAt(i, moduleBitCount->GetAt(moduleBitCount->GetSize() - 1 - i));
            moduleBitCount->SetAt(moduleBitCount->GetSize() - 1 - i, tmpCount);
        }
        endColumn = startColumn;
        startColumn = endColumn - codewordBitCount;
    }
    FX_INT32 decodedValue = CBC_PDF417CodewordDecoder::getDecodedValue(*moduleBitCount);
    FX_INT32 codeword = CBC_PDF417Common::getCodeword(decodedValue);
    delete moduleBitCount;
    if (codeword == -1) {
        return NULL;
    }
    return FX_NEW CBC_Codeword(startColumn, endColumn, getCodewordBucketNumber(decodedValue), codeword);
}
CFX_Int32Array* CBC_PDF417ScanningDecoder::getModuleBitCount(CBC_CommonBitMatrix* image, FX_INT32 minColumn, FX_INT32 maxColumn, FX_BOOL leftToRight, FX_INT32 startColumn, FX_INT32 imageRow)
{
    FX_INT32 imageColumn = startColumn;
    CFX_Int32Array* moduleBitCount = FX_NEW CFX_Int32Array;
    moduleBitCount->SetSize(8);
    FX_INT32 moduleNumber = 0;
    FX_INT32 increment = leftToRight ? 1 : -1;
    FX_BOOL previousPixelValue = leftToRight;
    while (((leftToRight && imageColumn < maxColumn) || (!leftToRight && imageColumn >= minColumn)) && moduleNumber < moduleBitCount->GetSize()) {
        if (image->Get(imageColumn, imageRow) == previousPixelValue) {
            moduleBitCount->SetAt(moduleNumber, moduleBitCount->GetAt(moduleNumber) + 1);
            imageColumn += increment;
        } else {
            moduleNumber++;
            previousPixelValue = !previousPixelValue;
        }
    }
    if (moduleNumber == moduleBitCount->GetSize() || (((leftToRight && imageColumn == maxColumn) || (!leftToRight && imageColumn == minColumn)) && moduleNumber == moduleBitCount->GetSize() - 1)) {
        return moduleBitCount;
    }
    delete moduleBitCount;
    return NULL;
}
FX_INT32 CBC_PDF417ScanningDecoder::getNumberOfECCodeWords(FX_INT32 barcodeECLevel)
{
    return 2 << barcodeECLevel;
}
FX_INT32 CBC_PDF417ScanningDecoder::adjustCodewordStartColumn(CBC_CommonBitMatrix* image, FX_INT32 minColumn, FX_INT32 maxColumn, FX_BOOL leftToRight, FX_INT32 codewordStartColumn, FX_INT32 imageRow)
{
    FX_INT32 correctedStartColumn = codewordStartColumn;
    FX_INT32 increment = leftToRight ? -1 : 1;
    for (FX_INT32 i = 0; i < 2; i++) {
        FX_BOOL l = image->Get(correctedStartColumn, imageRow);
        while (((leftToRight && correctedStartColumn >= minColumn) || (!leftToRight && correctedStartColumn < maxColumn)) && leftToRight == image->Get(correctedStartColumn, imageRow)) {
            if (abs(codewordStartColumn - correctedStartColumn) > CODEWORD_SKEW_SIZE) {
                return codewordStartColumn;
            }
            correctedStartColumn += increment;
        }
        increment = -increment;
        leftToRight = !leftToRight;
    }
    return correctedStartColumn;
}
FX_BOOL CBC_PDF417ScanningDecoder::checkCodewordSkew(FX_INT32 codewordSize, FX_INT32 minCodewordWidth, FX_INT32 maxCodewordWidth)
{
    return minCodewordWidth - CODEWORD_SKEW_SIZE <= codewordSize && codewordSize <= maxCodewordWidth + CODEWORD_SKEW_SIZE;
}
CBC_CommonDecoderResult* CBC_PDF417ScanningDecoder::decodeCodewords(CFX_Int32Array &codewords, FX_INT32 ecLevel, CFX_Int32Array &erasures, FX_INT32 &e)
{
    if (codewords.GetSize() == 0) {
        e = BCExceptionFormatInstance;
        return NULL;
    }
    FX_INT32 numECCodewords = 1 << (ecLevel + 1);
    FX_INT32 correctedErrorsCount = correctErrors(codewords, erasures, numECCodewords, e);
    BC_EXCEPTION_CHECK_ReturnValue(e , NULL);
    verifyCodewordCount(codewords, numECCodewords, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    CFX_ByteString bytestring;
    CBC_CommonDecoderResult* decoderResult = CBC_DecodedBitStreamPaser::decode(codewords, bytestring.FormatInteger(ecLevel), e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return decoderResult;
}
FX_INT32 CBC_PDF417ScanningDecoder::correctErrors(CFX_Int32Array &codewords, CFX_Int32Array &erasures, FX_INT32 numECCodewords, FX_INT32 &e)
{
    if ((erasures.GetSize() != 0 && erasures.GetSize() > (numECCodewords / 2 + MAX_ERRORS)) || numECCodewords < 0 || numECCodewords > MAX_EC_CODEWORDS) {
        e = BCExceptionChecksumInstance;
        return -1;
    }
    FX_INT32 result = CBC_PDF417ECErrorCorrection::decode(codewords, numECCodewords, erasures, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, -1);
    return result;
}
void CBC_PDF417ScanningDecoder::verifyCodewordCount(CFX_Int32Array &codewords, FX_INT32 numECCodewords, FX_INT32 &e)
{
    if (codewords.GetSize() < 4) {
        e = BCExceptionFormatInstance;
        return;
    }
    FX_INT32 numberOfCodewords = codewords.GetAt(0);
    if (numberOfCodewords > codewords.GetSize()) {
        e = BCExceptionFormatInstance;
        return;
    }
    if (numberOfCodewords == 0) {
        if (numECCodewords < codewords.GetSize()) {
            codewords[0] = codewords.GetSize() - numECCodewords;
        } else {
            e = BCExceptionFormatInstance;
            return;
        }
    }
}
CFX_Int32Array* CBC_PDF417ScanningDecoder::getBitCountForCodeword(FX_INT32 codeword)
{
    CFX_Int32Array* result = FX_NEW CFX_Int32Array;
    result->SetSize(8);
    FX_INT32 previousValue = 0;
    FX_INT32 i = result->GetSize() - 1;
    while (TRUE) {
        if ((codeword & 0x1) != previousValue) {
            previousValue = codeword & 0x1;
            i--;
            if (i < 0) {
                break;
            }
        }
        result->SetAt(i, result->GetAt(i) + 1);
        codeword >>= 1;
    }
    return result;
}
FX_INT32 CBC_PDF417ScanningDecoder::getCodewordBucketNumber(FX_INT32 codeword)
{
    CFX_Int32Array* array = getBitCountForCodeword(codeword);
    FX_INT32 result = getCodewordBucketNumber(*array);
    delete array;
    return result;
}
FX_INT32 CBC_PDF417ScanningDecoder::getCodewordBucketNumber(CFX_Int32Array& moduleBitCount)
{
    return (moduleBitCount.GetAt(0) - moduleBitCount.GetAt(2) + moduleBitCount.GetAt(4) - moduleBitCount.GetAt(6) + 9) % 9;
}
