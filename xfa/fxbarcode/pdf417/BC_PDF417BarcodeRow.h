// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_PDF417_BC_PDF417BARCODEROW_H_
#define XFA_FXBARCODE_PDF417_BC_PDF417BARCODEROW_H_

#include "core/fxcrt/fx_basic.h"

class CBC_BarcodeRow {
 public:
  explicit CBC_BarcodeRow(int32_t width);
  virtual ~CBC_BarcodeRow();

  void set(int32_t x, uint8_t value);
  void set(int32_t x, bool black);
  void addBar(bool black, int32_t width);
  CFX_ArrayTemplate<uint8_t>& getRow();
  CFX_ArrayTemplate<uint8_t>& getScaledRow(int32_t scale);

 private:
  CFX_ArrayTemplate<uint8_t> m_row;
  CFX_ArrayTemplate<uint8_t> m_output;
  int32_t m_currentLocation;
};

#endif  // XFA_FXBARCODE_PDF417_BC_PDF417BARCODEROW_H_
