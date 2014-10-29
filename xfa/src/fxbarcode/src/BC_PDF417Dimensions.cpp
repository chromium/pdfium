// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_PDF417Dimensions.h"
CBC_Dimensions::CBC_Dimensions(FX_INT32 minCols, FX_INT32 maxCols, FX_INT32 minRows, FX_INT32 maxRows)
{
    m_minCols = minCols;
    m_maxCols = maxCols;
    m_minRows = minRows;
    m_maxRows = maxRows;
}
CBC_Dimensions::~CBC_Dimensions()
{
}
FX_INT32 CBC_Dimensions::getMinCols()
{
    return m_minCols;
}
FX_INT32 CBC_Dimensions::getMaxCols()
{
    return m_maxCols;
}
FX_INT32 CBC_Dimensions::getMinRows()
{
    return m_minRows;
}
FX_INT32 CBC_Dimensions::getMaxRows()
{
    return m_maxRows;
}
