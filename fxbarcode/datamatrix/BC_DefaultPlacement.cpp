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
    : codewords_(std::move(codewords)),
      numrows_(numrows),
      numcols_(numcols),
      bits_(Fx2DSizeOrDie(numcols, numrows), 2) {
  CHECK_GT(numrows_, 0);
  CHECK_GT(numcols_, 0);
  Init();
}

CBC_DefaultPlacement::~CBC_DefaultPlacement() = default;

bool CBC_DefaultPlacement::GetBit(int32_t col, int32_t row) const {
  CHECK_GE(col, 0);
  CHECK_GE(row, 0);
  CHECK_LT(col, numcols_);
  CHECK_LT(row, numrows_);
  return bits_[GetIndex(col, row, numcols_)] == 1;
}

void CBC_DefaultPlacement::SetBit(int32_t col, int32_t row, bool bit) {
  DCHECK_GE(col, 0);
  DCHECK_GE(row, 0);
  DCHECK_LT(col, numcols_);
  DCHECK_LT(row, numrows_);
  bits_[GetIndex(col, row, numcols_)] = bit ? 1 : 0;
}

bool CBC_DefaultPlacement::HasBit(int32_t col, int32_t row) const {
  DCHECK_GE(col, 0);
  DCHECK_GE(row, 0);
  DCHECK_LT(col, numcols_);
  DCHECK_LT(row, numrows_);
  return bits_[GetIndex(col, row, numcols_)] != 2;
}

void CBC_DefaultPlacement::Init() {
  int32_t pos = 0;
  int32_t row = 4;
  int32_t col = 0;
  do {
    if ((row == numrows_) && (col == 0)) {
      SetCorner1(pos++);
    }
    if ((row == numrows_ - 2) && (col == 0) && ((numcols_ % 4) != 0)) {
      SetCorner2(pos++);
    }
    if ((row == numrows_ - 2) && (col == 0) && (numcols_ % 8 == 4)) {
      SetCorner3(pos++);
    }
    if ((row == numrows_ + 4) && (col == 2) && ((numcols_ % 8) == 0)) {
      SetCorner4(pos++);
    }
    do {
      if ((row < numrows_) && (col >= 0) && !HasBit(col, row)) {
        SetUtah(row, col, pos++);
      }
      row -= 2;
      col += 2;
    } while (row >= 0 && (col < numcols_));
    row++;
    col += 3;
    do {
      if ((row >= 0) && (col < numcols_) && !HasBit(col, row)) {
        SetUtah(row, col, pos++);
      }
      row += 2;
      col -= 2;
    } while ((row < numrows_) && (col >= 0));
    row += 3;
    col++;
  } while ((row < numrows_) || (col < numcols_));
  if (!HasBit(numcols_ - 1, numrows_ - 1)) {
    SetBit(numcols_ - 1, numrows_ - 1, true);
    SetBit(numcols_ - 2, numrows_ - 2, true);
  }
}

void CBC_DefaultPlacement::SetModule(int32_t row,
                                     int32_t col,
                                     int32_t pos,
                                     int32_t bit) {
  if (row < 0) {
    row += numrows_;
    col += 4 - ((numrows_ + 4) % 8);
  }
  if (col < 0) {
    col += numcols_;
    row += 4 - ((numcols_ + 4) % 8);
  }
  int32_t v = codewords_[pos];
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
  SetModule(numrows_ - 1, 0, pos, 1);
  SetModule(numrows_ - 1, 1, pos, 2);
  SetModule(numrows_ - 1, 2, pos, 3);
  SetModule(0, numcols_ - 2, pos, 4);
  SetModule(0, numcols_ - 1, pos, 5);
  SetModule(1, numcols_ - 1, pos, 6);
  SetModule(2, numcols_ - 1, pos, 7);
  SetModule(3, numcols_ - 1, pos, 8);
}

void CBC_DefaultPlacement::SetCorner2(int32_t pos) {
  SetModule(numrows_ - 3, 0, pos, 1);
  SetModule(numrows_ - 2, 0, pos, 2);
  SetModule(numrows_ - 1, 0, pos, 3);
  SetModule(0, numcols_ - 4, pos, 4);
  SetModule(0, numcols_ - 3, pos, 5);
  SetModule(0, numcols_ - 2, pos, 6);
  SetModule(0, numcols_ - 1, pos, 7);
  SetModule(1, numcols_ - 1, pos, 8);
}

void CBC_DefaultPlacement::SetCorner3(int32_t pos) {
  SetModule(numrows_ - 3, 0, pos, 1);
  SetModule(numrows_ - 2, 0, pos, 2);
  SetModule(numrows_ - 1, 0, pos, 3);
  SetModule(0, numcols_ - 2, pos, 4);
  SetModule(0, numcols_ - 1, pos, 5);
  SetModule(1, numcols_ - 1, pos, 6);
  SetModule(2, numcols_ - 1, pos, 7);
  SetModule(3, numcols_ - 1, pos, 8);
}

void CBC_DefaultPlacement::SetCorner4(int32_t pos) {
  SetModule(numrows_ - 1, 0, pos, 1);
  SetModule(numrows_ - 1, numcols_ - 1, pos, 2);
  SetModule(0, numcols_ - 3, pos, 3);
  SetModule(0, numcols_ - 2, pos, 4);
  SetModule(0, numcols_ - 1, pos, 5);
  SetModule(1, numcols_ - 3, pos, 6);
  SetModule(1, numcols_ - 2, pos, 7);
  SetModule(1, numcols_ - 1, pos, 8);
}
