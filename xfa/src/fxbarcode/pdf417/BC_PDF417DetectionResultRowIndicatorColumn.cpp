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
#include "../BC_ResultPoint.h"
#include "BC_PDF417BarcodeMetadata.h"
#include "BC_PDF417BoundingBox.h"
#include "BC_PDF417Codeword.h"
#include "BC_PDF417BarcodeValue.h"
#include "BC_PDF417Common.h"
#include "BC_PDF417DetectionResultColumn.h"
#include "BC_PDF417DetectionResultRowIndicatorColumn.h"
CBC_DetectionResultRowIndicatorColumn::CBC_DetectionResultRowIndicatorColumn(CBC_BoundingBox* boundingBox, FX_BOOL isLeft)
    : CBC_DetectionResultColumn(boundingBox)
{
    m_isLeft = isLeft;
}
CBC_DetectionResultRowIndicatorColumn::~CBC_DetectionResultRowIndicatorColumn()
{
}
void CBC_DetectionResultRowIndicatorColumn::setRowNumbers()
{
    for (FX_INT32 i = 0; i < m_codewords->GetSize(); i++) {
        CBC_Codeword * codeword = (CBC_Codeword*)m_codewords->GetAt(i);
        if (codeword != NULL) {
            codeword->setRowNumberAsRowIndicatorColumn();
        }
    }
}
FX_INT32 CBC_DetectionResultRowIndicatorColumn::adjustCompleteIndicatorColumnRowNumbers(CBC_BarcodeMetadata barcodeMetadata)
{
    CFX_PtrArray* codewords = getCodewords();
    setRowNumbers();
    removeIncorrectCodewords(codewords, barcodeMetadata);
    CBC_BoundingBox* boundingBox = getBoundingBox();
    CBC_ResultPoint* top = m_isLeft ? boundingBox->getTopLeft() : boundingBox->getTopRight();
    CBC_ResultPoint* bottom = m_isLeft ? boundingBox->getBottomLeft() : boundingBox->getBottomRight();
    FX_INT32 firstRow = imageRowToCodewordIndex((FX_INT32) top->GetY());
    FX_INT32 lastRow = imageRowToCodewordIndex((FX_INT32) bottom->GetY());
    FX_FLOAT averageRowHeight = (lastRow - firstRow) / (FX_FLOAT) barcodeMetadata.getRowCount();
    FX_INT32 barcodeRow = -1;
    FX_INT32 maxRowHeight = 1;
    FX_INT32 currentRowHeight = 0;
    for (FX_INT32 codewordsRow = firstRow; codewordsRow < lastRow; codewordsRow++) {
        if (codewords->GetAt(codewordsRow) == NULL) {
            continue;
        }
        CBC_Codeword* codeword = (CBC_Codeword*)codewords->GetAt(codewordsRow);
        FX_INT32 rowDifference = codeword->getRowNumber() - barcodeRow;
        if (rowDifference == 0) {
            currentRowHeight++;
        } else if (rowDifference == 1) {
            maxRowHeight = maxRowHeight > currentRowHeight ? maxRowHeight : currentRowHeight;
            currentRowHeight = 1;
            barcodeRow = codeword->getRowNumber();
        } else if (rowDifference < 0) {
            codewords->SetAt(codewordsRow, NULL);
        } else if (codeword->getRowNumber() >= barcodeMetadata.getRowCount()) {
            codewords->SetAt(codewordsRow, NULL);
        } else if (rowDifference > codewordsRow) {
            codewords->SetAt(codewordsRow, NULL);
        } else {
            FX_INT32 checkedRows;
            if (maxRowHeight > 2) {
                checkedRows = (maxRowHeight - 2) * rowDifference;
            } else {
                checkedRows = rowDifference;
            }
            FX_BOOL closePreviousCodewordFound = checkedRows >= codewordsRow;
            for (FX_INT32 i = 1; i <= checkedRows && !closePreviousCodewordFound; i++) {
                closePreviousCodewordFound = codewords->GetAt(codewordsRow - i) != NULL;
            }
            if (closePreviousCodewordFound) {
                codewords->SetAt(codewordsRow, NULL);
            } else {
                barcodeRow = codeword->getRowNumber();
                currentRowHeight = 1;
            }
        }
    }
    return (FX_INT32) (averageRowHeight + 0.5);
}
CFX_Int32Array* CBC_DetectionResultRowIndicatorColumn::getRowHeights(FX_INT32 &e)
{
    CBC_BarcodeMetadata* barcodeMetadata = getBarcodeMetadata();
    if (barcodeMetadata == NULL) {
        e = BCExceptionCannotMetadata;
        return NULL;
    }
    adjustIncompleteIndicatorColumnRowNumbers(*barcodeMetadata);
    CFX_Int32Array* result = FX_NEW CFX_Int32Array;
    result->SetSize(barcodeMetadata->getRowCount());
    for (FX_INT32 i = 0; i < getCodewords()->GetSize(); i++) {
        CBC_Codeword* codeword = (CBC_Codeword*)getCodewords()->GetAt(i);
        if (codeword != NULL) {
            result->SetAt(codeword->getRowNumber(),  result->GetAt(codeword->getRowNumber()) + 1);
        }
    }
    return result;
}
FX_INT32 CBC_DetectionResultRowIndicatorColumn::adjustIncompleteIndicatorColumnRowNumbers(CBC_BarcodeMetadata barcodeMetadata)
{
    CBC_BoundingBox* boundingBox = getBoundingBox();
    CBC_ResultPoint* top = m_isLeft ? boundingBox->getTopLeft() : boundingBox->getTopRight();
    CBC_ResultPoint* bottom = m_isLeft ? boundingBox->getBottomLeft() : boundingBox->getBottomRight();
    FX_INT32 firstRow = imageRowToCodewordIndex((FX_INT32) top->GetY());
    FX_INT32 lastRow = imageRowToCodewordIndex((FX_INT32) bottom->GetY());
    FX_FLOAT averageRowHeight = (lastRow - firstRow) / (FX_FLOAT) barcodeMetadata.getRowCount();
    CFX_PtrArray* codewords = getCodewords();
    FX_INT32 barcodeRow = -1;
    FX_INT32 maxRowHeight = 1;
    FX_INT32 currentRowHeight = 0;
    for (FX_INT32 codewordsRow = firstRow; codewordsRow < lastRow; codewordsRow++) {
        if (codewords->GetAt(codewordsRow) == NULL) {
            continue;
        }
        CBC_Codeword* codeword = (CBC_Codeword*)codewords->GetAt(codewordsRow);
        codeword->setRowNumberAsRowIndicatorColumn();
        FX_INT32 rowDifference = codeword->getRowNumber() - barcodeRow;
        if (rowDifference == 0) {
            currentRowHeight++;
        } else if (rowDifference == 1) {
            maxRowHeight = maxRowHeight > currentRowHeight ? maxRowHeight : currentRowHeight;
            currentRowHeight = 1;
            barcodeRow = codeword->getRowNumber();
        } else if (codeword->getRowNumber() >= barcodeMetadata.getRowCount()) {
            codewords->SetAt(codewordsRow, NULL);
        } else {
            barcodeRow = codeword->getRowNumber();
            currentRowHeight = 1;
        }
    }
    return (FX_INT32) (averageRowHeight + 0.5);
}
CBC_BarcodeMetadata* CBC_DetectionResultRowIndicatorColumn::getBarcodeMetadata()
{
    CFX_PtrArray* codewords = getCodewords();
    CBC_BarcodeValue barcodeColumnCount;
    CBC_BarcodeValue barcodeRowCountUpperPart;
    CBC_BarcodeValue barcodeRowCountLowerPart;
    CBC_BarcodeValue barcodeECLevel;
    for (FX_INT32 i = 0; i < codewords->GetSize(); i++) {
        CBC_Codeword* codeword = (CBC_Codeword*)codewords->GetAt(i);
        if (codeword == NULL) {
            continue;
        }
        codeword->setRowNumberAsRowIndicatorColumn();
        FX_INT32 rowIndicatorValue = codeword->getValue() % 30;
        FX_INT32 codewordRowNumber = codeword->getRowNumber();
        if (!m_isLeft) {
            codewordRowNumber += 2;
        }
        switch (codewordRowNumber % 3) {
            case 0:
                barcodeRowCountUpperPart.setValue(rowIndicatorValue * 3 + 1);
                break;
            case 1:
                barcodeECLevel.setValue(rowIndicatorValue / 3);
                barcodeRowCountLowerPart.setValue(rowIndicatorValue % 3);
                break;
            case 2:
                barcodeColumnCount.setValue(rowIndicatorValue + 1);
                break;
        }
    }
    if ((barcodeColumnCount.getValue()->GetSize() == 0) ||
            (barcodeRowCountUpperPart.getValue()->GetSize() == 0) ||
            (barcodeRowCountLowerPart.getValue()->GetSize() == 0) ||
            (barcodeECLevel.getValue()->GetSize() == 0) ||
            barcodeColumnCount.getValue()->GetAt(0) < 1 ||
            barcodeRowCountUpperPart.getValue()->GetAt(0) + barcodeRowCountLowerPart.getValue()->GetAt(0) < CBC_PDF417Common::MIN_ROWS_IN_BARCODE ||
            barcodeRowCountUpperPart.getValue()->GetAt(0) + barcodeRowCountLowerPart.getValue()->GetAt(0) > CBC_PDF417Common::MAX_ROWS_IN_BARCODE) {
        return NULL;
    }
    CBC_BarcodeMetadata* barcodeMetadata = FX_NEW CBC_BarcodeMetadata(barcodeColumnCount.getValue()->GetAt(0), barcodeRowCountUpperPart.getValue()->GetAt(0), barcodeRowCountLowerPart.getValue()->GetAt(0), barcodeECLevel.getValue()->GetAt(0));
    removeIncorrectCodewords(codewords, *barcodeMetadata);
    return barcodeMetadata;
}
FX_BOOL CBC_DetectionResultRowIndicatorColumn::isLeft()
{
    return m_isLeft;
}
CFX_ByteString CBC_DetectionResultRowIndicatorColumn::toString()
{
    return (CFX_ByteString)"IsLeft: " + (CFX_ByteString)m_isLeft + '\n' + CBC_DetectionResultColumn::toString();
}
void CBC_DetectionResultRowIndicatorColumn::removeIncorrectCodewords(CFX_PtrArray* codewords, CBC_BarcodeMetadata barcodeMetadata)
{
    for (FX_INT32 codewordRow = 0; codewordRow < codewords->GetSize(); codewordRow++) {
        CBC_Codeword* codeword = (CBC_Codeword*)codewords->GetAt(codewordRow);
        if (codeword == NULL) {
            continue;
        }
        FX_INT32 rowIndicatorValue = codeword->getValue() % 30;
        FX_INT32 codewordRowNumber = codeword->getRowNumber();
        if (codewordRowNumber > barcodeMetadata.getRowCount()) {
            codewords->SetAt(codewordRow, NULL);
            continue;
        }
        if (!m_isLeft) {
            codewordRowNumber += 2;
        }
        switch (codewordRowNumber % 3) {
            case 0:
                if (rowIndicatorValue * 3 + 1 != barcodeMetadata.getRowCountUpperPart()) {
                    codewords->SetAt(codewordRow, NULL);
                }
                break;
            case 1:
                if (rowIndicatorValue / 3 != barcodeMetadata.getErrorCorrectionLevel() ||
                        rowIndicatorValue % 3 != barcodeMetadata.getRowCountLowerPart()) {
                    codewords->SetAt(codewordRow, NULL);
                }
                break;
            case 2:
                if (rowIndicatorValue + 1 != barcodeMetadata.getColumnCount()) {
                    codewords->SetAt(codewordRow, NULL);
                }
                break;
        }
    }
}
