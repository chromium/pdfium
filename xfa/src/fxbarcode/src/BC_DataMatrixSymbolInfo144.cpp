// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_Encoder.h"
#include "include/BC_SymbolShapeHint.h"
#include "include/BC_SymbolInfo.h"
#include "include/BC_DataMatrixSymbolInfo144.h"
CBC_DataMatrixSymbolInfo144::CBC_DataMatrixSymbolInfo144() : CBC_SymbolInfo(FALSE, 1558, 620, 22, 22, 36)
{
    m_rsBlockData = -1;
    m_rsBlockError = 62;
}
CBC_DataMatrixSymbolInfo144::~CBC_DataMatrixSymbolInfo144()
{
}
FX_INT32 CBC_DataMatrixSymbolInfo144::getInterleavedBlockCount()
{
    return 10;
}
FX_INT32 CBC_DataMatrixSymbolInfo144getDataLengthForInterleavedBlock(FX_INT32 index)
{
    return (index <= 8) ? 156 : 155;
}
