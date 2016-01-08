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
#include "xfa/src/fxbarcode/common/BC_CommonBitMatrix.h"
#include "BC_QRAlignmentPattern.h"
#include "BC_QRAlignmentPatternFinder.h"
CBC_QRAlignmentPatternFinder::CBC_QRAlignmentPatternFinder(
    CBC_CommonBitMatrix* image,
    int32_t startX,
    int32_t startY,
    int32_t width,
    int32_t height,
    FX_FLOAT moduleSize)
    : m_image(image),
      m_startX(startX),
      m_startY(startY),
      m_width(width),
      m_height(height),
      m_moduleSize(moduleSize)

{
  m_crossCheckStateCount.SetSize(3);
}
CBC_QRAlignmentPatternFinder::~CBC_QRAlignmentPatternFinder() {
  for (int32_t i = 0; i < m_possibleCenters.GetSize(); i++) {
    delete (CBC_QRAlignmentPattern*)m_possibleCenters[i];
  }
  m_possibleCenters.RemoveAll();
}
CBC_QRAlignmentPattern* CBC_QRAlignmentPatternFinder::Find(int32_t& e) {
  int32_t startX = m_startX;
  int32_t height = m_height;
  int32_t maxJ = startX + m_width;
  int32_t middleI = m_startY + (height >> 1);
  CFX_Int32Array stateCount;
  stateCount.SetSize(3);
  for (int32_t iGen = 0; iGen < height; iGen++) {
    int32_t i =
        middleI + ((iGen & 0x01) == 0 ? ((iGen + 1) >> 1) : -((iGen + 1) >> 1));
    stateCount[0] = 0;
    stateCount[1] = 0;
    stateCount[2] = 0;
    int32_t j = startX;
    while (j < maxJ && !m_image->Get(j, i)) {
      j++;
    }
    int32_t currentState = 0;
    while (j < maxJ) {
      if (m_image->Get(j, i)) {
        if (currentState == 1) {
          stateCount[currentState]++;
        } else {
          if (currentState == 2) {
            if (FoundPatternCross(stateCount)) {
              CBC_QRAlignmentPattern* confirmed =
                  HandlePossibleCenter(stateCount, i, j);
              if (confirmed != NULL) {
                return confirmed;
              }
            }
            stateCount[0] = stateCount[2];
            stateCount[1] = 1;
            stateCount[2] = 0;
            currentState = 1;
          } else {
            stateCount[++currentState]++;
          }
        }
      } else {
        if (currentState == 1) {
          currentState++;
        }
        stateCount[currentState]++;
      }
      j++;
    }
    if (FoundPatternCross(stateCount)) {
      CBC_QRAlignmentPattern* confirmed =
          HandlePossibleCenter(stateCount, i, maxJ);
      if (confirmed != NULL) {
        return confirmed;
      }
    }
  }
  if (m_possibleCenters.GetSize() != 0) {
    return ((CBC_QRAlignmentPattern*)(m_possibleCenters[0]))->Clone();
  }
  e = BCExceptionRead;
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return NULL;
}
FX_FLOAT CBC_QRAlignmentPatternFinder::CenterFromEnd(
    const CFX_Int32Array& stateCount,
    int32_t end) {
  return (FX_FLOAT)(end - stateCount[2]) - stateCount[1] / 2.0f;
}
FX_BOOL CBC_QRAlignmentPatternFinder::FoundPatternCross(
    const CFX_Int32Array& stateCount) {
  FX_FLOAT moduleSize = m_moduleSize;
  FX_FLOAT maxVariance = moduleSize / 2.0f;
  for (int32_t i = 0; i < 3; i++) {
    if (fabs(moduleSize - stateCount[i]) >= maxVariance) {
      return false;
    }
  }
  return TRUE;
}
FX_FLOAT CBC_QRAlignmentPatternFinder::CrossCheckVertical(
    int32_t startI,
    int32_t centerJ,
    int32_t maxCount,
    int32_t originalStateCountTotal) {
  int32_t maxI = m_image->GetHeight();
  CFX_Int32Array stateCount;
  stateCount.Copy(m_crossCheckStateCount);
  stateCount[0] = 0;
  stateCount[1] = 0;
  stateCount[2] = 0;
  int32_t i = startI;
  while (i >= 0 && m_image->Get(centerJ, i) && stateCount[1] <= maxCount) {
    stateCount[1]++;
    i--;
  }
  if (i < 0 || stateCount[1] > maxCount) {
    return FXSYS_nan();
  }
  while (i >= 0 && !m_image->Get(centerJ, i) && stateCount[0] <= maxCount) {
    stateCount[0]++;
    i--;
  }
  if (stateCount[0] > maxCount) {
    return FXSYS_nan();
  }
  i = startI + 1;
  while (i < maxI && m_image->Get(centerJ, i) && stateCount[1] <= maxCount) {
    stateCount[1]++;
    i++;
  }
  if (i == maxI || stateCount[1] > maxCount) {
    return FXSYS_nan();
  }
  while (i < maxI && !m_image->Get(centerJ, i) && stateCount[2] <= maxCount) {
    stateCount[2]++;
    i++;
  }
  if (stateCount[2] > maxCount) {
    return FXSYS_nan();
  }
  int32_t stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2];
  if (5 * abs(stateCountTotal - originalStateCountTotal) >=
      originalStateCountTotal) {
    return FXSYS_nan();
  }
  return FoundPatternCross(stateCount) ? CenterFromEnd(stateCount, i)
                                       : FXSYS_nan();
}
CBC_QRAlignmentPattern* CBC_QRAlignmentPatternFinder::HandlePossibleCenter(
    const CFX_Int32Array& stateCount,
    int32_t i,
    int32_t j) {
  int32_t stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2];
  FX_FLOAT centerJ = CenterFromEnd(stateCount, j);
  FX_FLOAT centerI = CrossCheckVertical(i, (int32_t)centerJ, 2 * stateCount[1],
                                        stateCountTotal);
  if (!FXSYS_isnan(centerI)) {
    FX_FLOAT estimatedModuleSize =
        (FX_FLOAT)(stateCount[0] + stateCount[1] + stateCount[2]) / 3.0f;
    int32_t max = m_possibleCenters.GetSize();
    for (int32_t index = 0; index < max; index++) {
      CBC_QRAlignmentPattern* center =
          (CBC_QRAlignmentPattern*)(m_possibleCenters[index]);
      if (center->AboutEquals(estimatedModuleSize, centerI, centerJ)) {
        return new CBC_QRAlignmentPattern(centerJ, centerI,
                                          estimatedModuleSize);
      }
    }
    m_possibleCenters.Add(
        new CBC_QRAlignmentPattern(centerJ, centerI, estimatedModuleSize));
  }
  return NULL;
}
