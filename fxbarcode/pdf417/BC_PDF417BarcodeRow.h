// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_PDF417_BC_PDF417BARCODEROW_H_
#define FXBARCODE_PDF417_BC_PDF417BARCODEROW_H_

#include <stdint.h>

#include "core/fxcrt/fixed_zeroed_data_vector.h"
#include "third_party/base/containers/span.h"

class CBC_BarcodeRow final {
 public:
  explicit CBC_BarcodeRow(size_t width);
  ~CBC_BarcodeRow();

  void AddBar(bool black, size_t width);
  pdfium::span<const uint8_t> GetRow() const { return row_; }

 private:
  FixedZeroedDataVector<uint8_t> row_;
  size_t offset_ = 0;
};

#endif  // FXBARCODE_PDF417_BC_PDF417BARCODEROW_H_
