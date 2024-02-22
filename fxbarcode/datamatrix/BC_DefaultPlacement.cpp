// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2006 Jeremias Maerki.
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

#include "fxbarcode/datamatrix/BC_DefaultPlacement.h"

#include <stdint.h>

#include <utility>

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_2d_size.h"
#include "fxbarcode/datamatrix/BC_Encoder.h"

namespace {

size_t GetIndex(size_t col, size_t row, size_t num_cols) {
  return row * num_cols + col;
}

}  // namespace

CBC_DefaultPlacement::CBC_DefaultPlacement(WideString codewords,
                                           int32_t numcols,
                                           int32_t numrows)
    : m_codewords(std::move(codewords)),
      m_numrows(numrows),
      m_numcols(numcols),
      m_bits(Fx2DSizeOrDie(numcols, numrows), 2) {
  CHECK_GT(m_numrows, 0);
  CHECK_GT(m_numcols, 0);
  Init();
}

CBC_DefaultPlacement::~CBC_DefaultPlacement() = default;

bool CBC_DefaultPlacement::GetBit(int32_t col, int32_t row) const {
  CHECK_GE(col, 0);
  CHECK_GE(row, 0);
  CHECK_LT(col, m_numcols);
  CHECK_LT(row, m_numrows);
  return m_bits[GetIndex(col, row, m_numcols)] == 1;
}

void CBC_DefaultPlacement::SetBit(int32_t col, int32_t row, bool bit) {
  DCHECK_GE(col, 0);
  DCHECK_GE(row, 0);
  DCHECK_LT(col, m_numcols);
  DCHECK_LT(row, m_numrows);
  m_bits[GetIndex(col, row, m_numcols)] = bit ? 1 : 0;
}

bool CBC_DefaultPlacement::HasBit(int32_t col, int32_t row) const {
  DCHECK_GE(col, 0);
  DCHECK_GE(row, 0);
  DCHECK_LT(col, m_numcols);
  DCHECK_LT(row, m_numrows);
  return m_bits[GetIndex(col, row, m_numcols)] != 2;
}

void CBC_DefaultPlacement::Init() {
  int32_t pos = 0;
  int32_t row = 4;
  int32_t col = 0;
  do {
    if ((row == m_numrows) && (col == 0)) {
      SetCorner1(pos++);
    }
    if ((row == m_numrows - 2) && (col == 0) && ((m_numcols % 4) != 0)) {
      SetCorner2(pos++);
    }
    if ((row == m_numrows - 2) && (col == 0) && (m_numcols % 8 == 4)) {
      SetCorner3(pos++);
    }
    if ((row == m_numrows + 4) && (col == 2) && ((m_numcols % 8) == 0)) {
      SetCorner4(pos++);
    }
    do {
      if ((row < m_numrows) && (col >= 0) && !HasBit(col, row)) {
        SetUtah(row, col, pos++);
      }
      row -= 2;
      col += 2;
    } while (row >= 0 && (col < m_numcols));
    row++;
    col += 3;
    do {
      if ((row >= 0) && (col < m_numcols) && !HasBit(col, row)) {
        SetUtah(row, col, pos++);
      }
      row += 2;
      col -= 2;
    } while ((row < m_numrows) && (col >= 0));
    row += 3;
    col++;
  } while ((row < m_numrows) || (col < m_numcols));
  if (!HasBit(m_numcols - 1, m_numrows - 1)) {
    SetBit(m_numcols - 1, m_numrows - 1, true);
    SetBit(m_numcols - 2, m_numrows - 2, true);
  }
}

void CBC_DefaultPlacement::SetModule(int32_t row,
                                     int32_t col,
                                     int32_t pos,
                                     int32_t bit) {
  if (row < 0) {
    row += m_numrows;
    col += 4 - ((m_numrows + 4) % 8);
  }
  if (col < 0) {
    col += m_numcols;
    row += 4 - ((m_numcols + 4) % 8);
  }
  int32_t v = m_codewords[pos];
  v &= 1 << (8 - bit);
  SetBit(col, row, v != 0);
}

void CBC_DefaultPlacement::SetUtah(int32_t row, int32_t col, int32_t pos) {
  SetModule(row - 2, col - 2, pos, 1);
  SetModule(row - 2, col - 1, pos, 2);
  SetModule(row - 1, col - 2, pos, 3);
  SetModule(row - 1, col - 1, pos, 4);
  SetModule(row - 1, col, pos, 5);
  SetModule(row, col - 2, pos, 6);
  SetModule(row, col - 1, pos, 7);
  SetModule(row, col, pos, 8);
}

void CBC_DefaultPlacement::SetCorner1(int32_t pos) {
  SetModule(m_numrows - 1, 0, pos, 1);
  SetModule(m_numrows - 1, 1, pos, 2);
  SetModule(m_numrows - 1, 2, pos, 3);
  SetModule(0, m_numcols - 2, pos, 4);
  SetModule(0, m_numcols - 1, pos, 5);
  SetModule(1, m_numcols - 1, pos, 6);
  SetModule(2, m_numcols - 1, pos, 7);
  SetModule(3, m_numcols - 1, pos, 8);
}

void CBC_DefaultPlacement::SetCorner2(int32_t pos) {
  SetModule(m_numrows - 3, 0, pos, 1);
  SetModule(m_numrows - 2, 0, pos, 2);
  SetModule(m_numrows - 1, 0, pos, 3);
  SetModule(0, m_numcols - 4, pos, 4);
  SetModule(0, m_numcols - 3, pos, 5);
  SetModule(0, m_numcols - 2, pos, 6);
  SetModule(0, m_numcols - 1, pos, 7);
  SetModule(1, m_numcols - 1, pos, 8);
}

void CBC_DefaultPlacement::SetCorner3(int32_t pos) {
  SetModule(m_numrows - 3, 0, pos, 1);
  SetModule(m_numrows - 2, 0, pos, 2);
  SetModule(m_numrows - 1, 0, pos, 3);
  SetModule(0, m_numcols - 2, pos, 4);
  SetModule(0, m_numcols - 1, pos, 5);
  SetModule(1, m_numcols - 1, pos, 6);
  SetModule(2, m_numcols - 1, pos, 7);
  SetModule(3, m_numcols - 1, pos, 8);
}

void CBC_DefaultPlacement::SetCorner4(int32_t pos) {
  SetModule(m_numrows - 1, 0, pos, 1);
  SetModule(m_numrows - 1, m_numcols - 1, pos, 2);
  SetModule(0, m_numcols - 3, pos, 3);
  SetModule(0, m_numcols - 2, pos, 4);
  SetModule(0, m_numcols - 1, pos, 5);
  SetModule(1, m_numcols - 3, pos, 6);
  SetModule(1, m_numcols - 2, pos, 7);
  SetModule(1, m_numcols - 1, pos, 8);
}
