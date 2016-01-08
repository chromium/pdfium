// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2007 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "xfa/src/fxbarcode/barcode.h"
#include "BC_CommonPerspectiveTransform.h"
CBC_CommonPerspectiveTransform::CBC_CommonPerspectiveTransform(FX_FLOAT a11,
                                                               FX_FLOAT a21,
                                                               FX_FLOAT a31,
                                                               FX_FLOAT a12,
                                                               FX_FLOAT a22,
                                                               FX_FLOAT a32,
                                                               FX_FLOAT a13,
                                                               FX_FLOAT a23,
                                                               FX_FLOAT a33)
    : m_a11(a11),
      m_a12(a12),
      m_a13(a13),
      m_a21(a21),
      m_a22(a22),
      m_a23(a23),
      m_a31(a31),
      m_a32(a32),
      m_a33(a33) {
}
CBC_CommonPerspectiveTransform::~CBC_CommonPerspectiveTransform() {}
CBC_CommonPerspectiveTransform*
CBC_CommonPerspectiveTransform::QuadrilateralToQuadrilateral(FX_FLOAT x0,
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
                                                             FX_FLOAT y3p) {
  CBC_AutoPtr<CBC_CommonPerspectiveTransform> qToS(
      QuadrilateralToSquare(x0, y0, x1, y1, x2, y2, x3, y3));
  CBC_AutoPtr<CBC_CommonPerspectiveTransform> sToQ(
      SquareToQuadrilateral(x0p, y0p, x1p, y1p, x2p, y2p, x3p, y3p));
  return sToQ->Times(*(qToS.get()));
}
void CBC_CommonPerspectiveTransform::TransformPoints(CFX_FloatArray* points) {
  int32_t max = points->GetSize();
  FX_FLOAT a11 = m_a11;
  FX_FLOAT a12 = m_a12;
  FX_FLOAT a13 = m_a13;
  FX_FLOAT a21 = m_a21;
  FX_FLOAT a22 = m_a22;
  FX_FLOAT a23 = m_a23;
  FX_FLOAT a31 = m_a31;
  FX_FLOAT a32 = m_a32;
  FX_FLOAT a33 = m_a33;
  int32_t i;
  for (i = 0; i < max; i += 2) {
    FX_FLOAT x = (*points)[i];
    FX_FLOAT y = (*points)[i + 1];
    FX_FLOAT denominator = a13 * x + a23 * y + a33;
    (*points)[i] = (a11 * x + a21 * y + a31) / denominator;
    (*points)[i + 1] = (a12 * x + a22 * y + a32) / denominator;
  }
}
CBC_CommonPerspectiveTransform*
CBC_CommonPerspectiveTransform::SquareToQuadrilateral(FX_FLOAT x0,
                                                      FX_FLOAT y0,
                                                      FX_FLOAT x1,
                                                      FX_FLOAT y1,
                                                      FX_FLOAT x2,
                                                      FX_FLOAT y2,
                                                      FX_FLOAT x3,
                                                      FX_FLOAT y3) {
  FX_FLOAT dy2 = y3 - y2;
  FX_FLOAT dy3 = y0 - y1 + y2 - y3;
  if ((dy2 == 0.0f) && (dy3 == 0.0f)) {
    return new CBC_CommonPerspectiveTransform(x1 - x0, x2 - x1, x0, y1 - y0,
                                              y2 - y1, y0, 0.0f, 0.0f, 1.0f);
  } else {
    FX_FLOAT dx1 = x1 - x2;
    FX_FLOAT dx2 = x3 - x2;
    FX_FLOAT dx3 = x0 - x1 + x2 - x3;
    FX_FLOAT dy1 = y1 - y2;
    FX_FLOAT denominator = dx1 * dy2 - dx2 * dy1;
    FX_FLOAT a13 = (dx3 * dy2 - dx2 * dy3) / denominator;
    FX_FLOAT a23 = (dx1 * dy3 - dx3 * dy1) / denominator;
    return new CBC_CommonPerspectiveTransform(
        x1 - x0 + a13 * x1, x3 - x0 + a23 * x3, x0, y1 - y0 + a13 * y1,
        y3 - y0 + a23 * y3, y0, a13, a23, 1.0f);
  }
}
CBC_CommonPerspectiveTransform*
CBC_CommonPerspectiveTransform::QuadrilateralToSquare(FX_FLOAT x0,
                                                      FX_FLOAT y0,
                                                      FX_FLOAT x1,
                                                      FX_FLOAT y1,
                                                      FX_FLOAT x2,
                                                      FX_FLOAT y2,
                                                      FX_FLOAT x3,
                                                      FX_FLOAT y3) {
  CBC_AutoPtr<CBC_CommonPerspectiveTransform> temp1(
      SquareToQuadrilateral(x0, y0, x1, y1, x2, y2, x3, y3));
  return temp1->BuildAdjoint();
}
CBC_CommonPerspectiveTransform* CBC_CommonPerspectiveTransform::BuildAdjoint() {
  return new CBC_CommonPerspectiveTransform(
      m_a22 * m_a33 - m_a23 * m_a32, m_a23 * m_a31 - m_a21 * m_a33,
      m_a21 * m_a32 - m_a22 * m_a31, m_a13 * m_a32 - m_a12 * m_a33,
      m_a11 * m_a33 - m_a13 * m_a31, m_a12 * m_a31 - m_a11 * m_a32,
      m_a12 * m_a23 - m_a13 * m_a22, m_a13 * m_a21 - m_a11 * m_a23,
      m_a11 * m_a22 - m_a12 * m_a21);
}
CBC_CommonPerspectiveTransform* CBC_CommonPerspectiveTransform::Times(
    CBC_CommonPerspectiveTransform& other) {
  return new CBC_CommonPerspectiveTransform(
      m_a11 * other.m_a11 + m_a21 * other.m_a12 + m_a31 * other.m_a13,
      m_a11 * other.m_a21 + m_a21 * other.m_a22 + m_a31 * other.m_a23,
      m_a11 * other.m_a31 + m_a21 * other.m_a32 + m_a31 * other.m_a33,
      m_a12 * other.m_a11 + m_a22 * other.m_a12 + m_a32 * other.m_a13,
      m_a12 * other.m_a21 + m_a22 * other.m_a22 + m_a32 * other.m_a23,
      m_a12 * other.m_a31 + m_a22 * other.m_a32 + m_a32 * other.m_a33,
      m_a13 * other.m_a11 + m_a23 * other.m_a12 + m_a33 * other.m_a13,
      m_a13 * other.m_a21 + m_a23 * other.m_a22 + m_a33 * other.m_a23,
      m_a13 * other.m_a31 + m_a23 * other.m_a32 + m_a33 * other.m_a33);
}
