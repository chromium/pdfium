// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_WHITERECTANLEDETECTOR_H_
#define _BC_WHITERECTANLEDETECTOR_H_
class CBC_CommonBitMatrix;
class CBC_ResultPoint;
class CBC_WhiteRectangleDetector {
 public:
  CBC_WhiteRectangleDetector(CBC_CommonBitMatrix* image);
  CBC_WhiteRectangleDetector(CBC_CommonBitMatrix* image,
                             int32_t initSize,
                             int32_t x,
                             int32_t y);
  virtual ~CBC_WhiteRectangleDetector();
  CFX_PtrArray* Detect(int32_t& e);
  virtual void Init(int32_t& e);

 private:
  int32_t Round(float d);
  CBC_ResultPoint* GetBlackPointOnSegment(FX_FLOAT aX,
                                          FX_FLOAT aY,
                                          FX_FLOAT bX,
                                          FX_FLOAT bY);
  int32_t DistanceL2(FX_FLOAT aX, FX_FLOAT aY, FX_FLOAT bX, FX_FLOAT bY);
  CFX_PtrArray* CenterEdges(CBC_ResultPoint* y,
                            CBC_ResultPoint* z,
                            CBC_ResultPoint* x,
                            CBC_ResultPoint* t);
  FX_BOOL ContainsBlackPoint(int32_t a,
                             int32_t b,
                             int32_t fixed,
                             FX_BOOL horizontal);
  const static int32_t INIT_SIZE;
  const static int32_t CORR;

  CBC_CommonBitMatrix* m_image;
  int32_t m_height;
  int32_t m_width;
  int32_t m_leftInit;
  int32_t m_rightInit;
  int32_t m_downInit;
  int32_t m_upInit;
};
#endif
