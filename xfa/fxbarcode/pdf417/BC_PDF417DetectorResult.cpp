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

#include "xfa/fxbarcode/BC_ResultPoint.h"
#include "xfa/fxbarcode/common/BC_CommonBitMatrix.h"
#include "xfa/fxbarcode/pdf417/BC_PDF417DetectorResult.h"

CBC_PDF417DetectorResult::CBC_PDF417DetectorResult(
    CBC_CommonBitMatrix* bits,
    CBC_ResultPointArrayArray* points)
    : m_bits(bits), m_points(points) {}

CBC_PDF417DetectorResult::~CBC_PDF417DetectorResult() {
  for (int32_t i = 0; i < m_points->GetSize(); i++) {
    CBC_ResultPointArray* temp = m_points->GetAt(i);
    for (int32_t j = 0; j < temp->GetSize(); j++)
      delete temp->GetAt(j);

    delete temp;
  }
}

CBC_CommonBitMatrix* CBC_PDF417DetectorResult::getBits() const {
  return m_bits;
}

CBC_ResultPointArrayArray* CBC_PDF417DetectorResult::getPoints() const {
  return m_points.get();
}
