// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_PDF417_BC_PDF417DETECTORRESULT_H_
#define XFA_FXBARCODE_PDF417_BC_PDF417DETECTORRESULT_H_

#include "core/fxcrt/include/fx_basic.h"
#include "xfa/fxbarcode/BC_ResultPoint.h"

class CBC_CommonBitMatrix;

class CBC_PDF417DetectorResult final {
 public:
  // Takes ownership of |points|.
  CBC_PDF417DetectorResult(CBC_CommonBitMatrix* bits,
                           CBC_ResultPointArrayArray* points);
  ~CBC_PDF417DetectorResult();

  CBC_CommonBitMatrix* getBits() const;
  CBC_ResultPointArrayArray* getPoints() const;

 private:
  CBC_CommonBitMatrix* m_bits;
  std::unique_ptr<CBC_ResultPointArrayArray> m_points;
};

#endif  // XFA_FXBARCODE_PDF417_BC_PDF417DETECTORRESULT_H_
