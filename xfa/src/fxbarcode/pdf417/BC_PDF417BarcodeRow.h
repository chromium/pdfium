// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_BARCODEROW_H_
#define _BC_BARCODEROW_H_
class CBC_BarcodeRow {
 public:
  CBC_BarcodeRow(int32_t width);
  virtual ~CBC_BarcodeRow();
  void set(int32_t x, uint8_t value);
  void set(int32_t x, FX_BOOL black);
  void addBar(FX_BOOL black, int32_t width);
  CFX_ByteArray& getRow();
  CFX_ByteArray& getScaledRow(int32_t scale);

 private:
  CFX_ByteArray m_row;
  CFX_ByteArray m_output;
  int32_t m_currentLocation;
};
#endif
