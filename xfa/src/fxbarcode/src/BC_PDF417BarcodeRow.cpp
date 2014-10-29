// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_PDF417BarcodeRow.h"
CBC_BarcodeRow::CBC_BarcodeRow(FX_INT32 width)
{
    m_row.SetSize(width);
    m_currentLocation = 0;
}
CBC_BarcodeRow::~CBC_BarcodeRow()
{
    m_output.RemoveAll();
    m_row.RemoveAll();
}
void CBC_BarcodeRow::set(FX_INT32 x, FX_BYTE value)
{
    m_row.SetAt(x, value);
}
void CBC_BarcodeRow::set(FX_INT32 x, FX_BOOL black)
{
    m_row.SetAt(x, (FX_BYTE) (black ? 1 : 0));
}
void CBC_BarcodeRow::addBar(FX_BOOL black, FX_INT32 width)
{
    for (FX_INT32 ii = 0; ii < width; ii++) {
        set(m_currentLocation++, black);
    }
}
CFX_ByteArray& CBC_BarcodeRow::getRow()
{
    return m_row;
}
CFX_ByteArray& CBC_BarcodeRow::getScaledRow(FX_INT32 scale)
{
    m_output.SetSize(m_row.GetSize() * scale);
    for (FX_INT32 i = 0; i < m_output.GetSize(); i++) {
        m_output[i] = (m_row[i / scale]);
    }
    return m_output;
}
