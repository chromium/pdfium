// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ONEDCODA128READER_H_
#define _BC_ONEDCODA128READER_H_
class CBC_OneDReader;
class CBC_CommonBitArray;
class CBC_OnedCoda128Reader;
class CBC_OnedCode128Reader : public CBC_OneDReader {
 public:
  CBC_OnedCode128Reader();
  virtual ~CBC_OnedCode128Reader();
  virtual CFX_ByteString DecodeRow(int32_t rowNumber,
                                   CBC_CommonBitArray* row,
                                   int32_t hints,
                                   int32_t& e);
  const static int32_t CODE_PATTERNS[107][7];
  const static int32_t MAX_AVG_VARIANCE;
  const static int32_t MAX_INDIVIDUAL_VARIANCE;

  const static int32_t CODE_SHIFT;
  const static int32_t CODE_CODE_C;
  const static int32_t CODE_CODE_B;
  const static int32_t CODE_CODE_A;
  const static int32_t CODE_FNC_1;
  const static int32_t CODE_FNC_2;
  const static int32_t CODE_FNC_3;
  const static int32_t CODE_FNC_4_A;
  const static int32_t CODE_FNC_4_B;

  const static int32_t CODE_START_A;
  const static int32_t CODE_START_B;
  const static int32_t CODE_START_C;
  const static int32_t CODE_STOP;

 private:
  CFX_Int32Array* FindStartPattern(CBC_CommonBitArray* row, int32_t& e);
  int32_t DecodeCode(CBC_CommonBitArray* row,
                     CFX_Int32Array* counters,
                     int32_t rowOffset,
                     int32_t& e);
};
#endif
