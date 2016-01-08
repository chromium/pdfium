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
#include "xfa/src/fxbarcode/common/BC_CommonPerspectiveTransform.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitMatrix.h"
#include "BC_QRGridSampler.h"
CBC_QRGridSampler CBC_QRGridSampler::m_gridSampler;
CBC_QRGridSampler::CBC_QRGridSampler() {}
CBC_QRGridSampler::~CBC_QRGridSampler() {}
CBC_QRGridSampler& CBC_QRGridSampler::GetInstance() {
  return m_gridSampler;
}
void CBC_QRGridSampler::CheckAndNudgePoints(CBC_CommonBitMatrix* image,
                                            CFX_FloatArray* points,
                                            int32_t& e) {
  int32_t width = image->GetWidth();
  int32_t height = image->GetHeight();
  FX_BOOL nudged = TRUE;
  int32_t offset;
  for (offset = 0; offset < points->GetSize() && nudged; offset += 2) {
    int32_t x = (int32_t)(*points)[offset];
    int32_t y = (int32_t)(*points)[offset + 1];
    if (x < -1 || x > width || y < -1 || y > height) {
      e = BCExceptionRead;
      BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
    nudged = FALSE;
    if (x == -1) {
      (*points)[offset] = 0.0f;
      nudged = TRUE;
    } else if (x == width) {
      (*points)[offset] = (FX_FLOAT)(width - 1);
      nudged = TRUE;
    }
    if (y == -1) {
      (*points)[offset + 1] = 0.0f;
      nudged = TRUE;
    } else if (y == height) {
      (*points)[offset + 1] = (FX_FLOAT)(height - 1);
      nudged = TRUE;
    }
  }
  nudged = TRUE;
  for (offset = (*points).GetSize() - 2; offset >= 0 && nudged; offset -= 2) {
    int32_t x = (int32_t)(*points)[offset];
    int32_t y = (int32_t)(*points)[offset + 1];
    if (x < -1 || x > width || y < -1 || y > height) {
      e = BCExceptionRead;
      BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
    nudged = FALSE;
    if (x == -1) {
      (*points)[offset] = 0.0f;
      nudged = TRUE;
    } else if (x == width) {
      (*points)[offset] = (FX_FLOAT)(width - 1);
      nudged = TRUE;
    }
    if (y == -1) {
      (*points)[offset + 1] = 0.0f;
      nudged = TRUE;
    } else if (y == height) {
      (*points)[offset + 1] = (FX_FLOAT)(height - 1);
      nudged = TRUE;
    }
  }
}
CBC_CommonBitMatrix* CBC_QRGridSampler::SampleGrid(CBC_CommonBitMatrix* image,
                                                   int32_t dimensionX,
                                                   int32_t dimensionY,
                                                   FX_FLOAT p1ToX,
                                                   FX_FLOAT p1ToY,
                                                   FX_FLOAT p2ToX,
                                                   FX_FLOAT p2ToY,
                                                   FX_FLOAT p3ToX,
                                                   FX_FLOAT p3ToY,
                                                   FX_FLOAT p4ToX,
                                                   FX_FLOAT p4ToY,
                                                   FX_FLOAT p1FromX,
                                                   FX_FLOAT p1FromY,
                                                   FX_FLOAT p2FromX,
                                                   FX_FLOAT p2FromY,
                                                   FX_FLOAT p3FromX,
                                                   FX_FLOAT p3FromY,
                                                   FX_FLOAT p4FromX,
                                                   FX_FLOAT p4FromY,
                                                   int32_t& e) {
  CBC_AutoPtr<CBC_CommonPerspectiveTransform> transform(
      CBC_CommonPerspectiveTransform::QuadrilateralToQuadrilateral(
          p1ToX, p1ToY, p2ToX, p2ToY, p3ToX, p3ToY, p4ToX, p4ToY, p1FromX,
          p1FromY, p2FromX, p2FromY, p3FromX, p3FromY, p4FromX, p4FromY));
  CBC_CommonBitMatrix* tempBitM = new CBC_CommonBitMatrix();
  tempBitM->Init(dimensionX, dimensionY);
  CBC_AutoPtr<CBC_CommonBitMatrix> bits(tempBitM);
  CFX_FloatArray points;
  points.SetSize(dimensionX << 1);
  for (int32_t y = 0; y < dimensionY; y++) {
    int32_t max = points.GetSize();
    FX_FLOAT iValue = (FX_FLOAT)(y + 0.5f);
    int32_t x;
    for (x = 0; x < max; x += 2) {
      points[x] = (FX_FLOAT)((x >> 1) + 0.5f);
      points[x + 1] = iValue;
    }
    transform->TransformPoints(&points);
    CheckAndNudgePoints(image, &points, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    for (x = 0; x < max; x += 2) {
      if (image->Get((int32_t)points[x], (int32_t)points[x + 1])) {
        bits->Set(x >> 1, y);
      }
    }
  }
  return bits.release();
}
