// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_PDF417BarcodeMetadata.h"
CBC_BarcodeMetadata::CBC_BarcodeMetadata(FX_INT32 columnCount, FX_INT32 rowCountUpperPart, FX_INT32 rowCountLowerPart, FX_INT32 errorCorrectionLevel)
{
    m_columnCount = columnCount;
    m_rowCountUpperPart = rowCountUpperPart;
    m_rowCountLowerPart = rowCountLowerPart;
    m_errorCorrectionLevel = errorCorrectionLevel;
    m_rowCount = m_rowCountUpperPart + m_rowCountLowerPart;
}
CBC_BarcodeMetadata::~CBC_BarcodeMetadata()
{
}
FX_INT32 CBC_BarcodeMetadata::getColumnCount()
{
    return m_columnCount;
}
FX_INT32 CBC_BarcodeMetadata::getErrorCorrectionLevel()
{
    return m_errorCorrectionLevel;
}
FX_INT32 CBC_BarcodeMetadata::getRowCount()
{
    return m_rowCount;
}
FX_INT32 CBC_BarcodeMetadata::getRowCountUpperPart()
{
    return m_rowCountUpperPart;
}
FX_INT32 CBC_BarcodeMetadata::getRowCountLowerPart()
{
    return m_rowCountLowerPart;
}
