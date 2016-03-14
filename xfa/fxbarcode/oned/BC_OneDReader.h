// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_ONED_BC_ONEDREADER_H_
#define XFA_FXBARCODE_ONED_BC_ONEDREADER_H_

#include "xfa/fxbarcode/BC_Reader.h"

class CBC_BinaryBitmap;
class CBC_CommonBitArray;

class CBC_OneDReader : public CBC_Reader {
 public:
  CBC_OneDReader();
  virtual ~CBC_OneDReader();
  virtual CFX_ByteString Decode(CBC_BinaryBitmap* image, int32_t& e);
  virtual CFX_ByteString Decode(CBC_BinaryBitmap* image,
                                int32_t hints,
                                int32_t& e);
  virtual CFX_ByteString DecodeRow(int32_t rowNumber,
                                   CBC_CommonBitArray* row,
                                   int32_t hints,
                                   int32_t& e) {
    return "";
  }

 private:
  CFX_ByteString DeDecode(CBC_BinaryBitmap* image, int32_t hints, int32_t& e);

 protected:
  static const int32_t INTEGER_MATH_SHIFT = 8;
  static const int32_t PATTERN_MATCH_RESULT_SCALE_FACTOR = 1 << 8;

  void RecordPattern(CBC_CommonBitArray* row,
                     int32_t start,
                     CFX_Int32Array* counters,
                     int32_t& e);
  void RecordPatternInReverse(CBC_CommonBitArray* row,
                              int32_t start,
                              CFX_Int32Array* counters,
                              int32_t& e);
  int32_t PatternMatchVariance(CFX_Int32Array* counters,
                               const int32_t* pattern,
                               int32_t maxIndividualVariance);
};

#endif  // XFA_FXBARCODE_ONED_BC_ONEDREADER_H_
