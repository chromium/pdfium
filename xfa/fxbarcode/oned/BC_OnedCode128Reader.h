// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_ONED_BC_ONEDCODE128READER_H_
#define XFA_FXBARCODE_ONED_BC_ONEDCODE128READER_H_

#include "xfa/fxbarcode/oned/BC_OneDReader.h"

class CBC_CommonBitArray;

class CBC_OnedCode128Reader : public CBC_OneDReader {
 public:
  CBC_OnedCode128Reader();
  virtual ~CBC_OnedCode128Reader();
  virtual CFX_ByteString DecodeRow(int32_t rowNumber,
                                   CBC_CommonBitArray* row,
                                   int32_t hints,
                                   int32_t& e);

  static const int32_t CODE_PATTERNS[107][7];

 private:
  static const int32_t MAX_AVG_VARIANCE = (int32_t)(256 * 0.25f);
  static const int32_t MAX_INDIVIDUAL_VARIANCE = (int32_t)(256 * 0.7f);

  static const int32_t CODE_SHIFT = 98;
  static const int32_t CODE_CODE_C = 99;
  static const int32_t CODE_CODE_B = 100;
  static const int32_t CODE_CODE_A = 101;
  static const int32_t CODE_FNC_1 = 102;
  static const int32_t CODE_FNC_2 = 97;
  static const int32_t CODE_FNC_3 = 96;
  static const int32_t CODE_FNC_4_A = 101;
  static const int32_t CODE_FNC_4_B = 100;

  static const int32_t CODE_START_A = 103;
  static const int32_t CODE_START_B = 104;
  static const int32_t CODE_START_C = 105;
  static const int32_t CODE_STOP = 106;

  CFX_Int32Array* FindStartPattern(CBC_CommonBitArray* row, int32_t& e);
  int32_t DecodeCode(CBC_CommonBitArray* row,
                     CFX_Int32Array* counters,
                     int32_t rowOffset,
                     int32_t& e);
};

#endif  // XFA_FXBARCODE_ONED_BC_ONEDCODE128READER_H_
