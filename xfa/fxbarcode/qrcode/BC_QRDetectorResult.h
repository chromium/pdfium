// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_QRCODE_BC_QRDETECTORRESULT_H_
#define XFA_FXBARCODE_QRCODE_BC_QRDETECTORRESULT_H_

#include "core/fxcrt/include/fx_basic.h"

class CBC_CommonBitMatrix;

class CBC_QRDetectorResult {
 private:
  CBC_CommonBitMatrix* m_bits;
  CFX_PtrArray* m_points;

 public:
  CBC_QRDetectorResult(CBC_CommonBitMatrix* bits, CFX_PtrArray* points);
  virtual ~CBC_QRDetectorResult();
  CBC_CommonBitMatrix* GetBits();
  CFX_PtrArray* GetPoints();
};

#endif  // XFA_FXBARCODE_QRCODE_BC_QRDETECTORRESULT_H_
