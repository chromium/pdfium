// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2010 ZXing authors
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
#include "xfa/src/fxbarcode/common/BC_WhiteRectangleDetector.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitMatrix.h"
#include "xfa/src/fxbarcode/BC_ResultPoint.h"
const int32_t CBC_WhiteRectangleDetector::INIT_SIZE = 30;
const int32_t CBC_WhiteRectangleDetector::CORR = 1;
CBC_WhiteRectangleDetector::CBC_WhiteRectangleDetector(
    CBC_CommonBitMatrix* image) {
  m_image = image;
  m_height = image->GetHeight();
  m_width = image->GetWidth();
  m_leftInit = (m_width - INIT_SIZE) >> 1;
  m_rightInit = (m_width + INIT_SIZE) >> 1;
  m_upInit = (m_height - INIT_SIZE) >> 1;
  m_downInit = (m_height + INIT_SIZE) >> 1;
}
void CBC_WhiteRectangleDetector::Init(int32_t& e) {
  if (m_upInit < 0 || m_leftInit < 0 || m_downInit >= m_height ||
      m_rightInit >= m_width) {
    e = BCExceptionNotFound;
    BC_EXCEPTION_CHECK_ReturnVoid(e);
  }
}
CBC_WhiteRectangleDetector::CBC_WhiteRectangleDetector(
    CBC_CommonBitMatrix* image,
    int32_t initSize,
    int32_t x,
    int32_t y) {
  m_image = image;
  m_height = image->GetHeight();
  m_width = image->GetWidth();
  int32_t halfsize = initSize >> 1;
  m_leftInit = x - halfsize;
  m_rightInit = x + halfsize;
  m_upInit = y - halfsize;
  m_downInit = y + halfsize;
}
CBC_WhiteRectangleDetector::~CBC_WhiteRectangleDetector() {}
CFX_PtrArray* CBC_WhiteRectangleDetector::Detect(int32_t& e) {
  int32_t left = m_leftInit;
  int32_t right = m_rightInit;
  int32_t up = m_upInit;
  int32_t down = m_downInit;
  FX_BOOL sizeExceeded = FALSE;
  FX_BOOL aBlackPointFoundOnBorder = TRUE;
  FX_BOOL atLeastOneBlackPointFoundOnBorder = FALSE;
  while (aBlackPointFoundOnBorder) {
    aBlackPointFoundOnBorder = FALSE;
    FX_BOOL rightBorderNotWhite = TRUE;
    while (rightBorderNotWhite && right < m_width) {
      rightBorderNotWhite = ContainsBlackPoint(up, down, right, FALSE);
      if (rightBorderNotWhite) {
        right++;
        aBlackPointFoundOnBorder = TRUE;
      }
    }
    if (right >= m_width) {
      sizeExceeded = TRUE;
      break;
    }
    FX_BOOL bottomBorderNotWhite = TRUE;
    while (bottomBorderNotWhite && down < m_height) {
      bottomBorderNotWhite = ContainsBlackPoint(left, right, down, TRUE);
      if (bottomBorderNotWhite) {
        down++;
        aBlackPointFoundOnBorder = TRUE;
      }
    }
    if (down >= m_height) {
      sizeExceeded = TRUE;
      break;
    }
    FX_BOOL leftBorderNotWhite = TRUE;
    while (leftBorderNotWhite && left >= 0) {
      leftBorderNotWhite = ContainsBlackPoint(up, down, left, FALSE);
      if (leftBorderNotWhite) {
        left--;
        aBlackPointFoundOnBorder = TRUE;
      }
    }
    if (left < 0) {
      sizeExceeded = TRUE;
      break;
    }
    FX_BOOL topBorderNotWhite = TRUE;
    while (topBorderNotWhite && up >= 0) {
      topBorderNotWhite = ContainsBlackPoint(left, right, up, TRUE);
      if (topBorderNotWhite) {
        up--;
        aBlackPointFoundOnBorder = TRUE;
      }
    }
    if (up < 0) {
      sizeExceeded = TRUE;
      break;
    }
    if (aBlackPointFoundOnBorder) {
      atLeastOneBlackPointFoundOnBorder = TRUE;
    }
  }
  if (!sizeExceeded && atLeastOneBlackPointFoundOnBorder) {
    int32_t maxSize = right - left;
    CBC_AutoPtr<CBC_ResultPoint> z(NULL);
    for (int32_t i = 1; i < maxSize; i++) {
      z = CBC_AutoPtr<CBC_ResultPoint>(
          GetBlackPointOnSegment((FX_FLOAT)left, (FX_FLOAT)(down - i),
                                 (FX_FLOAT)(left + i), (FX_FLOAT)(down)));
      if (z.get() != NULL) {
        break;
      }
    }
    if (z.get() == NULL) {
      e = BCExceptionNotFound;
      BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    CBC_AutoPtr<CBC_ResultPoint> t(NULL);
    for (int32_t j = 1; j < maxSize; j++) {
      t = CBC_AutoPtr<CBC_ResultPoint>(
          GetBlackPointOnSegment((FX_FLOAT)left, (FX_FLOAT)(up + j),
                                 (FX_FLOAT)(left + j), (FX_FLOAT)up));
      if (t.get() != NULL) {
        break;
      }
    }
    if (t.get() == NULL) {
      e = BCExceptionNotFound;
      BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    CBC_AutoPtr<CBC_ResultPoint> x(NULL);
    for (int32_t k = 1; k < maxSize; k++) {
      x = CBC_AutoPtr<CBC_ResultPoint>(
          GetBlackPointOnSegment((FX_FLOAT)right, (FX_FLOAT)(up + k),
                                 (FX_FLOAT)(right - k), (FX_FLOAT)up));
      if (x.get() != NULL) {
        break;
      }
    }
    if (x.get() == NULL) {
      e = BCExceptionNotFound;
      BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    CBC_AutoPtr<CBC_ResultPoint> y(NULL);
    for (int32_t m = 1; m < maxSize; m++) {
      y = CBC_AutoPtr<CBC_ResultPoint>(
          GetBlackPointOnSegment((FX_FLOAT)right, (FX_FLOAT)(down - m),
                                 (FX_FLOAT)(right - m), (FX_FLOAT)down));
      if (y.get() != NULL) {
        break;
      }
    }
    if (y.get() == NULL) {
      e = BCExceptionNotFound;
      BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    return CenterEdges(y.get(), z.get(), x.get(), t.get());
  } else {
    e = BCExceptionNotFound;
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  }
  return NULL;
}
int32_t CBC_WhiteRectangleDetector::Round(FX_FLOAT d) {
  return (int32_t)(d + 0.5f);
}
CBC_ResultPoint* CBC_WhiteRectangleDetector::GetBlackPointOnSegment(
    FX_FLOAT aX,
    FX_FLOAT aY,
    FX_FLOAT bX,
    FX_FLOAT bY) {
  int32_t dist = DistanceL2(aX, aY, bX, bY);
  float xStep = (bX - aX) / dist;
  float yStep = (bY - aY) / dist;
  for (int32_t i = 0; i < dist; i++) {
    int32_t x = Round(aX + i * xStep);
    int32_t y = Round(aY + i * yStep);
    if (m_image->Get(x, y)) {
      return new CBC_ResultPoint((FX_FLOAT)x, (FX_FLOAT)y);
    }
  }
  return NULL;
}
int32_t CBC_WhiteRectangleDetector::DistanceL2(FX_FLOAT aX,
                                               FX_FLOAT aY,
                                               FX_FLOAT bX,
                                               FX_FLOAT bY) {
  float xDiff = aX - bX;
  float yDiff = aY - bY;
  return Round((float)sqrt(xDiff * xDiff + yDiff * yDiff));
}
CFX_PtrArray* CBC_WhiteRectangleDetector::CenterEdges(CBC_ResultPoint* y,
                                                      CBC_ResultPoint* z,
                                                      CBC_ResultPoint* x,
                                                      CBC_ResultPoint* t) {
  float yi = y->GetX();
  float yj = y->GetY();
  float zi = z->GetX();
  float zj = z->GetY();
  float xi = x->GetX();
  float xj = x->GetY();
  float ti = t->GetX();
  float tj = t->GetY();
  if (yi < m_width / 2) {
    CFX_PtrArray* result = new CFX_PtrArray;
    result->SetSize(4);
    (*result)[0] = new CBC_ResultPoint(ti - CORR, tj + CORR);
    (*result)[1] = new CBC_ResultPoint(zi + CORR, zj + CORR);
    (*result)[2] = new CBC_ResultPoint(xi - CORR, xj - CORR);
    (*result)[3] = new CBC_ResultPoint(yi + CORR, yj - CORR);
    return result;
  } else {
    CFX_PtrArray* result = new CFX_PtrArray;
    result->SetSize(4);
    (*result)[0] = new CBC_ResultPoint(ti + CORR, tj + CORR);
    (*result)[1] = new CBC_ResultPoint(zi + CORR, zj - CORR);
    (*result)[2] = new CBC_ResultPoint(xi - CORR, xj + CORR);
    (*result)[3] = new CBC_ResultPoint(yi - CORR, yj - CORR);
    return result;
  }
}
FX_BOOL CBC_WhiteRectangleDetector::ContainsBlackPoint(int32_t a,
                                                       int32_t b,
                                                       int32_t fixed,
                                                       FX_BOOL horizontal) {
  if (horizontal) {
    for (int32_t x = a; x <= b; x++) {
      if (m_image->Get(x, fixed)) {
        return TRUE;
      }
    }
  } else {
    for (int32_t y = a; y <= b; y++) {
      if (m_image->Get(fixed, y)) {
        return TRUE;
      }
    }
  }
  return FALSE;
}
