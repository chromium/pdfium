// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRALIGNMENTPATTERN_H_
#define _BC_QRALIGNMENTPATTERN_H_
class CBC_CommonBitArray;
class CBC_ResultPoint;
class CBC_REAI013x0x1xDecoder;
class CBC_QRAlignmentPattern : public CBC_ResultPoint {
 private:
  FX_FLOAT m_moduleSize;

 public:
  CBC_QRAlignmentPattern(FX_FLOAT posX,
                         FX_FLOAT posY,
                         FX_FLOAT estimateModuleSize);
  virtual ~CBC_QRAlignmentPattern();
  FX_BOOL AboutEquals(FX_FLOAT moduleSize, FX_FLOAT i, FX_FLOAT j);
  FX_FLOAT GetX();
  FX_FLOAT GetY();
  CBC_QRAlignmentPattern* Clone();
};
#endif
