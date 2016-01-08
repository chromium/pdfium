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

#include "xfa/src/fxbarcode/barcode.h"
#include "BC_PDF417Codeword.h"
#include "BC_PDF417BarcodeMetadata.h"
#include "BC_PDF417BoundingBox.h"
#include "BC_PDF417DetectionResultColumn.h"
#include "BC_PDF417Common.h"
#include "BC_PDF417DetectionResultRowIndicatorColumn.h"
#include "BC_PDF417DetectionResult.h"
int32_t CBC_DetectionResult::ADJUST_ROW_NUMBER_SKIP = 2;
CBC_DetectionResult::CBC_DetectionResult(CBC_BarcodeMetadata* barcodeMetadata,
                                         CBC_BoundingBox* boundingBox) {
  m_barcodeMetadata = barcodeMetadata;
  m_barcodeColumnCount = barcodeMetadata->getColumnCount();
  m_boundingBox = boundingBox;
  m_detectionResultColumns.SetSize(m_barcodeColumnCount + 2);
  for (int32_t i = 0; i < m_barcodeColumnCount + 2; i++) {
    m_detectionResultColumns[i] = NULL;
  }
}
CBC_DetectionResult::~CBC_DetectionResult() {
  delete m_boundingBox;
  delete m_barcodeMetadata;
  m_detectionResultColumns.RemoveAll();
}
CFX_PtrArray& CBC_DetectionResult::getDetectionResultColumns() {
  adjustIndicatorColumnRowNumbers(
      (CBC_DetectionResultColumn*)m_detectionResultColumns.GetAt(0));
  adjustIndicatorColumnRowNumbers(
      (CBC_DetectionResultColumn*)m_detectionResultColumns.GetAt(
          m_barcodeColumnCount + 1));
  int32_t unadjustedCodewordCount = CBC_PDF417Common::MAX_CODEWORDS_IN_BARCODE;
  int32_t previousUnadjustedCount;
  do {
    previousUnadjustedCount = unadjustedCodewordCount;
    unadjustedCodewordCount = adjustRowNumbers();
  } while (unadjustedCodewordCount > 0 &&
           unadjustedCodewordCount < previousUnadjustedCount);
  return m_detectionResultColumns;
}
void CBC_DetectionResult::setBoundingBox(CBC_BoundingBox* boundingBox) {
  m_boundingBox = boundingBox;
}
CBC_BoundingBox* CBC_DetectionResult::getBoundingBox() {
  return m_boundingBox;
}
void CBC_DetectionResult::setDetectionResultColumn(
    int32_t barcodeColumn,
    CBC_DetectionResultColumn* detectionResultColumn) {
  m_detectionResultColumns[barcodeColumn] = detectionResultColumn;
}
CBC_DetectionResultColumn* CBC_DetectionResult::getDetectionResultColumn(
    int32_t barcodeColumn) {
  return (CBC_DetectionResultColumn*)m_detectionResultColumns[barcodeColumn];
}
CFX_ByteString CBC_DetectionResult::toString() {
  CBC_DetectionResultColumn* rowIndicatorColumn =
      (CBC_DetectionResultColumn*)m_detectionResultColumns[0];
  if (rowIndicatorColumn == NULL) {
    rowIndicatorColumn = (CBC_DetectionResultColumn*)
        m_detectionResultColumns[m_barcodeColumnCount + 1];
  }
  CFX_ByteString result;
  for (int32_t codewordsRow = 0;
       codewordsRow < rowIndicatorColumn->getCodewords()->GetSize();
       codewordsRow++) {
    result += (FX_CHAR)codewordsRow;
    for (int32_t barcodeColumn = 0; barcodeColumn < m_barcodeColumnCount + 2;
         barcodeColumn++) {
      if (m_detectionResultColumns[barcodeColumn] == NULL) {
        result += "    |   ";
        continue;
      }
      CBC_Codeword* codeword =
          (CBC_Codeword*)((CBC_DetectionResultColumn*)
                              m_detectionResultColumns[barcodeColumn])
              ->getCodewords()
              ->GetAt(codewordsRow);
      if (codeword == NULL) {
        result += "    |   ";
        continue;
      }
      result += codeword->getRowNumber();
      result += codeword->getValue();
    }
  }
  return result;
}
void CBC_DetectionResult::adjustIndicatorColumnRowNumbers(
    CBC_DetectionResultColumn* detectionResultColumn) {
  if (detectionResultColumn != NULL) {
    ((CBC_DetectionResultRowIndicatorColumn*)detectionResultColumn)
        ->adjustCompleteIndicatorColumnRowNumbers(*m_barcodeMetadata);
  }
}
int32_t CBC_DetectionResult::adjustRowNumbers() {
  int32_t unadjustedCount = adjustRowNumbersByRow();
  if (unadjustedCount == 0) {
    return 0;
  }
  for (int32_t barcodeColumn = 1; barcodeColumn < m_barcodeColumnCount + 1;
       barcodeColumn++) {
    CFX_PtrArray* codewords =
        ((CBC_DetectionResultColumn*)m_detectionResultColumns[barcodeColumn])
            ->getCodewords();
    for (int32_t codewordsRow = 0; codewordsRow < codewords->GetSize();
         codewordsRow++) {
      if (codewords->GetAt(codewordsRow) == NULL) {
        continue;
      }
      if (!((CBC_Codeword*)codewords->GetAt(codewordsRow))
               ->hasValidRowNumber()) {
        adjustRowNumbers(barcodeColumn, codewordsRow, codewords);
      }
    }
  }
  return unadjustedCount;
}
int32_t CBC_DetectionResult::adjustRowNumbersByRow() {
  adjustRowNumbersFromBothRI();
  int32_t unadjustedCount = adjustRowNumbersFromLRI();
  return unadjustedCount + adjustRowNumbersFromRRI();
}
int32_t CBC_DetectionResult::adjustRowNumbersFromBothRI() {
  if (m_detectionResultColumns[0] == NULL ||
      m_detectionResultColumns[m_barcodeColumnCount + 1] == NULL) {
    return 0;
  }
  CFX_PtrArray* LRIcodewords =
      ((CBC_DetectionResultColumn*)m_detectionResultColumns[0])->getCodewords();
  CFX_PtrArray* RRIcodewords =
      ((CBC_DetectionResultColumn*)
           m_detectionResultColumns[m_barcodeColumnCount + 1])
          ->getCodewords();
  for (int32_t codewordsRow = 0; codewordsRow < LRIcodewords->GetSize();
       codewordsRow++) {
    if (LRIcodewords->GetAt(codewordsRow) != NULL &&
        RRIcodewords->GetAt(codewordsRow) != NULL &&
        ((CBC_Codeword*)LRIcodewords->GetAt(codewordsRow))->getRowNumber() ==
            ((CBC_Codeword*)RRIcodewords->GetAt(codewordsRow))
                ->getRowNumber()) {
      for (int32_t barcodeColumn = 1; barcodeColumn <= m_barcodeColumnCount;
           barcodeColumn++) {
        CBC_Codeword* codeword =
            (CBC_Codeword*)((CBC_DetectionResultColumn*)
                                m_detectionResultColumns[barcodeColumn])
                ->getCodewords()
                ->GetAt(codewordsRow);
        if (codeword == NULL) {
          continue;
        }
        codeword->setRowNumber(
            ((CBC_Codeword*)LRIcodewords->GetAt(codewordsRow))->getRowNumber());
        if (!codeword->hasValidRowNumber()) {
          ((CBC_DetectionResultColumn*)m_detectionResultColumns[barcodeColumn])
              ->getCodewords()
              ->SetAt(codewordsRow, NULL);
        }
      }
    }
  }
  return 0;
}
int32_t CBC_DetectionResult::adjustRowNumbersFromRRI() {
  if (m_detectionResultColumns[m_barcodeColumnCount + 1] == NULL) {
    return 0;
  }
  int32_t unadjustedCount = 0;
  CFX_PtrArray* codewords =
      ((CBC_DetectionResultColumn*)m_detectionResultColumns.GetAt(
           m_barcodeColumnCount + 1))
          ->getCodewords();
  for (int32_t codewordsRow = 0; codewordsRow < codewords->GetSize();
       codewordsRow++) {
    if (codewords->GetAt(codewordsRow) == NULL) {
      continue;
    }
    int32_t rowIndicatorRowNumber =
        ((CBC_Codeword*)codewords->GetAt(codewordsRow))->getRowNumber();
    int32_t invalidRowCounts = 0;
    for (int32_t barcodeColumn = m_barcodeColumnCount + 1;
         barcodeColumn > 0 && invalidRowCounts < ADJUST_ROW_NUMBER_SKIP;
         barcodeColumn--) {
      CBC_Codeword* codeword =
          (CBC_Codeword*)((CBC_DetectionResultColumn*)
                              m_detectionResultColumns.GetAt(barcodeColumn))
              ->getCodewords()
              ->GetAt(codewordsRow);
      if (codeword != NULL) {
        invalidRowCounts = adjustRowNumberIfValid(rowIndicatorRowNumber,
                                                  invalidRowCounts, codeword);
        if (!codeword->hasValidRowNumber()) {
          unadjustedCount++;
        }
      }
    }
  }
  return unadjustedCount;
}
int32_t CBC_DetectionResult::adjustRowNumbersFromLRI() {
  if (m_detectionResultColumns[0] == NULL) {
    return 0;
  }
  int32_t unadjustedCount = 0;
  CFX_PtrArray* codewords =
      ((CBC_DetectionResultColumn*)m_detectionResultColumns.GetAt(0))
          ->getCodewords();
  for (int32_t codewordsRow = 0; codewordsRow < codewords->GetSize();
       codewordsRow++) {
    if (codewords->GetAt(codewordsRow) == NULL) {
      continue;
    }
    int32_t rowIndicatorRowNumber =
        ((CBC_Codeword*)codewords->GetAt(codewordsRow))->getRowNumber();
    int32_t invalidRowCounts = 0;
    for (int32_t barcodeColumn = 1; barcodeColumn < m_barcodeColumnCount + 1 &&
                                    invalidRowCounts < ADJUST_ROW_NUMBER_SKIP;
         barcodeColumn++) {
      CBC_Codeword* codeword =
          (CBC_Codeword*)((CBC_DetectionResultColumn*)
                              m_detectionResultColumns[barcodeColumn])
              ->getCodewords()
              ->GetAt(codewordsRow);
      if (codeword != NULL) {
        invalidRowCounts = adjustRowNumberIfValid(rowIndicatorRowNumber,
                                                  invalidRowCounts, codeword);
        if (!codeword->hasValidRowNumber()) {
          unadjustedCount++;
        }
      }
    }
  }
  return unadjustedCount;
}
int32_t CBC_DetectionResult::adjustRowNumberIfValid(
    int32_t rowIndicatorRowNumber,
    int32_t invalidRowCounts,
    CBC_Codeword* codeword) {
  if (codeword == NULL) {
    return invalidRowCounts;
  }
  if (!codeword->hasValidRowNumber()) {
    if (codeword->isValidRowNumber(rowIndicatorRowNumber)) {
      codeword->setRowNumber(rowIndicatorRowNumber);
      invalidRowCounts = 0;
    } else {
      ++invalidRowCounts;
    }
  }
  return invalidRowCounts;
}
void CBC_DetectionResult::adjustRowNumbers(int32_t barcodeColumn,
                                           int32_t codewordsRow,
                                           CFX_PtrArray* codewords) {
  CBC_Codeword* codeword = (CBC_Codeword*)codewords->GetAt(codewordsRow);
  CFX_PtrArray* previousColumnCodewords =
      ((CBC_DetectionResultColumn*)m_detectionResultColumns.GetAt(
           barcodeColumn - 1))
          ->getCodewords();
  CFX_PtrArray* nextColumnCodewords = previousColumnCodewords;
  if (m_detectionResultColumns[barcodeColumn + 1] != NULL) {
    nextColumnCodewords = ((CBC_DetectionResultColumn*)
                               m_detectionResultColumns[barcodeColumn + 1])
                              ->getCodewords();
  }
  CFX_PtrArray otherCodewords;
  otherCodewords.SetSize(14);
  otherCodewords[2] = previousColumnCodewords->GetAt(codewordsRow);
  otherCodewords[3] = nextColumnCodewords->GetAt(codewordsRow);
  if (codewordsRow > 0) {
    otherCodewords[0] = codewords->GetAt(codewordsRow - 1);
    otherCodewords[4] = previousColumnCodewords->GetAt(codewordsRow - 1);
    otherCodewords[5] = nextColumnCodewords->GetAt(codewordsRow - 1);
  }
  if (codewordsRow > 1) {
    otherCodewords[8] = codewords->GetAt(codewordsRow - 2);
    otherCodewords[10] = previousColumnCodewords->GetAt(codewordsRow - 2);
    otherCodewords[11] = nextColumnCodewords->GetAt(codewordsRow - 2);
  }
  if (codewordsRow < codewords->GetSize() - 1) {
    otherCodewords[1] = codewords->GetAt(codewordsRow + 1);
    otherCodewords[6] = previousColumnCodewords->GetAt(codewordsRow + 1);
    otherCodewords[7] = nextColumnCodewords->GetAt(codewordsRow + 1);
  }
  if (codewordsRow < codewords->GetSize() - 2) {
    otherCodewords[9] = codewords->GetAt(codewordsRow + 2);
    otherCodewords[12] = previousColumnCodewords->GetAt(codewordsRow + 2);
    otherCodewords[13] = nextColumnCodewords->GetAt(codewordsRow + 2);
  }
  for (int32_t i = 0; i < otherCodewords.GetSize(); i++) {
    CBC_Codeword* otherCodeword = (CBC_Codeword*)otherCodewords.GetAt(i);
    if (adjustRowNumber(codeword, otherCodeword)) {
      return;
    }
  }
}
FX_BOOL CBC_DetectionResult::adjustRowNumber(CBC_Codeword* codeword,
                                             CBC_Codeword* otherCodeword) {
  if (otherCodeword == NULL) {
    return FALSE;
  }
  if (otherCodeword->hasValidRowNumber() &&
      otherCodeword->getBucket() == codeword->getBucket()) {
    codeword->setRowNumber(otherCodeword->getRowNumber());
    return TRUE;
  }
  return FALSE;
}
int32_t CBC_DetectionResult::getBarcodeColumnCount() {
  return m_barcodeColumnCount;
}
int32_t CBC_DetectionResult::getBarcodeRowCount() {
  return m_barcodeMetadata->getRowCount();
}
int32_t CBC_DetectionResult::getBarcodeECLevel() {
  return m_barcodeMetadata->getErrorCorrectionLevel();
}
