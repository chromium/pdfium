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
#include "BC_FinderPatternInfo.h"
CBC_QRFinderPatternInfo::CBC_QRFinderPatternInfo(CFX_PtrArray* patternCenters) {
  m_bottomLeft = (CBC_QRFinderPattern*)(*patternCenters)[0];
  m_topLeft = (CBC_QRFinderPattern*)(*patternCenters)[1];
  m_topRight = (CBC_QRFinderPattern*)(*patternCenters)[2];
}
CBC_QRFinderPatternInfo::~CBC_QRFinderPatternInfo() {}
CBC_QRFinderPattern* CBC_QRFinderPatternInfo::GetBottomLeft() {
  return m_bottomLeft;
}
CBC_QRFinderPattern* CBC_QRFinderPatternInfo::GetTopLeft() {
  return m_topLeft;
}
CBC_QRFinderPattern* CBC_QRFinderPatternInfo::GetTopRight() {
  return m_topRight;
}
