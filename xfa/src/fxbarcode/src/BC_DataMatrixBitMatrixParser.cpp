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

#include "barcode.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_DataMatrixVersion.h"
#include "include/BC_DataMatrixBitMatrixParser.h"
CBC_DataMatrixBitMatrixParser::CBC_DataMatrixBitMatrixParser()
{
    m_mappingBitMatrix = NULL;
    m_version = NULL;
    m_readMappingMatrix = NULL;
}
void CBC_DataMatrixBitMatrixParser::Init(CBC_CommonBitMatrix *bitMatrix, FX_INT32 &e)
{
    FX_INT32 dimension = bitMatrix->GetHeight();
    if (dimension < 8 || dimension > 144 || (dimension & 0x01) != 0) {
        e = BCExceptionFormatException;
        return;
    }
    m_version = ReadVersion(bitMatrix, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    m_mappingBitMatrix = ExtractDataRegion(bitMatrix, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    m_readMappingMatrix = FX_NEW CBC_CommonBitMatrix();
    m_readMappingMatrix->Init(m_mappingBitMatrix->GetWidth(), m_mappingBitMatrix->GetHeight());
}
CBC_DataMatrixBitMatrixParser::~CBC_DataMatrixBitMatrixParser()
{
    if(m_mappingBitMatrix != NULL) {
        delete m_mappingBitMatrix;
    }
    m_mappingBitMatrix = NULL;
    if(m_readMappingMatrix != NULL) {
        delete m_readMappingMatrix;
    }
    m_readMappingMatrix = NULL;
}
CBC_DataMatrixVersion *CBC_DataMatrixBitMatrixParser::GetVersion()
{
    return m_version;
}
CBC_DataMatrixVersion *CBC_DataMatrixBitMatrixParser::ReadVersion(CBC_CommonBitMatrix *bitMatrix, FX_INT32 &e)
{
    FX_INT32 rows = bitMatrix->GetHeight();
    FX_INT32 columns = bitMatrix->GetWidth();
    CBC_DataMatrixVersion *temp = CBC_DataMatrixVersion::GetVersionForDimensions(rows, columns, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return temp;
}
CFX_ByteArray *CBC_DataMatrixBitMatrixParser::ReadCodewords(FX_INT32 &e)
{
    CBC_AutoPtr<CFX_ByteArray> result(FX_NEW CFX_ByteArray());
    result->SetSize(m_version->GetTotalCodewords());
    FX_INT32 resultOffset = 0;
    FX_INT32 row = 4;
    FX_INT32 column = 0;
    FX_INT32 numRows = m_mappingBitMatrix->GetHeight();
    FX_INT32 numColumns = m_mappingBitMatrix->GetWidth();
    FX_BOOL corner1Read = FALSE;
    FX_BOOL corner2Read = FALSE;
    FX_BOOL corner3Read = FALSE;
    FX_BOOL corner4Read = FALSE;
    do {
        if ((row == numRows) && (column == 0) && !corner1Read) {
            (*result)[resultOffset++] = (FX_BYTE) ReadCorner1(numRows, numColumns);
            row -= 2;
            column += 2;
            corner1Read = TRUE;
        } else if ((row == numRows - 2) && (column == 0) && ((numColumns & 0x03) != 0) && !corner2Read) {
            (*result)[resultOffset++] = (FX_BYTE) ReadCorner2(numRows, numColumns);
            row -= 2;
            column += 2;
            corner2Read = TRUE;
        } else if ((row == numRows + 4) && (column == 2) && ((numColumns & 0x07) == 0) && !corner3Read) {
            (*result)[resultOffset++] = (FX_BYTE) ReadCorner3(numRows, numColumns);
            row -= 2;
            column += 2;
            corner3Read = TRUE;
        } else if ((row == numRows - 2) && (column == 0) && ((numColumns & 0x07) == 4) && !corner4Read) {
            (*result)[resultOffset++] = (FX_BYTE) ReadCorner4(numRows, numColumns);
            row -= 2;
            column += 2;
            corner4Read = TRUE;
        } else {
            do {
                if ((row < numRows) && (column >= 0) && !m_readMappingMatrix->Get(column, row)) {
                    if (resultOffset < (*result).GetSize() ) {
                        (*result)[resultOffset++] = (FX_BYTE) ReadUtah(row, column, numRows, numColumns);
                    }
                }
                row -= 2;
                column += 2;
            } while ((row >= 0) && (column < numColumns));
            row += 1;
            column += 3;
            do {
                if ((row >= 0) && (column < numColumns) && !m_readMappingMatrix->Get(column, row)) {
                    if (resultOffset < (*result).GetSize() ) {
                        (*result)[resultOffset++] = (FX_BYTE) ReadUtah(row, column, numRows, numColumns);
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
FX_BOOL CBC_DataMatrixBitMatrixParser::ReadModule(FX_INT32 row, FX_INT32 column, FX_INT32 numRows, FX_INT32 numColumns)
{
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
FX_INT32 CBC_DataMatrixBitMatrixParser::ReadUtah(FX_INT32 row, FX_INT32 column, FX_INT32 numRows, FX_INT32 numColumns)
{
    FX_INT32 currentByte = 0;
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
FX_INT32 CBC_DataMatrixBitMatrixParser::ReadCorner1(FX_INT32 numRows, FX_INT32 numColumns)
{
    FX_INT32 currentByte = 0;
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
FX_INT32 CBC_DataMatrixBitMatrixParser::ReadCorner2(FX_INT32 numRows, FX_INT32 numColumns)
{
    FX_INT32 currentByte = 0;
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
FX_INT32 CBC_DataMatrixBitMatrixParser::ReadCorner3(FX_INT32 numRows, FX_INT32 numColumns)
{
    FX_INT32 currentByte = 0;
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
FX_INT32 CBC_DataMatrixBitMatrixParser::ReadCorner4(FX_INT32 numRows, FX_INT32 numColumns)
{
    FX_INT32 currentByte = 0;
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
CBC_CommonBitMatrix *CBC_DataMatrixBitMatrixParser::ExtractDataRegion(CBC_CommonBitMatrix *bitMatrix , FX_INT32 &e)
{
    FX_INT32 symbolSizeRows = m_version->GetSymbolSizeRows();
    FX_INT32 symbolSizeColumns = m_version->GetSymbolSizeColumns();
    if (bitMatrix->GetHeight() != symbolSizeRows) {
        e = BCExceptionCanNotCallGetDimensionOnNonSquareMatrix;
        return NULL;
    }
    FX_INT32 dataRegionSizeRows = m_version->GetDataRegionSizeRows();
    FX_INT32 dataRegionSizeColumns = m_version->GetDataRegionSizeColumns();
    FX_INT32 numDataRegionsRow = symbolSizeRows / dataRegionSizeRows;
    FX_INT32 numDataRegionsColumn = symbolSizeColumns / dataRegionSizeColumns;
    FX_INT32 sizeDataRegionRow = numDataRegionsRow * dataRegionSizeRows;
    FX_INT32 sizeDataRegionColumn = numDataRegionsColumn * dataRegionSizeColumns;
    CBC_CommonBitMatrix *bitMatrixWithoutAlignment = FX_NEW CBC_CommonBitMatrix();
    bitMatrixWithoutAlignment->Init(sizeDataRegionColumn, sizeDataRegionRow);
    FX_INT32 dataRegionRow;
    for (dataRegionRow = 0; dataRegionRow < numDataRegionsRow; ++dataRegionRow) {
        FX_INT32 dataRegionRowOffset = dataRegionRow * dataRegionSizeRows;
        FX_INT32 dataRegionColumn;
        for (dataRegionColumn = 0; dataRegionColumn < numDataRegionsColumn; ++dataRegionColumn) {
            FX_INT32 dataRegionColumnOffset = dataRegionColumn * dataRegionSizeColumns;
            FX_INT32 i;
            for (i = 0; i < dataRegionSizeRows; ++i) {
                FX_INT32 readRowOffset = dataRegionRow * (dataRegionSizeRows + 2) + 1 + i;
                FX_INT32 writeRowOffset = dataRegionRowOffset + i;
                FX_INT32 j;
                for (j = 0; j < dataRegionSizeColumns; ++j) {
                    FX_INT32 readColumnOffset = dataRegionColumn * (dataRegionSizeColumns + 2) + 1 + j;
                    if (bitMatrix->Get(readColumnOffset, readRowOffset)) {
                        FX_INT32 writeColumnOffset = dataRegionColumnOffset + j;
                        bitMatrixWithoutAlignment->Set(writeColumnOffset, writeRowOffset);
                    }
                }
            }
        }
    }
    return bitMatrixWithoutAlignment;
}
