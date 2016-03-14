// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_BC_BINARYBITMAP_H_
#define XFA_FXBARCODE_BC_BINARYBITMAP_H_

#include <cstdint>

class CBC_Binarizer;
class CBC_CommonBitMatrix;
class CBC_CommonBitArray;

class CBC_BinaryBitmap {
 public:
  CBC_BinaryBitmap(CBC_Binarizer* binarizer);
  virtual ~CBC_BinaryBitmap();
  int32_t GetWidth();
  int32_t GetHeight();
  CBC_CommonBitMatrix* GetMatrix(int32_t& e);
  CBC_CommonBitArray* GetBlackRow(int32_t y,
                                  CBC_CommonBitArray* row,
                                  int32_t& e);
  CBC_CommonBitMatrix* GetBlackMatrix(int32_t& e);

 private:
  CBC_Binarizer* m_binarizer;
  CBC_CommonBitMatrix* m_matrix;
};

#endif  // XFA_FXBARCODE_BC_BINARYBITMAP_H_
