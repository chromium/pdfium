// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_PDF417_BC_PDF417DETECTORRESULT_H_
#define XFA_FXBARCODE_PDF417_BC_PDF417DETECTORRESULT_H_

#include "core/fxcrt/include/fx_basic.h"

class CBC_CommonBitMatrix;

class CBC_PDF417DetectorResult {
 public:
  CBC_PDF417DetectorResult(CBC_CommonBitMatrix* bits, CFX_PtrArray* points);
  virtual ~CBC_PDF417DetectorResult();
  CBC_CommonBitMatrix* getBits();
  CFX_PtrArray* getPoints();

 private:
  CBC_CommonBitMatrix* m_bits;
  CFX_PtrArray* m_points;
};

#endif  // XFA_FXBARCODE_PDF417_BC_PDF417DETECTORRESULT_H_
