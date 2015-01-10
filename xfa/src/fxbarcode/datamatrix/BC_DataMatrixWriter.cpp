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

#include "../barcode.h"
#include "../BC_Writer.h"
#include "../BC_TwoDimWriter.h"
#include "../BC_Dimension.h"
#include "../BC_BinaryBitmap.h"
#include "../BC_UtilCodingConvert.h"
#include "../common/BC_CommonBitMatrix.h"
#include "../common/BC_CommonByteMatrix.h"
#include "BC_Encoder.h"
#include "BC_DefaultPlacement.h"
#include "BC_SymbolShapeHint.h"
#include "BC_SymbolInfo.h"
#include "BC_DataMatrixSymbolInfo144.h"
#include "BC_ErrorCorrection.h"
#include "BC_EncoderContext.h"
#include "BC_C40Encoder.h"
#include "BC_TextEncoder.h"
#include "BC_X12Encoder.h"
#include "BC_EdifactEncoder.h"
#include "BC_Base256Encoder.h"
#include "BC_ASCIIEncoder.h"
#include "BC_HighLevelEncoder.h"
#include "BC_DataMatrixWriter.h"
CBC_DataMatrixWriter::CBC_DataMatrixWriter()
{
}
CBC_DataMatrixWriter::~CBC_DataMatrixWriter()
{
}
FX_BOOL CBC_DataMatrixWriter::SetErrorCorrectionLevel (FX_INT32 level)
{
    m_iCorrectLevel = level;
    return TRUE;
}
FX_BYTE* CBC_DataMatrixWriter::Encode(const CFX_WideString &contents, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e)
{
    if (outWidth < 0 || outHeight < 0) {
        e = BCExceptionHeightAndWidthMustBeAtLeast1;
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    CBC_SymbolShapeHint::SymbolShapeHint shape = CBC_SymbolShapeHint::FORCE_SQUARE;
    CBC_Dimension* minSize = NULL;
    CBC_Dimension* maxSize = NULL;
    CFX_WideString ecLevel;
    CFX_WideString encoded = CBC_HighLevelEncoder::encodeHighLevel(contents, ecLevel, shape, minSize, maxSize, e);
    BC_EXCEPTION_CHECK_ReturnValue(e,  NULL);
    CBC_SymbolInfo* symbolInfo = CBC_SymbolInfo::lookup(encoded.GetLength(), shape, minSize, maxSize, TRUE, e);
    BC_EXCEPTION_CHECK_ReturnValue(e,  NULL);
    CFX_WideString codewords = CBC_ErrorCorrection::encodeECC200(encoded, symbolInfo, e);
    BC_EXCEPTION_CHECK_ReturnValue(e,  NULL);
    CBC_DefaultPlacement* placement = FX_NEW CBC_DefaultPlacement(codewords, symbolInfo->getSymbolDataWidth(e), symbolInfo->getSymbolDataHeight(e));
    BC_EXCEPTION_CHECK_ReturnValue(e,  NULL);
    placement->place();
    CBC_CommonByteMatrix* bytematrix = encodeLowLevel(placement, symbolInfo, e);
    BC_EXCEPTION_CHECK_ReturnValue(e,  NULL);
    outWidth = bytematrix->GetWidth();
    outHeight = bytematrix->GetHeight();
    FX_BYTE* result = FX_Alloc(FX_BYTE, outWidth * outHeight);
    FXSYS_memcpy32(result, bytematrix->GetArray(), outWidth * outHeight);
    delete bytematrix;
    delete placement;
    return result;
}
FX_BYTE *CBC_DataMatrixWriter::Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e)
{
    return NULL;
}
FX_BYTE *CBC_DataMatrixWriter::Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 hints, FX_INT32 &e)
{
    return NULL;
}
CBC_CommonByteMatrix* CBC_DataMatrixWriter::encodeLowLevel(CBC_DefaultPlacement* placement, CBC_SymbolInfo* symbolInfo, FX_INT32 &e)
{
    FX_INT32 symbolWidth = symbolInfo->getSymbolDataWidth(e);
    BC_EXCEPTION_CHECK_ReturnValue(e,  NULL);
    FX_INT32 symbolHeight = symbolInfo->getSymbolDataHeight(e);
    BC_EXCEPTION_CHECK_ReturnValue(e,  NULL);
    CBC_CommonByteMatrix* matrix = FX_NEW CBC_CommonByteMatrix(symbolInfo->getSymbolWidth(e), symbolInfo->getSymbolHeight(e));
    BC_EXCEPTION_CHECK_ReturnValue(e,  NULL);
    matrix->Init();
    FX_INT32 matrixY = 0;
    for (FX_INT32 y = 0; y < symbolHeight; y++) {
        FX_INT32 matrixX;
        if ((y % symbolInfo->m_matrixHeight) == 0) {
            matrixX = 0;
            for (FX_INT32 x = 0; x < symbolInfo->getSymbolWidth(e); x++) {
                matrix->Set(matrixX, matrixY, (x % 2) == 0);
                matrixX++;
            }
            matrixY++;
        }
        matrixX = 0;
        for (FX_INT32 x = 0; x < symbolWidth; x++) {
            if ((x % symbolInfo->m_matrixWidth) == 0) {
                matrix->Set(matrixX, matrixY, TRUE);
                matrixX++;
            }
            matrix->Set(matrixX, matrixY, placement->getBit(x, y));
            matrixX++;
            if ((x % symbolInfo->m_matrixWidth) == symbolInfo->m_matrixWidth - 1) {
                matrix->Set(matrixX, matrixY, (y % 2) == 0);
                matrixX++;
            }
        }
        matrixY++;
        if ((y % symbolInfo->m_matrixHeight) == symbolInfo->m_matrixHeight - 1) {
            matrixX = 0;
            for (FX_INT32 x = 0; x < symbolInfo->getSymbolWidth(e); x++) {
                matrix->Set(matrixX, matrixY, TRUE);
                matrixX++;
            }
            matrixY++;
        }
    }
    return matrix;
}
