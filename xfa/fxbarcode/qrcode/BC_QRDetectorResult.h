// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_QRCODE_BC_QRDETECTORRESULT_H_
#define XFA_FXBARCODE_QRCODE_BC_QRDETECTORRESULT_H_

#include <memory>

#include "core/fxcrt/include/fx_basic.h"

class CBC_CommonBitMatrix;
class CBC_ResultPoint;

class CBC_QRDetectorResult final {
 public:
  // Takes ownership of |bits| and |points|.
  CBC_QRDetectorResult(CBC_CommonBitMatrix* bits,
                       CFX_ArrayTemplate<CBC_ResultPoint*>* points);
  ~CBC_QRDetectorResult();

  CBC_CommonBitMatrix* GetBits() const;
  CFX_ArrayTemplate<CBC_ResultPoint*>* GetPoints() const;

 private:
  std::unique_ptr<CBC_CommonBitMatrix> m_bits;
  std::unique_ptr<CFX_ArrayTemplate<CBC_ResultPoint*>> m_points;
};

#endif  // XFA_FXBARCODE_QRCODE_BC_QRDETECTORRESULT_H_
