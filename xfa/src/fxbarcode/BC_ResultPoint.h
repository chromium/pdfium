// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_RESULTPOINT_H_
#define _BC_RESULTPOINT_H_
class CBC_ResultPoint;
class CBC_ResultPoint {
 public:
  CBC_ResultPoint();
  CBC_ResultPoint(FX_FLOAT x, FX_FLOAT y);
  virtual ~CBC_ResultPoint() {}
  virtual FX_FLOAT GetX();
  virtual FX_FLOAT GetY();

 protected:
  FX_FLOAT m_x;
  FX_FLOAT m_y;
};
#endif
