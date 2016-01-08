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
#include "xfa/src/fxbarcode/BC_ResultPoint.h"
#include "BC_QRFinderPattern.h"
CBC_QRFinderPattern::CBC_QRFinderPattern(FX_FLOAT x,
                                         FX_FLOAT posY,
                                         FX_FLOAT estimatedModuleSize)
    : CBC_ResultPoint(x, posY),
      m_estimatedModuleSize(estimatedModuleSize),
      m_count(1) {}
CBC_QRFinderPattern::~CBC_QRFinderPattern() {
  m_count = 0;
  m_x = 0.0f;
  m_y = 0.0f;
  m_estimatedModuleSize = 0.0f;
}
CBC_QRFinderPattern* CBC_QRFinderPattern::Clone() {
  CBC_QRFinderPattern* temp =
      new CBC_QRFinderPattern(m_x, m_y, m_estimatedModuleSize);
  temp->m_count = m_count;
  return temp;
}
FX_FLOAT CBC_QRFinderPattern::GetEstimatedModuleSize() {
  return m_estimatedModuleSize;
}
int32_t CBC_QRFinderPattern::GetCount() {
  return m_count;
}
void CBC_QRFinderPattern::IncrementCount() {
  m_count++;
}
FX_BOOL CBC_QRFinderPattern::AboutEquals(FX_FLOAT moduleSize,
                                         FX_FLOAT i,
                                         FX_FLOAT j) {
  if ((fabs(i - GetY()) <= moduleSize) && (fabs(j - GetX()) <= moduleSize)) {
    FX_FLOAT moduleSizeDiff = fabs(moduleSize - m_estimatedModuleSize);
    return (moduleSizeDiff <= 1.0f) ||
           (moduleSizeDiff / m_estimatedModuleSize <= 1.0f);
  }
  return false;
}
FX_FLOAT CBC_QRFinderPattern::GetX() {
  return m_x;
}
FX_FLOAT CBC_QRFinderPattern::GetY() {
  return m_y;
}
