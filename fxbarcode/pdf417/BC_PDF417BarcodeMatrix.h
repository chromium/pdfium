// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_PDF417_BC_PDF417BARCODEMATRIX_H_
#define FXBARCODE_PDF417_BC_PDF417BARCODEMATRIX_H_

#include <memory>
#include <vector>

class CBC_BarcodeRow;

class CBC_BarcodeMatrix {
 public:
  CBC_BarcodeMatrix(size_t height, size_t width);
  virtual ~CBC_BarcodeMatrix();

  CBC_BarcodeRow* getCurrentRow() const { return m_matrix[m_currentRow].get(); }
  size_t getWidth() const { return m_width; }
  size_t getHeight() const { return m_height; }
  void nextRow();
  std::vector<uint8_t>& getMatrix();

 private:
  std::vector<std::unique_ptr<CBC_BarcodeRow>> m_matrix;
  std::vector<uint8_t> m_matrixOut;
  size_t m_currentRow = 0;
  size_t m_width;
  size_t m_height;
};

#endif  // FXBARCODE_PDF417_BC_PDF417BARCODEMATRIX_H_
