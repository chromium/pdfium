// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2011 ZXing authors
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
#include "include/BC_PDF417BarcodeRow.h"
#include "include/BC_PDF417BarcodeMatrix.h"
CBC_BarcodeMatrix::CBC_BarcodeMatrix(FX_INT32 height, FX_INT32 width)
{
    m_matrix.SetSize(height + 2);
    for (FX_INT32 i = 0, matrixLength = m_matrix.GetSize(); i < matrixLength; i++) {
        m_matrix[i] = FX_NEW CBC_BarcodeRow((width + 4) * 17 + 1);
    }
    m_width = width * 17;
    m_height = height + 2;
    m_currentRow = 0;
    m_outHeight = 0;
    m_outWidth = 0;
}
CBC_BarcodeMatrix::~CBC_BarcodeMatrix()
{
    for (FX_INT32 i = 0; i < m_matrix.GetSize(); i++) {
        delete (CBC_BarcodeRow*)m_matrix.GetAt(i);
    }
    m_matrix.RemoveAll();
    m_matrixOut.RemoveAll();
}
void CBC_BarcodeMatrix::set(FX_INT32 x, FX_INT32 y, FX_BYTE value)
{
    ((CBC_BarcodeRow*)m_matrix[y])->set(x, value);
}
void CBC_BarcodeMatrix::setMatrix(FX_INT32 x, FX_INT32 y, FX_BOOL black)
{
    set(x, y, (FX_BYTE) (black ? 1 : 0));
}
void CBC_BarcodeMatrix::startRow()
{
    ++m_currentRow;
}
CBC_BarcodeRow* CBC_BarcodeMatrix::getCurrentRow()
{
    return (CBC_BarcodeRow*)m_matrix[m_currentRow];
}
FX_INT32 CBC_BarcodeMatrix::getWidth()
{
    return m_outWidth;
}
FX_INT32 CBC_BarcodeMatrix::getHeight()
{
    return m_outHeight;
}
CFX_ByteArray& CBC_BarcodeMatrix::getMatrix()
{
    return getScaledMatrix(1, 1);
}
CFX_ByteArray& CBC_BarcodeMatrix::getScaledMatrix(FX_INT32 scale)
{
    return getScaledMatrix(scale, scale);
}
CFX_ByteArray& CBC_BarcodeMatrix::getScaledMatrix(FX_INT32 xScale, FX_INT32 yScale)
{
    FX_INT32 yMax = m_height * yScale;
    CFX_ByteArray bytearray;
    bytearray.Copy(((CBC_BarcodeRow*)m_matrix[0])->getScaledRow(xScale));
    FX_INT32 xMax = bytearray.GetSize();
    m_matrixOut.SetSize(xMax * yMax);
    m_outWidth = xMax;
    m_outHeight = yMax;
    FX_INT32 k = 0;
    for (FX_INT32 i = 0; i < yMax; i++) {
        if (i != 0) {
            bytearray.Copy(((CBC_BarcodeRow*)m_matrix[i / yScale])->getScaledRow(xScale));
        }
        k = i * xMax;
        for (FX_INT32 l = 0; l < xMax; l++) {
            m_matrixOut[k + l] = bytearray.GetAt(l);
        }
    }
    return m_matrixOut;
}
