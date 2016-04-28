// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_QRCODE_BC_FINDERPATTERNINFO_H_
#define XFA_FXBARCODE_QRCODE_BC_FINDERPATTERNINFO_H_

#include "core/fxcrt/include/fx_basic.h"

class CBC_QRFinderPattern;

class CBC_QRFinderPatternInfo {
 public:
  explicit CBC_QRFinderPatternInfo(
      CFX_ArrayTemplate<CBC_QRFinderPattern*>* patternCenters);
  ~CBC_QRFinderPatternInfo();

  CBC_QRFinderPattern* GetBottomLeft() const;
  CBC_QRFinderPattern* GetTopLeft() const;
  CBC_QRFinderPattern* GetTopRight() const;

 private:
  CBC_QRFinderPattern* m_bottomLeft;
  CBC_QRFinderPattern* m_topLeft;
  CBC_QRFinderPattern* m_topRight;
};

#endif  // XFA_FXBARCODE_QRCODE_BC_FINDERPATTERNINFO_H_
