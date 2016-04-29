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

#include "xfa/fxbarcode/pdf417/BC_PDF417BarcodeMetadata.h"
#include "xfa/fxbarcode/pdf417/BC_PDF417BoundingBox.h"
#include "xfa/fxbarcode/pdf417/BC_PDF417Codeword.h"
#include "xfa/fxbarcode/pdf417/BC_PDF417Common.h"
#include "xfa/fxbarcode/pdf417/BC_PDF417DetectionResult.h"
#include "xfa/fxbarcode/pdf417/BC_PDF417DetectionResultColumn.h"
#include "xfa/fxbarcode/pdf417/BC_PDF417DetectionResultRowIndicatorColumn.h"

int32_t CBC_DetectionResult::ADJUST_ROW_NUMBER_SKIP = 2;

CBC_DetectionResult::CBC_DetectionResult(CBC_BarcodeMetadata* barcodeMetadata,
                                         CBC_BoundingBox* boundingBox) {
  m_barcodeMetadata = barcodeMetadata;
  m_barcodeColumnCount = barcodeMetadata->getColumnCount();
  m_boundingBox = boundingBox;
  m_detectionResultColumns.SetSize(m_barcodeColumnCount + 2);
  for (int32_t i = 0; i < m_barcodeColumnCount + 2; i++)
    m_detectionResultColumns[i] = nullptr;
}

CBC_DetectionResult::~CBC_DetectionResult() {
  delete m_boundingBox;
  delete m_barcodeMetadata;
}

