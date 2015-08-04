// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRDATAMASK_H_
#define _BC_QRDATAMASK_H_
class CBC_CommonBitMatrix;
class CBC_QRDataMask {
 public:
  static CFX_PtrArray* DATA_MASKS;
  CBC_QRDataMask();
  virtual ~CBC_QRDataMask();
  static void Initialize();
  static void Finalize();
  virtual FX_BOOL IsMasked(int32_t i, int32_t j) = 0;
  void UnmaskBitMatirx(CBC_CommonBitMatrix* bits, int32_t dimension);
  static CBC_QRDataMask* ForReference(int32_t reference, int32_t& e);
  static int32_t BuildDataMasks();
  static void Destroy();
};
#endif
