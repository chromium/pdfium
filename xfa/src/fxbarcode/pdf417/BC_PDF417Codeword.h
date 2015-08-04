// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_CODEWORD_H_
#define _BC_CODEWORD_H_
class CBC_Codeword {
 public:
  CBC_Codeword(int32_t startX, int32_t endX, int32_t bucket, int32_t value);
  virtual ~CBC_Codeword();
  FX_BOOL hasValidRowNumber();
  FX_BOOL isValidRowNumber(int32_t rowNumber);
  void setRowNumberAsRowIndicatorColumn();
  int32_t getWidth();
  int32_t getStartX();
  int32_t getEndX();
  int32_t getBucket();
  int32_t getValue();
  int32_t getRowNumber();
  void setRowNumber(int32_t rowNumber);
  CFX_ByteString toString();

 private:
  static int32_t BARCODE_ROW_UNKNOWN;
  int32_t m_startX;
  int32_t m_endX;
  int32_t m_bucket;
  int32_t m_value;
  int32_t m_rowNumber;
};
#endif
