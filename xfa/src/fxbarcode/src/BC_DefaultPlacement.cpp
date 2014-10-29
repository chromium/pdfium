// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_Encoder.h"
#include "include/BC_DefaultPlacement.h"
CBC_DefaultPlacement::CBC_DefaultPlacement(CFX_WideString codewords, FX_INT32 numcols, FX_INT32 numrows)
{
    m_codewords = codewords;
    m_numcols = numcols;
    m_numrows = numrows;
    m_bits.SetSize(numcols * numrows);
    for (FX_INT32 i = 0; i < numcols * numrows; i++) {
        m_bits[i] = (FX_BYTE) 2;
    }
}
CBC_DefaultPlacement::~CBC_DefaultPlacement()
{
    m_bits.RemoveAll();
}
FX_INT32 CBC_DefaultPlacement::getNumrows()
{
    return m_numrows;
}
FX_INT32 CBC_DefaultPlacement::getNumcols()
{
    return m_numcols;
}
CFX_ByteArray& CBC_DefaultPlacement::getBits()
{
    return m_bits;
}
FX_BOOL CBC_DefaultPlacement::getBit(FX_INT32 col, FX_INT32 row)
{
    return m_bits[row * m_numcols + col] == 1;
}
void CBC_DefaultPlacement::setBit(FX_INT32 col, FX_INT32 row, FX_BOOL bit)
{
    m_bits[row * m_numcols + col] = bit ? (FX_BYTE) 1 : (FX_BYTE) 0;
}
FX_BOOL CBC_DefaultPlacement::hasBit(FX_INT32 col, FX_INT32 row)
{
    return m_bits[row * m_numcols + col] != 2;
}
void CBC_DefaultPlacement::place()
{
    FX_INT32 pos = 0;
    FX_INT32 row = 4;
    FX_INT32 col = 0;
    do {
        if ((row == m_numrows) && (col == 0)) {
            corner1(pos++);
        }
        if ((row == m_numrows - 2) && (col == 0) && ((m_numcols % 4) != 0)) {
            corner2(pos++);
        }
        if ((row == m_numrows - 2) && (col == 0) && (m_numcols % 8 == 4)) {
            corner3(pos++);
        }
        if ((row == m_numrows + 4) && (col == 2) && ((m_numcols % 8) == 0)) {
            corner4(pos++);
        }
        do {
            if ((row < m_numrows) && (col >= 0) && !hasBit(col, row)) {
                utah(row, col, pos++);
            }
            row -= 2;
            col += 2;
        } while (row >= 0 && (col < m_numcols));
        row++;
        col += 3;
        do {
            if ((row >= 0) && (col < m_numcols) && !hasBit(col, row)) {
                utah(row, col, pos++);
            }
            row += 2;
            col -= 2;
        } while ((row < m_numrows) && (col >= 0));
        row += 3;
        col++;
    } while ((row < m_numrows) || (col < m_numcols));
    if (!hasBit(m_numcols - 1, m_numrows - 1)) {
        setBit(m_numcols - 1, m_numrows - 1, TRUE);
        setBit(m_numcols - 2, m_numrows - 2, TRUE);
    }
}
void CBC_DefaultPlacement::module(FX_INT32 row, FX_INT32 col, FX_INT32 pos, FX_INT32 bit)
{
    if (row < 0) {
        row += m_numrows;
        col += 4 - ((m_numrows + 4) % 8);
    }
    if (col < 0) {
        col += m_numcols;
        row += 4 - ((m_numcols + 4) % 8);
    }
    FX_INT32 v = m_codewords.GetAt(pos);
    v &= 1 << (8 - bit);
    setBit(col, row, v != 0);
}
void CBC_DefaultPlacement::utah(FX_INT32 row, FX_INT32 col, FX_INT32 pos)
{
    module(row - 2, col - 2, pos, 1);
    module(row - 2, col - 1, pos, 2);
    module(row - 1, col - 2, pos, 3);
    module(row - 1, col - 1, pos, 4);
    module(row - 1, col, pos, 5);
    module(row, col - 2, pos, 6);
    module(row, col - 1, pos, 7);
    module(row, col, pos, 8);
}
void CBC_DefaultPlacement::corner1(FX_INT32 pos)
{
    module(m_numrows - 1, 0, pos, 1);
    module(m_numrows - 1, 1, pos, 2);
    module(m_numrows - 1, 2, pos, 3);
    module(0, m_numcols - 2, pos, 4);
    module(0, m_numcols - 1, pos, 5);
    module(1, m_numcols - 1, pos, 6);
    module(2, m_numcols - 1, pos, 7);
    module(3, m_numcols - 1, pos, 8);
}
void CBC_DefaultPlacement::corner2(FX_INT32 pos)
{
    module(m_numrows - 3, 0, pos, 1);
    module(m_numrows - 2, 0, pos, 2);
    module(m_numrows - 1, 0, pos, 3);
    module(0, m_numcols - 4, pos, 4);
    module(0, m_numcols - 3, pos, 5);
    module(0, m_numcols - 2, pos, 6);
    module(0, m_numcols - 1, pos, 7);
    module(1, m_numcols - 1, pos, 8);
}
void CBC_DefaultPlacement::corner3(FX_INT32 pos)
{
    module(m_numrows - 3, 0, pos, 1);
    module(m_numrows - 2, 0, pos, 2);
    module(m_numrows - 1, 0, pos, 3);
    module(0, m_numcols - 2, pos, 4);
    module(0, m_numcols - 1, pos, 5);
    module(1, m_numcols - 1, pos, 6);
    module(2, m_numcols - 1, pos, 7);
    module(3, m_numcols - 1, pos, 8);
}
void CBC_DefaultPlacement::corner4(FX_INT32 pos)
{
    module(m_numrows - 1, 0, pos, 1);
    module(m_numrows - 1, m_numcols - 1, pos, 2);
    module(0, m_numcols - 3, pos, 3);
    module(0, m_numcols - 2, pos, 4);
    module(0, m_numcols - 1, pos, 5);
    module(1, m_numcols - 3, pos, 6);
    module(1, m_numcols - 2, pos, 7);
    module(1, m_numcols - 1, pos, 8);
}
