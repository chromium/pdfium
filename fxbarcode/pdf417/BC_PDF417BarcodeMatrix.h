// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_PDF417_BC_PDF417BARCODEMATRIX_H_
#define FXBARCODE_PDF417_BC_PDF417BARCODEMATRIX_H_

#include <stdint.h>

#include <memory>
#include <vector>

#include "core/fxcrt/data_vector.h"

class CBC_BarcodeRow;

class CBC_BarcodeMatrix final {
 public:
  CBC_BarcodeMatrix(size_t width, size_t height);
  ~CBC_BarcodeMatrix();

  CBC_BarcodeRow* getRow(size_t row) const { return matrix_[row].get(); }
  size_t getWidth() const { return width_; }
  size_t getHeight() const { return height_; }
  DataVector<uint8_t> toBitArray();

 private:
  std::vector<std::unique_ptr<CBC_BarcodeRow>> matrix_;
  size_t width_;
  size_t height_;
};

#endif  // FXBARCODE_PDF417_BC_PDF417BARCODEMATRIX_H_
