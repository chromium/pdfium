// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ONEDUPCEANREADER_H_
#define _BC_ONEDUPCEANREADER_H_
class CBC_OneDReader;
class CBC_CommonBitArray;
class CBC_OneDimReader;
class CBC_OneDimReader : public CBC_OneDReader {
 private:
  const static int32_t MAX_AVG_VARIANCE;
  const static int32_t MAX_INDIVIDUAL_VARIANCE;

  FX_BOOL CheckStandardUPCEANChecksum(CFX_ByteString& s, int32_t& e);

 public:
  const static int32_t START_END_PATTERN[3];
  const static int32_t MIDDLE_PATTERN[5];
  const static int32_t L_PATTERNS[10][4];
  const static int32_t L_AND_G_PATTERNS[20][4];
  CBC_OneDimReader();
  virtual ~CBC_OneDimReader();
  CFX_ByteString DecodeRow(int32_t rowNumber,
                           CBC_CommonBitArray* row,
                           int32_t hints,
                           int32_t& e);
  CFX_ByteString DecodeRow(int32_t rowNumber,
                           CBC_CommonBitArray* row,
                           CFX_Int32Array* startGuardRange,
                           int32_t hints,
                           int32_t& e);

 protected:
  CFX_Int32Array* FindStartGuardPattern(CBC_CommonBitArray* row, int32_t& e);
  virtual FX_BOOL CheckChecksum(CFX_ByteString& s, int32_t& e);
  CFX_Int32Array* FindGuardPattern(CBC_CommonBitArray* row,
                                   int32_t rowOffset,
                                   FX_BOOL whiteFirst,
                                   CFX_Int32Array* pattern,
                                   int32_t& e);
  int32_t DecodeDigit(CBC_CommonBitArray* row,
                      CFX_Int32Array* counters,
                      int32_t rowOffset,
                      const int32_t* patterns,
                      int32_t patternLength,
                      int32_t& e);
  virtual int32_t DecodeMiddle(CBC_CommonBitArray* row,
                               CFX_Int32Array* startRange,
                               CFX_ByteString& resultResult,
                               int32_t& e) {
    return 0;
  }
  virtual CFX_Int32Array* DecodeEnd(CBC_CommonBitArray* row,
                                    int32_t endStart,
                                    int32_t& e);
};
#endif
