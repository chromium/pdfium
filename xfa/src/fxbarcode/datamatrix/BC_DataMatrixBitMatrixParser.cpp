// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2007 ZXing authors
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
#include "xfa/src/fxbarcode/common/BC_CommonBitMatrix.h"
#include "BC_DataMatrixVersion.h"
#include "BC_DataMatrixBitMatrixParser.h"
CBC_DataMatrixBitMatrixParser::CBC_DataMatrixBitMatrixParser() {
  m_mappingBitMatrix = NULL;
  m_version = NULL;
  m_readMappingMatrix = NULL;
}
void CBC_DataMatrixBitMatrixParser::Init(CBC_CommonBitMatrix* bitMatrix,
                                         int32_t& e) {
  int32_t dimension = bitMatrix->GetHeight();
  if (dimension < 8 || dimension > 144 || (dimension & 0x01) != 0) {
    e = BCExceptionFormatException;
    return;
  }
  m_version = ReadVersion(bitMatrix, e);
  BC_EXCEPTION_CHECK_ReturnVoid(e);
  m_mappingBitMatrix = ExtractDataRegion(bitMatrix, e);
  BC_EXCEPTION_CHECK_ReturnVoid(e);
  m_readMappingMatrix = new CBC_CommonBitMatrix();
  m_readMappingMatrix->Init(m_mappingBitMatrix->GetWidth(),
                            m_mappingBitMatrix->GetHeight());
}
CBC_DataMatrixBitMatrixParser::~CBC_DataMatrixBitMatrixParser() {
  if (m_mappingBitMatrix != NULL) {
    delete m_mappingBitMatrix;
  }
  m_mappingBitMatrix = NULL;
  if (m_readMappingMatrix != NULL) {
    delete m_readMappingMatrix;
  }
  m_readMappingMatrix = NULL;
}
CBC_DataMatrixVersion* CBC_DataMatrixBitMatrixParser::GetVersion() {
  return m_version;
}
CBC_DataMatrixVersion* CBC_DataMatrixBitMatrixParser::ReadVersion(
    CBC_CommonBitMatrix* bitMatrix,
    int32_t& e) {
  int32_t rows = bitMatrix->GetHeight();
  int32_t columns = bitMatrix->GetWidth();
  CBC_DataMatrixVersion* temp =
      CBC_DataMatrixVersion::GetVersionForDimensions(rows, columns, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return temp;
}
CFX_ByteArray* CBC_DataMatrixBitMatrixParser::ReadCodewords(int32_t& e) {
  CBC_AutoPtr<CFX_ByteArray> result(new CFX_ByteArray());
  result->SetSize(m_version->GetTotalCodewords());
  int32_t resultOffset = 0;
  int32_t row = 4;
  int32_t column = 0;
  int32_t numRows = m_mappingBitMatrix->GetHeight();
  int32_t numColumns = m_mappingBitMatrix->GetWidth();
  FX_BOOL corner1Read = FALSE;
  FX_BOOL corner2Read = FALSE;
  FX_BOOL corner3Read = FALSE;
  FX_BOOL corner4Read = FALSE;
  do {
    if ((row == numRows) && (column == 0) && !corner1Read) {
      (*result)[resultOffset++] = (uint8_t)ReadCorner1(numRows, numColumns);
      row -= 2;
      column += 2;
      corner1Read = TRUE;
    } else if ((row == numRows - 2) && (column == 0) &&
               ((numColumns & 0x03) != 0) && !corner2Read) {
      (*result)[resultOffset++] = (uint8_t)ReadCorner2(numRows, numColumns);
      row -= 2;
      column += 2;
      corner2Read = TRUE;
    } else if ((row == numRows + 4) && (column == 2) &&
               ((numColumns & 0x07) == 0) && !corner3Read) {
      (*result)[resultOffset++] = (uint8_t)ReadCorner3(numRows, numColumns);
      row -= 2;
      column += 2;
      corner3Read = TRUE;
    } else if ((row == numRows - 2) && (column == 0) &&
               ((numColumns & 0x07) == 4) && !corner4Read) {
      (*result)[resultOffset++] = (uint8_t)ReadCorner4(numRows, numColumns);
      row -= 2;
      column += 2;
      corner4Read = TRUE;
    } else {
      do {
        if ((row < numRows) && (column >= 0) &&
            !m_readMappingMatrix->Get(column, row)) {
          if (resultOffset < (*result).GetSize()) {
            (*result)[resultOffset++] =
                (uint8_t)ReadUtah(row, column, numRows, numColumns);
          }
        }
        row -= 2;
        column += 2;
      } while ((row >= 0) && (column < numColumns));
      row += 1;
      column += 3;
      do {
        if ((row >= 0) && (column < numColumns) &&
            !m_readMappingMatrix->Get(column, row)) {
          if (resultOffset < (*result).GetSize()) {
            (*result)[resultOffset++] =
                (uint8_t)ReadUtah(row, column, numRows, numColumns);
          }
        }
        row += 2;
        column -= 2;
      } while ((row < numRows) && (column >= 0));
      row += 3;
      column += 1;
    }
  } while ((row < numRows) || (column < numColumns));
  if (resultOffset != m_version->GetTotalCodewords()) {
    e = BCExceptionFormatException;
    return NULL;
  }
  return result.release();
}
FX_BOOL CBC_DataMatrixBitMatrixParser::ReadModule(int32_t row,
                                                  int32_t column,
                                                  int32_t numRows,
                                                  int32_t numColumns) {
  if (row < 0) {
    row += numRows;
    column += 4 - ((numRows + 4) & 0x07);
  }
  if (column < 0) {
    column += numColumns;
    row += 4 - ((numColumns + 4) & 0x07);
  }
  m_readMappingMatrix->Set(column, row);
  return m_mappingBitMatrix->Get(column, row);
}
int32_t CBC_DataMatrixBitMatrixParser::ReadUtah(int32_t row,
                                                int32_t column,
                                                int32_t numRows,
                                                int32_t numColumns) {
  int32_t currentByte = 0;
  if (ReadModule(row - 2, column - 2, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(row - 2, column - 1, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(row - 1, column - 2, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(row - 1, column - 1, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(row - 1, column, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(row, column - 2, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(row, column - 1, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(row, column, numRows, numColumns)) {
    currentByte |= 1;
  }
  return currentByte;
}
int32_t CBC_DataMatrixBitMatrixParser::ReadCorner1(int32_t numRows,
                                                   int32_t numColumns) {
  int32_t currentByte = 0;
  if (ReadModule(numRows - 1, 0, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(numRows - 1, 1, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(numRows - 1, 2, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(0, numColumns - 2, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(0, numColumns - 1, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(1, numColumns - 1, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(2, numColumns - 1, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(3, numColumns - 1, numRows, numColumns)) {
    currentByte |= 1;
  }
  return currentByte;
}
int32_t CBC_DataMatrixBitMatrixParser::ReadCorner2(int32_t numRows,
                                                   int32_t numColumns) {
  int32_t currentByte = 0;
  if (ReadModule(numRows - 3, 0, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(numRows - 2, 0, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(numRows - 1, 0, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(0, numColumns - 4, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(0, numColumns - 3, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(0, numColumns - 2, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(0, numColumns - 1, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(1, numColumns - 1, numRows, numColumns)) {
    currentByte |= 1;
  }
  return currentByte;
}
int32_t CBC_DataMatrixBitMatrixParser::ReadCorner3(int32_t numRows,
                                                   int32_t numColumns) {
  int32_t currentByte = 0;
  if (ReadModule(numRows - 1, 0, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(numRows - 1, numColumns - 1, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(0, numColumns - 3, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(0, numColumns - 2, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(0, numColumns - 1, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(1, numColumns - 3, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(1, numColumns - 2, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(1, numColumns - 1, numRows, numColumns)) {
    currentByte |= 1;
  }
  return currentByte;
}
int32_t CBC_DataMatrixBitMatrixParser::ReadCorner4(int32_t numRows,
                                                   int32_t numColumns) {
  int32_t currentByte = 0;
  if (ReadModule(numRows - 3, 0, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(numRows - 2, 0, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(numRows - 1, 0, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(0, numColumns - 2, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(0, numColumns - 1, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(1, numColumns - 1, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(2, numColumns - 1, numRows, numColumns)) {
    currentByte |= 1;
  }
  currentByte <<= 1;
  if (ReadModule(3, numColumns - 1, numRows, numColumns)) {
    currentByte |= 1;
  }
  return currentByte;
}
CBC_CommonBitMatrix* CBC_DataMatrixBitMatrixParser::ExtractDataRegion(
    CBC_CommonBitMatrix* bitMatrix,
    int32_t& e) {
  int32_t symbolSizeRows = m_version->GetSymbolSizeRows();
  int32_t symbolSizeColumns = m_version->GetSymbolSizeColumns();
  if (bitMatrix->GetHeight() != symbolSizeRows) {
    e = BCExceptionCanNotCallGetDimensionOnNonSquareMatrix;
    return NULL;
  }
  int32_t dataRegionSizeRows = m_version->GetDataRegionSizeRows();
  int32_t dataRegionSizeColumns = m_version->GetDataRegionSizeColumns();
  int32_t numDataRegionsRow = symbolSizeRows / dataRegionSizeRows;
  int32_t numDataRegionsColumn = symbolSizeColumns / dataRegionSizeColumns;
  int32_t sizeDataRegionRow = numDataRegionsRow * dataRegionSizeRows;
  int32_t sizeDataRegionColumn = numDataRegionsColumn * dataRegionSizeColumns;
  CBC_CommonBitMatrix* bitMatrixWithoutAlignment = new CBC_CommonBitMatrix();
  bitMatrixWithoutAlignment->Init(sizeDataRegionColumn, sizeDataRegionRow);
  int32_t dataRegionRow;
  for (dataRegionRow = 0; dataRegionRow < numDataRegionsRow; ++dataRegionRow) {
    int32_t dataRegionRowOffset = dataRegionRow * dataRegionSizeRows;
    int32_t dataRegionColumn;
    for (dataRegionColumn = 0; dataRegionColumn < numDataRegionsColumn;
         ++dataRegionColumn) {
      int32_t dataRegionColumnOffset = dataRegionColumn * dataRegionSizeColumns;
      int32_t i;
      for (i = 0; i < dataRegionSizeRows; ++i) {
        int32_t readRowOffset =
            dataRegionRow * (dataRegionSizeRows + 2) + 1 + i;
        int32_t writeRowOffset = dataRegionRowOffset + i;
        int32_t j;
        for (j = 0; j < dataRegionSizeColumns; ++j) {
          int32_t readColumnOffset =
              dataRegionColumn * (dataRegionSizeColumns + 2) + 1 + j;
          if (bitMatrix->Get(readColumnOffset, readRowOffset)) {
            int32_t writeColumnOffset = dataRegionColumnOffset + j;
            bitMatrixWithoutAlignment->Set(writeColumnOffset, writeRowOffset);
          }
        }
      }
    }
  }
  return bitMatrixWithoutAlignment;
}
