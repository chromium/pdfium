// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2008 ZXing authors
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

#include <algorithm>

#include "xfa/src/fxbarcode/barcode.h"
#include "xfa/src/fxbarcode/BC_ResultPoint.h"
#include "xfa/src/fxbarcode/common/BC_WhiteRectangleDetector.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitMatrix.h"
#include "xfa/src/fxbarcode/qrcode/BC_QRFinderPatternFinder.h"
#include "xfa/src/fxbarcode/qrcode/BC_QRDetectorResult.h"
#include "xfa/src/fxbarcode/qrcode/BC_QRGridSampler.h"
#include "BC_DataMatrixDetector.h"
const int32_t CBC_DataMatrixDetector::INTEGERS[5] = {0, 1, 2, 3, 4};
CBC_DataMatrixDetector::CBC_DataMatrixDetector(CBC_CommonBitMatrix* image)
    : m_image(image), m_rectangleDetector(NULL) {}
void CBC_DataMatrixDetector::Init(int32_t& e) {
  m_rectangleDetector = new CBC_WhiteRectangleDetector(m_image);
  m_rectangleDetector->Init(e);
  BC_EXCEPTION_CHECK_ReturnVoid(e);
}
CBC_DataMatrixDetector::~CBC_DataMatrixDetector() {
  if (m_rectangleDetector != NULL) {
    delete m_rectangleDetector;
  }
  m_rectangleDetector = NULL;
}
inline FX_BOOL ResultPointsAndTransitionsComparator(void* a, void* b) {
  return ((CBC_ResultPointsAndTransitions*)b)->GetTransitions() >
         ((CBC_ResultPointsAndTransitions*)a)->GetTransitions();
}
CBC_QRDetectorResult* CBC_DataMatrixDetector::Detect(int32_t& e) {
  CFX_PtrArray* cornerPoints = m_rectangleDetector->Detect(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  CBC_ResultPoint* pointA = (CBC_ResultPoint*)(*cornerPoints)[0];
  CBC_ResultPoint* pointB = (CBC_ResultPoint*)(*cornerPoints)[1];
  CBC_ResultPoint* pointC = (CBC_ResultPoint*)(*cornerPoints)[2];
  CBC_ResultPoint* pointD = (CBC_ResultPoint*)(*cornerPoints)[3];
  delete cornerPoints;
  cornerPoints = NULL;
  CFX_PtrArray transitions;
  transitions.Add(TransitionsBetween(pointA, pointB));
  transitions.Add(TransitionsBetween(pointA, pointC));
  transitions.Add(TransitionsBetween(pointB, pointD));
  transitions.Add(TransitionsBetween(pointC, pointD));
  BC_FX_PtrArray_Sort(transitions, &ResultPointsAndTransitionsComparator);
  delete ((CBC_ResultPointsAndTransitions*)transitions[2]);
  delete ((CBC_ResultPointsAndTransitions*)transitions[3]);
  CBC_ResultPointsAndTransitions* lSideOne =
      (CBC_ResultPointsAndTransitions*)transitions[0];
  CBC_ResultPointsAndTransitions* lSideTwo =
      (CBC_ResultPointsAndTransitions*)transitions[1];
  CFX_MapPtrTemplate<CBC_ResultPoint*, int32_t> pointCount;
  Increment(pointCount, lSideOne->GetFrom());
  Increment(pointCount, lSideOne->GetTo());
  Increment(pointCount, lSideTwo->GetFrom());
  Increment(pointCount, lSideTwo->GetTo());
  delete ((CBC_ResultPointsAndTransitions*)transitions[1]);
  delete ((CBC_ResultPointsAndTransitions*)transitions[0]);
  transitions.RemoveAll();
  CBC_ResultPoint* maybeTopLeft = NULL;
  CBC_ResultPoint* bottomLeft = NULL;
  CBC_ResultPoint* maybeBottomRight = NULL;
  FX_POSITION itBegin = pointCount.GetStartPosition();
  while (itBegin != NULL) {
    CBC_ResultPoint* key = 0;
    int32_t value = 0;
    pointCount.GetNextAssoc(itBegin, key, value);
    if (value == 2) {
      bottomLeft = key;
    } else {
      if (maybeBottomRight == NULL) {
        maybeBottomRight = key;
      } else {
        maybeTopLeft = key;
      }
    }
  }
  if (maybeTopLeft == NULL || bottomLeft == NULL || maybeBottomRight == NULL) {
    delete pointA;
    delete pointB;
    delete pointC;
    delete pointD;
    e = BCExceptionNotFound;
    return NULL;
  }
  CFX_PtrArray corners;
  corners.SetSize(3);
  corners[0] = maybeTopLeft;
  corners[1] = bottomLeft;
  corners[2] = maybeBottomRight;
  OrderBestPatterns(&corners);
  CBC_ResultPoint* bottomRight = (CBC_ResultPoint*)corners[0];
  bottomLeft = (CBC_ResultPoint*)corners[1];
  CBC_ResultPoint* topLeft = (CBC_ResultPoint*)corners[2];
  CBC_ResultPoint* topRight = NULL;
  int32_t value;
  if (!pointCount.Lookup(pointA, value)) {
    topRight = pointA;
  } else if (!pointCount.Lookup(pointB, value)) {
    topRight = pointB;
  } else if (!pointCount.Lookup(pointC, value)) {
    topRight = pointC;
  } else {
    topRight = pointD;
  }
  int32_t dimensionTop = CBC_AutoPtr<CBC_ResultPointsAndTransitions>(
                             TransitionsBetween(topLeft, topRight))
                             ->GetTransitions();
  int32_t dimensionRight = CBC_AutoPtr<CBC_ResultPointsAndTransitions>(
                               TransitionsBetween(bottomRight, topRight))
                               ->GetTransitions();
  if ((dimensionTop & 0x01) == 1) {
    dimensionTop++;
  }
  dimensionTop += 2;
  if ((dimensionRight & 0x01) == 1) {
    dimensionRight++;
  }
  dimensionRight += 2;
  CBC_AutoPtr<CBC_CommonBitMatrix> bits(NULL);
  CBC_AutoPtr<CBC_ResultPoint> correctedTopRight(NULL);
  if (4 * dimensionTop >= 7 * dimensionRight ||
      4 * dimensionRight >= 7 * dimensionTop) {
    correctedTopRight = CBC_AutoPtr<CBC_ResultPoint>(
        CorrectTopRightRectangular(bottomLeft, bottomRight, topLeft, topRight,
                                   dimensionTop, dimensionRight));
    if (correctedTopRight.get() == NULL) {
      correctedTopRight = CBC_AutoPtr<CBC_ResultPoint>(topRight);
    } else {
      delete topRight;
      topRight = NULL;
    }
    dimensionTop = CBC_AutoPtr<CBC_ResultPointsAndTransitions>(
                       TransitionsBetween(topLeft, correctedTopRight.get()))
                       ->GetTransitions();
    dimensionRight =
        CBC_AutoPtr<CBC_ResultPointsAndTransitions>(
            TransitionsBetween(bottomRight, correctedTopRight.get()))
            ->GetTransitions();
    if ((dimensionTop & 0x01) == 1) {
      dimensionTop++;
    }
    if ((dimensionRight & 0x01) == 1) {
      dimensionRight++;
    }
    bits = CBC_AutoPtr<CBC_CommonBitMatrix>(
        SampleGrid(m_image, topLeft, bottomLeft, bottomRight,
                   correctedTopRight.get(), dimensionTop, dimensionRight, e));
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  } else {
    int32_t dimension = std::min(dimensionRight, dimensionTop);
    correctedTopRight = CBC_AutoPtr<CBC_ResultPoint>(
        CorrectTopRight(bottomLeft, bottomRight, topLeft, topRight, dimension));
    if (correctedTopRight.get() == NULL) {
      correctedTopRight = CBC_AutoPtr<CBC_ResultPoint>(topRight);
    } else {
      delete topRight;
      topRight = NULL;
    }
    int32_t dimensionCorrected =
        std::max(CBC_AutoPtr<CBC_ResultPointsAndTransitions>(
                     TransitionsBetween(topLeft, correctedTopRight.get()))
                     ->GetTransitions(),
                 CBC_AutoPtr<CBC_ResultPointsAndTransitions>(
                     TransitionsBetween(bottomRight, correctedTopRight.get()))
                     ->GetTransitions());
    dimensionCorrected++;
    if ((dimensionCorrected & 0x01) == 1) {
      dimensionCorrected++;
    }
    bits = CBC_AutoPtr<CBC_CommonBitMatrix>(SampleGrid(
        m_image, topLeft, bottomLeft, bottomRight, correctedTopRight.get(),
        dimensionCorrected, dimensionCorrected, e));
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  }
  CFX_PtrArray* result = new CFX_PtrArray;
  result->SetSize(4);
  result->Add(topLeft);
  result->Add(bottomLeft);
  result->Add(bottomRight);
  result->Add(correctedTopRight.release());
  return new CBC_QRDetectorResult(bits.release(), result);
}
CBC_ResultPoint* CBC_DataMatrixDetector::CorrectTopRightRectangular(
    CBC_ResultPoint* bottomLeft,
    CBC_ResultPoint* bottomRight,
    CBC_ResultPoint* topLeft,
    CBC_ResultPoint* topRight,
    int32_t dimensionTop,
    int32_t dimensionRight) {
  FX_FLOAT corr = Distance(bottomLeft, bottomRight) / (FX_FLOAT)dimensionTop;
  int32_t norm = Distance(topLeft, topRight);
  FX_FLOAT cos = (topRight->GetX() - topLeft->GetX()) / norm;
  FX_FLOAT sin = (topRight->GetY() - topLeft->GetY()) / norm;
  CBC_AutoPtr<CBC_ResultPoint> c1(new CBC_ResultPoint(
      topRight->GetX() + corr * cos, topRight->GetY() + corr * sin));
  corr = Distance(bottomLeft, topLeft) / (FX_FLOAT)dimensionRight;
  norm = Distance(bottomRight, topRight);
  cos = (topRight->GetX() - bottomRight->GetX()) / norm;
  sin = (topRight->GetY() - bottomRight->GetY()) / norm;
  CBC_AutoPtr<CBC_ResultPoint> c2(new CBC_ResultPoint(
      topRight->GetX() + corr * cos, topRight->GetY() + corr * sin));
  if (!IsValid(c1.get())) {
    if (IsValid(c2.get())) {
      return c2.release();
    }
    return NULL;
  } else if (!IsValid(c2.get())) {
    return c1.release();
  }
  int32_t l1 = FXSYS_abs(dimensionTop -
                         CBC_AutoPtr<CBC_ResultPointsAndTransitions>(
                             TransitionsBetween(topLeft, c1.get()))
                             ->GetTransitions()) +
               FXSYS_abs(dimensionRight -
                         CBC_AutoPtr<CBC_ResultPointsAndTransitions>(
                             TransitionsBetween(bottomRight, c1.get()))
                             ->GetTransitions());
  int32_t l2 = FXSYS_abs(dimensionTop -
                         CBC_AutoPtr<CBC_ResultPointsAndTransitions>(
                             TransitionsBetween(topLeft, c2.get()))
                             ->GetTransitions()) +
               FXSYS_abs(dimensionRight -
                         CBC_AutoPtr<CBC_ResultPointsAndTransitions>(
                             TransitionsBetween(bottomRight, c2.get()))
                             ->GetTransitions());
  if (l1 <= l2) {
    return c1.release();
  }
  return c2.release();
}
CBC_ResultPoint* CBC_DataMatrixDetector::CorrectTopRight(
    CBC_ResultPoint* bottomLeft,
    CBC_ResultPoint* bottomRight,
    CBC_ResultPoint* topLeft,
    CBC_ResultPoint* topRight,
    int32_t dimension) {
  FX_FLOAT corr = Distance(bottomLeft, bottomRight) / (FX_FLOAT)dimension;
  int32_t norm = Distance(topLeft, topRight);
  FX_FLOAT cos = (topRight->GetX() - topLeft->GetX()) / norm;
  FX_FLOAT sin = (topRight->GetY() - topLeft->GetY()) / norm;
  CBC_AutoPtr<CBC_ResultPoint> c1(new CBC_ResultPoint(
      topRight->GetX() + corr * cos, topRight->GetY() + corr * sin));
  corr = Distance(bottomLeft, bottomRight) / (FX_FLOAT)dimension;
  norm = Distance(bottomRight, topRight);
  cos = (topRight->GetX() - bottomRight->GetX()) / norm;
  sin = (topRight->GetY() - bottomRight->GetY()) / norm;
  CBC_AutoPtr<CBC_ResultPoint> c2(new CBC_ResultPoint(
      topRight->GetX() + corr * cos, topRight->GetY() + corr * sin));
  if (!IsValid(c1.get())) {
    if (IsValid(c2.get())) {
      return c2.release();
    }
    return NULL;
  } else if (!IsValid(c2.get())) {
    return c1.release();
  }
  int32_t l1 = FXSYS_abs(CBC_AutoPtr<CBC_ResultPointsAndTransitions>(
                             TransitionsBetween(topLeft, c1.get()))
                             ->GetTransitions() -
                         CBC_AutoPtr<CBC_ResultPointsAndTransitions>(
                             TransitionsBetween(bottomRight, c1.get()))
                             ->GetTransitions());
  int32_t l2 = FXSYS_abs(CBC_AutoPtr<CBC_ResultPointsAndTransitions>(
                             TransitionsBetween(topLeft, c2.get()))
                             ->GetTransitions() -
                         CBC_AutoPtr<CBC_ResultPointsAndTransitions>(
                             TransitionsBetween(bottomRight, c2.get()))
                             ->GetTransitions());
  return l1 <= l2 ? c1.release() : c2.release();
}
FX_BOOL CBC_DataMatrixDetector::IsValid(CBC_ResultPoint* p) {
  return p->GetX() >= 0 && p->GetX() < m_image->GetWidth() && p->GetY() > 0 &&
         p->GetY() < m_image->GetHeight();
}
int32_t CBC_DataMatrixDetector::Round(FX_FLOAT d) {
  return (int32_t)(d + 0.5f);
}
int32_t CBC_DataMatrixDetector::Distance(CBC_ResultPoint* a,
                                         CBC_ResultPoint* b) {
  return Round(
      (FX_FLOAT)sqrt((a->GetX() - b->GetX()) * (a->GetX() - b->GetX()) +
                     (a->GetY() - b->GetY()) * (a->GetY() - b->GetY())));
}
void CBC_DataMatrixDetector::Increment(
    CFX_MapPtrTemplate<CBC_ResultPoint*, int32_t>& table,
    CBC_ResultPoint* key) {
  int32_t value;
  if (table.Lookup(key, value)) {
    table.SetAt(key, INTEGERS[value + 1]);
  } else {
    table.SetAt(key, INTEGERS[1]);
  }
}
CBC_CommonBitMatrix* CBC_DataMatrixDetector::SampleGrid(
    CBC_CommonBitMatrix* image,
    CBC_ResultPoint* topLeft,
    CBC_ResultPoint* bottomLeft,
    CBC_ResultPoint* bottomRight,
    CBC_ResultPoint* topRight,
    int32_t dimensionX,
    int32_t dimensionY,
    int32_t& e) {
  CBC_QRGridSampler& sampler = CBC_QRGridSampler::GetInstance();
  CBC_CommonBitMatrix* cbm = sampler.SampleGrid(
      image, dimensionX, dimensionY, 0.5f, 0.5f, dimensionX - 0.5f, 0.5f,
      dimensionX - 0.5f, dimensionY - 0.5f, 0.5f, dimensionY - 0.5f,
      topLeft->GetX(), topLeft->GetY(), topRight->GetX(), topRight->GetY(),
      bottomRight->GetX(), bottomRight->GetY(), bottomLeft->GetX(),
      bottomLeft->GetY(), e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return cbm;
}
CBC_ResultPointsAndTransitions* CBC_DataMatrixDetector::TransitionsBetween(
    CBC_ResultPoint* from,
    CBC_ResultPoint* to) {
  int32_t fromX = (int32_t)from->GetX();
  int32_t fromY = (int32_t)from->GetY();
  int32_t toX = (int32_t)to->GetX();
  int32_t toY = (int32_t)to->GetY();
  FX_BOOL steep = FXSYS_abs(toY - fromY) > FXSYS_abs(toX - fromX);
  if (steep) {
    int32_t temp = fromX;
    fromX = fromY;
    fromY = temp;
    temp = toX;
    toX = toY;
    toY = temp;
  }
  int32_t dx = FXSYS_abs(toX - fromX);
  int32_t dy = FXSYS_abs(toY - fromY);
  int32_t error = -dx >> 1;
  int32_t ystep = fromY < toY ? 1 : -1;
  int32_t xstep = fromX < toX ? 1 : -1;
  int32_t transitions = 0;
  FX_BOOL inBlack = m_image->Get(steep ? fromY : fromX, steep ? fromX : fromY);
  for (int32_t x = fromX, y = fromY; x != toX; x += xstep) {
    FX_BOOL isBlack = m_image->Get(steep ? y : x, steep ? x : y);
    if (isBlack != inBlack) {
      transitions++;
      inBlack = isBlack;
    }
    error += dy;
    if (error > 0) {
      if (y == toY) {
        break;
      }
      y += ystep;
      error -= dx;
    }
  }
  return new CBC_ResultPointsAndTransitions(from, to, transitions);
}
void CBC_DataMatrixDetector::OrderBestPatterns(CFX_PtrArray* patterns) {
  FX_FLOAT abDistance = (FX_FLOAT)Distance((CBC_ResultPoint*)(*patterns)[0],
                                           (CBC_ResultPoint*)(*patterns)[1]);
  FX_FLOAT bcDistance = (FX_FLOAT)Distance((CBC_ResultPoint*)(*patterns)[1],
                                           (CBC_ResultPoint*)(*patterns)[2]);
  FX_FLOAT acDistance = (FX_FLOAT)Distance((CBC_ResultPoint*)(*patterns)[0],
                                           (CBC_ResultPoint*)(*patterns)[2]);
  CBC_ResultPoint *topLeft, *topRight, *bottomLeft;
  if (bcDistance >= abDistance && bcDistance >= acDistance) {
    topLeft = (CBC_ResultPoint*)(*patterns)[0];
    topRight = (CBC_ResultPoint*)(*patterns)[1];
    bottomLeft = (CBC_ResultPoint*)(*patterns)[2];
  } else if (acDistance >= bcDistance && acDistance >= abDistance) {
    topLeft = (CBC_ResultPoint*)(*patterns)[1];
    topRight = (CBC_ResultPoint*)(*patterns)[0];
    bottomLeft = (CBC_ResultPoint*)(*patterns)[2];
  } else {
    topLeft = (CBC_ResultPoint*)(*patterns)[2];
    topRight = (CBC_ResultPoint*)(*patterns)[0];
    bottomLeft = (CBC_ResultPoint*)(*patterns)[1];
  }
  if ((bottomLeft->GetY() - topLeft->GetY()) *
          (topRight->GetX() - topLeft->GetX()) <
      (bottomLeft->GetX() - topLeft->GetX()) *
          (topRight->GetY() - topLeft->GetY())) {
    CBC_ResultPoint* temp = topRight;
    topRight = bottomLeft;
    bottomLeft = temp;
  }
  (*patterns)[0] = bottomLeft;
  (*patterns)[1] = topLeft;
  (*patterns)[2] = topRight;
}
