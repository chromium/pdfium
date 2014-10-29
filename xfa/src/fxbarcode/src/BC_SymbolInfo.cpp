// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_Encoder.h"
#include "include/BC_Dimension.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_SymbolShapeHint.h"
#include "include/BC_SymbolInfo.h"
#include "include/BC_DataMatrixSymbolInfo144.h"
#define  SYMBOLS_COUNT  30
CBC_SymbolInfo* CBC_SymbolInfo::m_PROD_SYMBOLS[30] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
CBC_SymbolInfo* CBC_SymbolInfo::m_symbols[30] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
void CBC_SymbolInfo::Initialize()
{
    m_PROD_SYMBOLS[0] = FX_NEW CBC_SymbolInfo(FALSE, 3, 5, 8, 8, 1);
    m_PROD_SYMBOLS[1] = FX_NEW CBC_SymbolInfo(FALSE, 5, 7, 10, 10, 1);
    m_PROD_SYMBOLS[2] = FX_NEW CBC_SymbolInfo(TRUE, 5, 7, 16, 6, 1);
    m_PROD_SYMBOLS[3] = FX_NEW CBC_SymbolInfo(FALSE, 8, 10, 12, 12, 1);
    m_PROD_SYMBOLS[4] = FX_NEW CBC_SymbolInfo(TRUE, 10, 11, 14, 6, 2);
    m_PROD_SYMBOLS[5] = FX_NEW CBC_SymbolInfo(FALSE, 12, 12, 14, 14, 1);
    m_PROD_SYMBOLS[6] = FX_NEW CBC_SymbolInfo(TRUE, 16, 14, 24, 10, 1);
    m_PROD_SYMBOLS[7] = FX_NEW CBC_SymbolInfo(FALSE, 18, 14, 16, 16, 1);
    m_PROD_SYMBOLS[8] = FX_NEW CBC_SymbolInfo(FALSE, 22, 18, 18, 18, 1);
    m_PROD_SYMBOLS[9] = FX_NEW CBC_SymbolInfo(TRUE, 22, 18, 16, 10, 2);
    m_PROD_SYMBOLS[10] = FX_NEW CBC_SymbolInfo(FALSE, 30, 20, 20, 20, 1);
    m_PROD_SYMBOLS[11] = FX_NEW CBC_SymbolInfo(TRUE, 32, 24, 16, 14, 2);
    m_PROD_SYMBOLS[12] = FX_NEW CBC_SymbolInfo(FALSE, 36, 24, 22, 22, 1);
    m_PROD_SYMBOLS[13] = FX_NEW CBC_SymbolInfo(FALSE, 44, 28, 24, 24, 1);
    m_PROD_SYMBOLS[14] = FX_NEW CBC_SymbolInfo(TRUE, 49, 28, 22, 14, 2);
    m_PROD_SYMBOLS[15] = FX_NEW CBC_SymbolInfo(FALSE, 62, 36, 14, 14, 4);
    m_PROD_SYMBOLS[16] = FX_NEW CBC_SymbolInfo(FALSE, 86, 42, 16, 16, 4);
    m_PROD_SYMBOLS[17] = FX_NEW CBC_SymbolInfo(FALSE, 114, 48, 18, 18, 4);
    m_PROD_SYMBOLS[18] = FX_NEW CBC_SymbolInfo(FALSE, 144, 56, 20, 20, 4);
    m_PROD_SYMBOLS[19] = FX_NEW CBC_SymbolInfo(FALSE, 174, 68, 22, 22, 4);
    m_PROD_SYMBOLS[20] = FX_NEW CBC_SymbolInfo(FALSE, 204, 84, 24, 24, 4, 102, 42);
    m_PROD_SYMBOLS[21] = FX_NEW CBC_SymbolInfo(FALSE, 280, 112, 14, 14, 16, 140, 56);
    m_PROD_SYMBOLS[22] = FX_NEW CBC_SymbolInfo(FALSE, 368, 144, 16, 16, 16, 92, 36);
    m_PROD_SYMBOLS[23] = FX_NEW CBC_SymbolInfo(FALSE, 456, 192, 18, 18, 16, 114, 48);
    m_PROD_SYMBOLS[24] = FX_NEW CBC_SymbolInfo(FALSE, 576, 224, 20, 20, 16, 144, 56);
    m_PROD_SYMBOLS[25] = FX_NEW CBC_SymbolInfo(FALSE, 696, 272, 22, 22, 16, 174, 68);
    m_PROD_SYMBOLS[26] = FX_NEW CBC_SymbolInfo(FALSE, 816, 336, 24, 24, 16, 136, 56);
    m_PROD_SYMBOLS[27] = FX_NEW CBC_SymbolInfo(FALSE, 1050, 408, 18, 18, 36, 175, 68);
    m_PROD_SYMBOLS[28] = FX_NEW CBC_SymbolInfo(FALSE, 1304, 496, 20, 20, 36, 163, 62);
    m_PROD_SYMBOLS[29] = FX_NEW CBC_DataMatrixSymbolInfo144();
    for (FX_INT32 i = 0; i < SYMBOLS_COUNT; i++) {
        m_symbols[i] = m_PROD_SYMBOLS[i];
    }
}
void CBC_SymbolInfo::Finalize()
{
    for (FX_INT32 i = 0; i < SYMBOLS_COUNT; i++) {
        delete m_PROD_SYMBOLS[i];
        m_PROD_SYMBOLS[i] = NULL;
        m_symbols[i] = NULL;
    }
}
CBC_SymbolInfo::CBC_SymbolInfo(FX_BOOL rectangular, FX_INT32 dataCapacity, FX_INT32 errorCodewords, FX_INT32 matrixWidth, FX_INT32 matrixHeight, FX_INT32 dataRegions)
{
    m_rectangular = rectangular;
    m_dataCapacity = dataCapacity;
    m_errorCodewords = errorCodewords;
    m_matrixWidth = matrixWidth;
    m_matrixHeight = matrixHeight;
    m_dataRegions = dataRegions;
    m_rsBlockData = dataCapacity;
    m_rsBlockError = errorCodewords;
}
CBC_SymbolInfo::CBC_SymbolInfo(FX_BOOL rectangular, FX_INT32 dataCapacity, FX_INT32 errorCodewords, FX_INT32 matrixWidth, FX_INT32 matrixHeight, FX_INT32 dataRegions,
                               FX_INT32 rsBlockData, FX_INT32 rsBlockError)
{
    m_rectangular = rectangular;
    m_dataCapacity = dataCapacity;
    m_errorCodewords = errorCodewords;
    m_matrixWidth = matrixWidth;
    m_matrixHeight = matrixHeight;
    m_dataRegions = dataRegions;
    m_rsBlockData = rsBlockData;
    m_rsBlockError = rsBlockError;
}
CBC_SymbolInfo::~CBC_SymbolInfo()
{
}

