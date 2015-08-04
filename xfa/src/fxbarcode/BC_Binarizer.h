// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_BINARIZER_H_
#define _BC_BINARIZER_H_
class CBC_LuminanceSource;
class CBC_CommonBitMatrix;
class CBC_CommonBitArray;
class CBC_BinaryBitmap;
class CBC_Binarizer {
 public:
  CBC_Binarizer(CBC_LuminanceSource* source);
  virtual ~CBC_Binarizer();
  CBC_LuminanceSource* GetLuminanceSource();
  virtual CBC_CommonBitMatrix* GetBlackMatrix(int32_t& e) = 0;
  virtual CBC_CommonBitArray* GetBlackRow(int32_t y,
                                          CBC_CommonBitArray* row,
                                          int32_t& e) = 0;

 private:
  CBC_LuminanceSource* m_source;
};
#endif
