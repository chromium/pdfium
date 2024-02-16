// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_COMMON_BC_COMMONBYTEMATRIX_H_
#define FXBARCODE_COMMON_BC_COMMONBYTEMATRIX_H_

#include <stdint.h>

#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/span.h"

class CBC_CommonByteMatrix final {
 public:
  CBC_CommonByteMatrix(size_t width, size_t height);
  ~CBC_CommonByteMatrix();

  size_t GetWidth() const { return m_width; }
  size_t GetHeight() const { return m_height; }
  pdfium::span<const uint8_t> GetArray() const { return m_bytes; }
  DataVector<uint8_t> TakeArray();

  uint8_t Get(size_t x, size_t y) const;
  void Set(size_t x, size_t y, uint8_t value);
  void Fill(uint8_t value);

 private:
  const size_t m_width;
  const size_t m_height;
  DataVector<uint8_t> m_bytes;
};

#endif  // FXBARCODE_COMMON_BC_COMMONBYTEMATRIX_H_