CFX_ArrayTemplate<CBC_DetectionResultColumn*>&
CBC_DetectionResult::getDetectionResultColumns() {
  adjustIndicatorColumnRowNumbers(m_detectionResultColumns.GetAt(0));
  adjustIndicatorColumnRowNumbers(
      m_detectionResultColumns.GetAt(m_barcodeColumnCount + 1));
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
  return m_detectionResultColumns[barcodeColumn];
}
CFX_ByteString CBC_DetectionResult::toString() {
  CBC_DetectionResultColumn* rowIndicatorColumn = m_detectionResultColumns[0];
  if (!rowIndicatorColumn)
    rowIndicatorColumn = m_detectionResultColumns[m_barcodeColumnCount + 1];

  CFX_ByteString result;
  for (int32_t codewordsRow = 0;
       codewordsRow < rowIndicatorColumn->getCodewords()->GetSize();
       codewordsRow++) {
    result += (FX_CHAR)codewordsRow;
    for (int32_t barcodeColumn = 0; barcodeColumn < m_barcodeColumnCount + 2;
         barcodeColumn++) {
      if (!m_detectionResultColumns[barcodeColumn]) {
        result += "    |   ";
        continue;
      }
      CBC_Codeword* codeword =
          (CBC_Codeword*)m_detectionResultColumns[barcodeColumn]
              ->getCodewords()
              ->GetAt(codewordsRow);
      if (!codeword) {
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
  if (detectionResultColumn) {
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
    CFX_ArrayTemplate<CBC_Codeword*>* codewords =
        m_detectionResultColumns[barcodeColumn]->getCodewords();
    for (int32_t codewordsRow = 0; codewordsRow < codewords->GetSize();
         codewordsRow++) {
      if (codewords->GetAt(codewordsRow) == NULL) {
        continue;
      }
      if (!codewords->GetAt(codewordsRow)->hasValidRowNumber()) {
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
  if (!m_detectionResultColumns[0] ||
      !m_detectionResultColumns[m_barcodeColumnCount + 1]) {
    return 0;
  }
  CFX_ArrayTemplate<CBC_Codeword*>* LRIcodewords =
      m_detectionResultColumns[0]->getCodewords();
  CFX_ArrayTemplate<CBC_Codeword*>* RRIcodewords =
      m_detectionResultColumns[m_barcodeColumnCount + 1]->getCodewords();
  for (int32_t codewordsRow = 0; codewordsRow < LRIcodewords->GetSize();
       codewordsRow++) {
    if (LRIcodewords->GetAt(codewordsRow) &&
        RRIcodewords->GetAt(codewordsRow) &&
        LRIcodewords->GetAt(codewordsRow)->getRowNumber() ==
            RRIcodewords->GetAt(codewordsRow)->getRowNumber()) {
      for (int32_t barcodeColumn = 1; barcodeColumn <= m_barcodeColumnCount;
           barcodeColumn++) {
        CBC_Codeword* codeword =
            m_detectionResultColumns[barcodeColumn]->getCodewords()->GetAt(
                codewordsRow);
        if (!codeword) {
          continue;
        }
        codeword->setRowNumber(
            LRIcodewords->GetAt(codewordsRow)->getRowNumber());
        if (!codeword->hasValidRowNumber()) {
          m_detectionResultColumns[barcodeColumn]->getCodewords()->SetAt(
              codewordsRow, nullptr);
        }
      }
    }
  }
  return 0;
}
int32_t CBC_DetectionResult::adjustRowNumbersFromRRI() {
  if (!m_detectionResultColumns[m_barcodeColumnCount + 1]) {
    return 0;
  }
  int32_t unadjustedCount = 0;
  CFX_ArrayTemplate<CBC_Codeword*>* codewords =
      m_detectionResultColumns.GetAt(m_barcodeColumnCount + 1)->getCodewords();
  for (int32_t codewordsRow = 0; codewordsRow < codewords->GetSize();
       codewordsRow++) {
    if (codewords->GetAt(codewordsRow) == NULL) {
      continue;
    }
    int32_t rowIndicatorRowNumber =
        codewords->GetAt(codewordsRow)->getRowNumber();
    int32_t invalidRowCounts = 0;
    for (int32_t barcodeColumn = m_barcodeColumnCount + 1;
         barcodeColumn > 0 && invalidRowCounts < ADJUST_ROW_NUMBER_SKIP;
         barcodeColumn--) {
      CBC_Codeword* codeword = m_detectionResultColumns.GetAt(barcodeColumn)
                                   ->getCodewords()
                                   ->GetAt(codewordsRow);
      if (codeword) {
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
  CFX_ArrayTemplate<CBC_Codeword*>* codewords =
      m_detectionResultColumns.GetAt(0)->getCodewords();
  for (int32_t codewordsRow = 0; codewordsRow < codewords->GetSize();
       codewordsRow++) {
    if (codewords->GetAt(codewordsRow) == NULL) {
      continue;
    }
    int32_t rowIndicatorRowNumber =
        codewords->GetAt(codewordsRow)->getRowNumber();
    int32_t invalidRowCounts = 0;
    for (int32_t barcodeColumn = 1; barcodeColumn < m_barcodeColumnCount + 1 &&
                                    invalidRowCounts < ADJUST_ROW_NUMBER_SKIP;
         barcodeColumn++) {
      CBC_Codeword* codeword =
          m_detectionResultColumns[barcodeColumn]->getCodewords()->GetAt(
              codewordsRow);
      if (codeword) {
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
void CBC_DetectionResult::adjustRowNumbers(
    int32_t barcodeColumn,
    int32_t codewordsRow,
    CFX_ArrayTemplate<CBC_Codeword*>* codewords) {
  CBC_Codeword* codeword = codewords->GetAt(codewordsRow);
  CFX_ArrayTemplate<CBC_Codeword*>* previousColumnCodewords =
      m_detectionResultColumns.GetAt(barcodeColumn - 1)->getCodewords();
  CFX_ArrayTemplate<CBC_Codeword*>* nextColumnCodewords =
      previousColumnCodewords;
  if (m_detectionResultColumns[barcodeColumn + 1]) {
    nextColumnCodewords =
        m_detectionResultColumns[barcodeColumn + 1]->getCodewords();
  }
  CFX_ArrayTemplate<CBC_Codeword*> otherCodewords;
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
    CBC_Codeword* otherCodeword = otherCodewords.GetAt(i);
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
