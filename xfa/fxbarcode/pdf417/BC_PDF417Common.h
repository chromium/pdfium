// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_PDF417_BC_PDF417COMMON_H_
#define XFA_FXBARCODE_PDF417_BC_PDF417COMMON_H_

#include "core/fxcrt/include/fx_basic.h"

class CBC_PDF417Common {
 public:
  CBC_PDF417Common();
  virtual ~CBC_PDF417Common();
  static int32_t getBitCountSum(CFX_Int32Array& moduleBitCount);
  static int32_t getCodeword(uint32_t symbol);
  static const int32_t NUMBER_OF_CODEWORDS = 929;
  static const int32_t MAX_CODEWORDS_IN_BARCODE = NUMBER_OF_CODEWORDS - 1;
  static const int32_t MIN_ROWS_IN_BARCODE = 3;
  static const int32_t MAX_ROWS_IN_BARCODE = 90;
  static const int32_t MAX_CODEWORDS_IN_ROW = 32;
  static const int32_t MODULES_IN_CODEWORD = 17;
  static const int32_t MODULES_IN_STOP_PATTERN = 18;
  static const int32_t BARS_IN_MODULE = 8;
  static const int32_t SYMBOL_TABLE[];
  static const uint16_t CODEWORD_TABLE[];

 private:
  static CFX_Int32Array* EMPTY_INT_ARRAY;
  static int32_t findCodewordIndex(uint32_t symbol);
};

#endif  // XFA_FXBARCODE_PDF417_BC_PDF417COMMON_H_