CBC_SymbolInfo* CBC_SymbolInfo::lookup(FX_INT32 dataCodewords, FX_INT32 &e)
{
    return lookup(dataCodewords, FORCE_NONE, TRUE, e);
}
CBC_SymbolInfo* CBC_SymbolInfo::lookup(FX_INT32 dataCodewords, SymbolShapeHint shape, FX_INT32 &e)
{
    return lookup(dataCodewords, shape, TRUE, e);
}
CBC_SymbolInfo* CBC_SymbolInfo::lookup(FX_INT32 dataCodewords, FX_BOOL allowRectangular, FX_BOOL fail, FX_INT32 &e)
{
    SymbolShapeHint shape = allowRectangular ? FORCE_NONE : FORCE_SQUARE;
    return lookup(dataCodewords, shape, fail, e);
}
CBC_SymbolInfo* CBC_SymbolInfo::lookup(FX_INT32 dataCodewords, SymbolShapeHint shape, FX_BOOL fail, FX_INT32 &e)
{
    return lookup(dataCodewords, shape, NULL, NULL, fail, e);
}
CBC_SymbolInfo* CBC_SymbolInfo::lookup(FX_INT32 dataCodewords, SymbolShapeHint shape, CBC_Dimension* minSize, CBC_Dimension* maxSize, FX_BOOL fail, FX_INT32 &e)
{
    for (FX_INT32 i = 0; i < SYMBOLS_COUNT; i++) {
        CBC_SymbolInfo* symbol = m_symbols[i];
        if (shape == FORCE_SQUARE && symbol->m_rectangular) {
            continue;
        }
        if (shape == FORCE_RECTANGLE && !symbol->m_rectangular) {
            continue;
        }
        if (minSize != NULL && (symbol->getSymbolWidth(e) < minSize->getWidth() || symbol->getSymbolHeight(e) < minSize->getHeight())) {
            BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
            continue;
        }
        if (maxSize != NULL && (symbol->getSymbolWidth(e) > maxSize->getWidth() || symbol->getSymbolHeight(e) > maxSize->getHeight())) {
            BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
            continue;
        }
        if (dataCodewords <= symbol->m_dataCapacity) {
            return symbol;
        }
    }
    if (fail) {
        e = BCExceptionIllegalDataCodewords;
        return NULL;
    }
    return NULL;
}
FX_INT32 CBC_SymbolInfo::getHorizontalDataRegions(FX_INT32 &e)
{
    switch (m_dataRegions) {
        case 1:
            return 1;
        case 2:
            return 2;
        case 4:
            return 2;
        case 16:
            return 4;
        case 36:
            return 6;
        default:
            e = BCExceptionCannotHandleThisNumberOfDataRegions;
            return 0;
    }
}
FX_INT32 CBC_SymbolInfo::getVerticalDataRegions(FX_INT32 &e)
{
    switch (m_dataRegions) {
        case 1:
            return 1;
        case 2:
            return 1;
        case 4:
            return 2;
        case 16:
            return 4;
        case 36:
            return 6;
        default:
            e = BCExceptionCannotHandleThisNumberOfDataRegions;
            return 0;
    }
}
FX_INT32 CBC_SymbolInfo::getSymbolDataWidth(FX_INT32 &e)
{
    return getHorizontalDataRegions(e) * m_matrixWidth;
}
FX_INT32 CBC_SymbolInfo::getSymbolDataHeight(FX_INT32 &e)
{
    return getVerticalDataRegions(e) * m_matrixHeight;
}
FX_INT32 CBC_SymbolInfo::getSymbolWidth(FX_INT32 &e)
{
    return getSymbolDataWidth(e) + (getHorizontalDataRegions(e) * 2);
}
FX_INT32 CBC_SymbolInfo::getSymbolHeight(FX_INT32 &e)
{
    return getSymbolDataHeight(e) + (getVerticalDataRegions(e) * 2);
}
FX_INT32 CBC_SymbolInfo::getCodewordCount()
{
    return m_dataCapacity + m_errorCodewords;
}
FX_INT32 CBC_SymbolInfo::getInterleavedBlockCount()
{
    return m_dataCapacity / m_rsBlockData;
}
FX_INT32 CBC_SymbolInfo::getDataLengthForInterleavedBlock(FX_INT32 index)
{
    return m_rsBlockData;
}
FX_INT32 CBC_SymbolInfo::getErrorLengthForInterleavedBlock(FX_INT32 index)
{
    return m_rsBlockError;
}
CFX_WideString CBC_SymbolInfo::toString(FX_INT32 &e)
{
    CFX_WideString sb;
    sb += (FX_LPWSTR)(m_rectangular ? "Rectangular Symbol:" : "Square Symbol:");
    sb += (FX_LPWSTR)" data region ";
    sb += m_matrixWidth;
    sb += (FX_WCHAR)'x';
    sb += m_matrixHeight;
    sb += (FX_LPWSTR)", symbol size ";
    sb += getSymbolWidth(e);
    BC_EXCEPTION_CHECK_ReturnValue(e, (FX_LPWSTR)"");
    sb += (FX_WCHAR)'x';
    sb += getSymbolHeight(e);
    BC_EXCEPTION_CHECK_ReturnValue(e, (FX_LPWSTR)"");
    sb += (FX_LPWSTR)", symbol data size ";
    sb += getSymbolDataWidth(e);
    BC_EXCEPTION_CHECK_ReturnValue(e, (FX_LPWSTR)"");
    sb += (FX_WCHAR)'x';
    sb += getSymbolDataHeight(e);
    BC_EXCEPTION_CHECK_ReturnValue(e, (FX_LPWSTR)"");
    sb += (FX_LPWSTR)", codewords ";
    sb += m_dataCapacity;
    sb += (FX_WCHAR)'+';
    sb += m_errorCodewords;
    return sb;
}
