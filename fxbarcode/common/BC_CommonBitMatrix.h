// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_COMMON_BC_COMMONBITMATRIX_H_
#define FXBARCODE_COMMON_BC_COMMONBITMATRIX_H_

#include <stddef.h>
#include <stdint.h>

#include "core/fxcrt/fixed_size_data_vector.h"

class CBC_CommonBitMatrix {
 public:
  CBC_CommonBitMatrix(size_t width, size_t height);
  ~CBC_CommonBitMatrix();

  bool Get(size_t x, size_t y) const;
  void Set(size_t x, size_t y);

 private:
  const size_t m_height;
  const size_t m_rowSize;
  FixedSizeDataVector<uint32_t> m_bits;
};

#endif  // FXBARCODE_COMMON_BC_COMMONBITMATRIX_H_
