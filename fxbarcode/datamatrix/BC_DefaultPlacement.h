// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_DATAMATRIX_BC_DEFAULTPLACEMENT_H_
#define FXBARCODE_DATAMATRIX_BC_DEFAULTPLACEMENT_H_

#include <stdint.h>

#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/widestring.h"

class CBC_DefaultPlacement final {
 public:
  CBC_DefaultPlacement(WideString codewords, int32_t numcols, int32_t numrows);
  ~CBC_DefaultPlacement();

  bool GetBit(int32_t col, int32_t row) const;

 private:
  void Init();
  void SetModule(int32_t row, int32_t col, int32_t pos, int32_t bit);
  void SetUtah(int32_t row, int32_t col, int32_t pos);
  void SetCorner1(int32_t pos);
  void SetCorner2(int32_t pos);
  void SetCorner3(int32_t pos);
  void SetCorner4(int32_t pos);

  void SetBit(int32_t col, int32_t row, bool bit);
  bool HasBit(int32_t col, int32_t row) const;

  const WideString m_codewords;
  const int32_t m_numrows;
  const int32_t m_numcols;
  DataVector<uint8_t> m_bits;
};

#endif  // FXBARCODE_DATAMATRIX_BC_DEFAULTPLACEMENT_H_
