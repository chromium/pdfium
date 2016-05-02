// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_PDF417_BC_PDF417BARCODEVALUE_H_
#define XFA_FXBARCODE_PDF417_BC_PDF417BARCODEVALUE_H_

#include "core/fxcrt/include/fx_basic.h"

class CBC_BarcodeValue final {
 public:
  CBC_BarcodeValue();
  ~CBC_BarcodeValue();

  void setValue(int32_t value);
  CFX_Int32Array* getValue();
  int32_t getConfidence(int32_t value);

 private:
  CFX_Int32Array m_keys;
  CFX_Int32Array m_values;
};

using CBC_BarcodeValueArray = CFX_ArrayTemplate<CBC_BarcodeValue*>;
using CBC_BarcodeValueArrayArray = CFX_ArrayTemplate<CBC_BarcodeValueArray*>;

#endif  // XFA_FXBARCODE_PDF417_BC_PDF417BARCODEVALUE_H_
