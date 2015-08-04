// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_PDF417COMMON_H_
#define _BC_PDF417COMMON_H_
class CBC_PDF417Common {
 public:
  CBC_PDF417Common();
  virtual ~CBC_PDF417Common();
  static int32_t getBitCountSum(CFX_Int32Array& moduleBitCount);
  static int32_t getCodeword(FX_DWORD symbol);
  static int32_t NUMBER_OF_CODEWORDS;
  static int32_t MAX_CODEWORDS_IN_BARCODE;
  static int32_t MIN_ROWS_IN_BARCODE;
  static int32_t MAX_ROWS_IN_BARCODE;
  static int32_t MAX_CODEWORDS_IN_ROW;
  static int32_t MODULES_IN_CODEWORD;
  static int32_t MODULES_IN_STOP_PATTERN;
  static int32_t BARS_IN_MODULE;
  static int32_t SYMBOL_TABLE[];
  static int32_t CODEWORD_TABLE[];

 private:
  static CFX_Int32Array* EMPTY_INT_ARRAY;
  static int32_t findCodewordIndex(FX_DWORD symbol);
};
#endif
