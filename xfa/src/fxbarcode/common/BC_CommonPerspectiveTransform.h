// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_COMMONPERSPECTIVETRANSFORM_H_
#define _BC_COMMONPERSPECTIVETRANSFORM_H_
class CBC_CommonPerspectiveTransform {
 public:
  CBC_CommonPerspectiveTransform(FX_FLOAT a11,
                                 FX_FLOAT a21,
                                 FX_FLOAT a31,
                                 FX_FLOAT a12,
                                 FX_FLOAT a22,
                                 FX_FLOAT a32,
                                 FX_FLOAT a13,
                                 FX_FLOAT a23,
                                 FX_FLOAT a33);
  virtual ~CBC_CommonPerspectiveTransform();
  static CBC_CommonPerspectiveTransform* QuadrilateralToQuadrilateral(
      FX_FLOAT x0,
      FX_FLOAT y0,
      FX_FLOAT x1,
      FX_FLOAT y1,
      FX_FLOAT x2,
      FX_FLOAT y2,
      FX_FLOAT x3,
      FX_FLOAT y3,
      FX_FLOAT x0p,
      FX_FLOAT y0p,
      FX_FLOAT x1p,
      FX_FLOAT y1p,
      FX_FLOAT x2p,
      FX_FLOAT y2p,
      FX_FLOAT x3p,
      FX_FLOAT y3p);
  static CBC_CommonPerspectiveTransform* SquareToQuadrilateral(FX_FLOAT x0,
                                                               FX_FLOAT y0,
                                                               FX_FLOAT x1,
                                                               FX_FLOAT y1,
                                                               FX_FLOAT x2,
                                                               FX_FLOAT y2,
                                                               FX_FLOAT x3,
                                                               FX_FLOAT y3);
  static CBC_CommonPerspectiveTransform* QuadrilateralToSquare(FX_FLOAT x0,
                                                               FX_FLOAT y0,
                                                               FX_FLOAT x1,
                                                               FX_FLOAT y1,
                                                               FX_FLOAT x2,
                                                               FX_FLOAT y2,
                                                               FX_FLOAT x3,
                                                               FX_FLOAT y3);
  CBC_CommonPerspectiveTransform* BuildAdjoint();
  CBC_CommonPerspectiveTransform* Times(CBC_CommonPerspectiveTransform& other);
  void TransformPoints(CFX_FloatArray* points);

 private:
  FX_FLOAT m_a11, m_a12, m_a13, m_a21, m_a22, m_a23, m_a31, m_a32, m_a33;
};
#endif
