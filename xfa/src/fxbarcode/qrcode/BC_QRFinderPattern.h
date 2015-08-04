// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRFINDERPATTERN_H_
#define _BC_QRFINDERPATTERN_H_
class CBC_ResultPoint;
class CBC_QRFinderPattern;
class CBC_QRFinderPattern : public CBC_ResultPoint {
 private:
  FX_FLOAT m_estimatedModuleSize;
  int32_t m_count;

 public:
  CBC_QRFinderPattern(FX_FLOAT x, FX_FLOAT posY, FX_FLOAT estimatedModuleSize);
  virtual ~CBC_QRFinderPattern();

  int32_t GetCount();
  FX_FLOAT GetX();
  FX_FLOAT GetY();
  FX_FLOAT GetEstimatedModuleSize();
  void IncrementCount();
  FX_BOOL AboutEquals(FX_FLOAT moduleSize, FX_FLOAT i, FX_FLOAT j);
  CBC_QRFinderPattern* Clone();
};
typedef CBC_QRFinderPattern FinderPattern;
#endif
