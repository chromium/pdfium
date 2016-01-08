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
#include "BC_QRFinderPatternFinder.h"
#include "BC_FinderPatternInfo.h"
#include "BC_QRFinderPattern.h"
const int32_t CBC_QRFinderPatternFinder::CENTER_QUORUM = 2;
const int32_t CBC_QRFinderPatternFinder::MIN_SKIP = 3;
const int32_t CBC_QRFinderPatternFinder::MAX_MODULES = 57;
const int32_t CBC_QRFinderPatternFinder::INTEGER_MATH_SHIFT = 8;
CBC_QRFinderPatternFinder::CBC_QRFinderPatternFinder(
    CBC_CommonBitMatrix* image) {
  m_image = image;
  m_crossCheckStateCount.SetSize(5);
  m_hasSkipped = FALSE;
}
CBC_QRFinderPatternFinder::~CBC_QRFinderPatternFinder() {
  for (int32_t i = 0; i < m_possibleCenters.GetSize(); i++) {
    delete (CBC_QRFinderPattern*)m_possibleCenters[i];
  }
  m_possibleCenters.RemoveAll();
}
class ClosestToAverageComparator {
 private:
  FX_FLOAT m_averageModuleSize;

 public:
  ClosestToAverageComparator(FX_FLOAT averageModuleSize)
      : m_averageModuleSize(averageModuleSize) {}
  int32_t operator()(FinderPattern* a, FinderPattern* b) {
    FX_FLOAT dA =
        (FX_FLOAT)fabs(a->GetEstimatedModuleSize() - m_averageModuleSize);
    FX_FLOAT dB =
        (FX_FLOAT)fabs(b->GetEstimatedModuleSize() - m_averageModuleSize);
    return dA < dB ? -1 : dA > dB ? 1 : 0;
  }
};
class CenterComparator {
 public:
  int32_t operator()(FinderPattern* a, FinderPattern* b) {
    return b->GetCount() - a->GetCount();
  }
};
CBC_CommonBitMatrix* CBC_QRFinderPatternFinder::GetImage() {
  return m_image;
}
CFX_Int32Array& CBC_QRFinderPatternFinder::GetCrossCheckStateCount() {
  m_crossCheckStateCount[0] = 0;
  m_crossCheckStateCount[1] = 0;
  m_crossCheckStateCount[2] = 0;
  m_crossCheckStateCount[3] = 0;
  m_crossCheckStateCount[4] = 0;
  return m_crossCheckStateCount;
}
CFX_PtrArray* CBC_QRFinderPatternFinder::GetPossibleCenters() {
  return &m_possibleCenters;
}
CBC_QRFinderPatternInfo* CBC_QRFinderPatternFinder::Find(int32_t hint,
                                                         int32_t& e) {
  int32_t maxI = m_image->GetHeight();
  int32_t maxJ = m_image->GetWidth();
  int32_t iSkip = (3 * maxI) / (4 * MAX_MODULES);
  if (iSkip < MIN_SKIP || 0) {
    iSkip = MIN_SKIP;
  }
  FX_BOOL done = FALSE;
  CFX_Int32Array stateCount;
  stateCount.SetSize(5);
  for (int32_t i = iSkip - 1; i < maxI && !done; i += iSkip) {
    stateCount[0] = 0;
    stateCount[1] = 0;
    stateCount[2] = 0;
    stateCount[3] = 0;
    stateCount[4] = 0;
    int32_t currentState = 0;
    for (int32_t j = 0; j < maxJ; j++) {
      if (m_image->Get(j, i)) {
        if ((currentState & 1) == 1) {
          currentState++;
        }
        stateCount[currentState]++;
      } else {
        if ((currentState & 1) == 0) {
          if (currentState == 4) {
            if (FoundPatternCross(stateCount)) {
              FX_BOOL confirmed = HandlePossibleCenter(stateCount, i, j);
              if (confirmed) {
                iSkip = 2;
                if (m_hasSkipped) {
                  done = HaveMultiplyConfirmedCenters();
                } else {
                  int32_t rowSkip = FindRowSkip();
                  if (rowSkip > stateCount[2]) {
                    i += rowSkip - stateCount[2] - iSkip;
                    j = maxJ - 1;
                  }
                }
              } else {
                do {
                  j++;
                } while (j < maxJ && !m_image->Get(j, i));
                j--;
              }
              currentState = 0;
              stateCount[0] = 0;
              stateCount[1] = 0;
              stateCount[2] = 0;
              stateCount[3] = 0;
              stateCount[4] = 0;
            } else {
              stateCount[0] = stateCount[2];
              stateCount[1] = stateCount[3];
              stateCount[2] = stateCount[4];
              stateCount[3] = 1;
              stateCount[4] = 0;
              currentState = 3;
            }
          } else {
            stateCount[++currentState]++;
          }
        } else {
          stateCount[currentState]++;
        }
      }
    }
    if (FoundPatternCross(stateCount)) {
      FX_BOOL confirmed = HandlePossibleCenter(stateCount, i, maxJ);
      if (confirmed) {
        iSkip = stateCount[0];
        if (m_hasSkipped) {
          done = HaveMultiplyConfirmedCenters();
        }
      }
    }
  }
  CFX_PtrArray* ptr = SelectBestpatterns(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  CBC_AutoPtr<CFX_PtrArray> patternInfo(ptr);
  OrderBestPatterns(patternInfo.get());
  return new CBC_QRFinderPatternInfo(patternInfo.get());
}
void CBC_QRFinderPatternFinder::OrderBestPatterns(CFX_PtrArray* patterns) {
  FX_FLOAT abDistance = Distance((CBC_ResultPoint*)(*patterns)[0],
                                 (CBC_ResultPoint*)(*patterns)[1]);
  FX_FLOAT bcDistance = Distance((CBC_ResultPoint*)(*patterns)[1],
                                 (CBC_ResultPoint*)(*patterns)[2]);
  FX_FLOAT acDistance = Distance((CBC_ResultPoint*)(*patterns)[0],
                                 (CBC_ResultPoint*)(*patterns)[2]);
  CBC_QRFinderPattern *topLeft, *topRight, *bottomLeft;
  if (bcDistance >= abDistance && bcDistance >= acDistance) {
    topLeft = (CBC_QRFinderPattern*)(*patterns)[0];
    topRight = (CBC_QRFinderPattern*)(*patterns)[1];
    bottomLeft = (CBC_QRFinderPattern*)(*patterns)[2];
  } else if (acDistance >= bcDistance && acDistance >= abDistance) {
    topLeft = (CBC_QRFinderPattern*)(*patterns)[1];
    topRight = (CBC_QRFinderPattern*)(*patterns)[0];
    bottomLeft = (CBC_QRFinderPattern*)(*patterns)[2];
  } else {
    topLeft = (CBC_QRFinderPattern*)(*patterns)[2];
    topRight = (CBC_QRFinderPattern*)(*patterns)[0];
    bottomLeft = (CBC_QRFinderPattern*)(*patterns)[1];
  }
  if ((bottomLeft->GetY() - topLeft->GetY()) *
          (topRight->GetX() - topLeft->GetX()) <
      (bottomLeft->GetX() - topLeft->GetX()) *
          (topRight->GetY() - topLeft->GetY())) {
    CBC_QRFinderPattern* temp = topRight;
    topRight = bottomLeft;
    bottomLeft = temp;
  }
  (*patterns)[0] = bottomLeft;
  (*patterns)[1] = topLeft;
  (*patterns)[2] = topRight;
}
FX_FLOAT CBC_QRFinderPatternFinder::Distance(CBC_ResultPoint* point1,
                                             CBC_ResultPoint* point2) {
  FX_FLOAT dx = point1->GetX() - point2->GetX();
  FX_FLOAT dy = point1->GetY() - point2->GetY();
  return (FX_FLOAT)FXSYS_sqrt(dx * dx + dy * dy);
}
FX_FLOAT CBC_QRFinderPatternFinder::CenterFromEnd(
    const CFX_Int32Array& stateCount,
    int32_t end) {
  return (FX_FLOAT)(end - stateCount[4] - stateCount[3]) - stateCount[2] / 2.0f;
}
FX_BOOL CBC_QRFinderPatternFinder::FoundPatternCross(
    const CFX_Int32Array& stateCount) {
  int32_t totalModuleSize = 0;
  for (int32_t i = 0; i < 5; i++) {
    int32_t count = stateCount[i];
    if (count == 0) {
      return FALSE;
    }
    totalModuleSize += count;
  }
  if (totalModuleSize < 7) {
    return FALSE;
  }
  int32_t moduleSize = (totalModuleSize << INTEGER_MATH_SHIFT) / 7;
  int32_t maxVariance = moduleSize / 2;
  return FXSYS_abs(moduleSize - (stateCount[0] << INTEGER_MATH_SHIFT)) <
             maxVariance &&
         FXSYS_abs(moduleSize - (stateCount[1] << INTEGER_MATH_SHIFT)) <
             maxVariance &&
         FXSYS_abs(3 * moduleSize - (stateCount[2] << INTEGER_MATH_SHIFT)) <
             3 * maxVariance &&
         FXSYS_abs(moduleSize - (stateCount[3] << INTEGER_MATH_SHIFT)) <
             maxVariance &&
         FXSYS_abs(moduleSize - (stateCount[4] << INTEGER_MATH_SHIFT)) <
             maxVariance;
}
FX_FLOAT CBC_QRFinderPatternFinder::CrossCheckVertical(
    int32_t startI,
    int32_t centerJ,
    int32_t maxCount,
    int32_t originalStateCountTotal) {
  CBC_CommonBitMatrix* image = m_image;
  int32_t maxI = image->GetHeight();
  CFX_Int32Array& stateCount = GetCrossCheckStateCount();
  int32_t i = startI;
  while (i >= 0 && image->Get(centerJ, i)) {
    stateCount[2]++;
    i--;
  }
  if (i < 0) {
    return FXSYS_nan();
  }
  while (i >= 0 && !image->Get(centerJ, i) && stateCount[1] <= maxCount) {
    stateCount[1]++;
    i--;
  }
  if (i < 0 || stateCount[1] > maxCount) {
    return FXSYS_nan();
  }
  while (i >= 0 && image->Get(centerJ, i) && stateCount[0] <= maxCount) {
    stateCount[0]++;
    i--;
  }
  if (stateCount[0] > maxCount) {
    return FXSYS_nan();
  }
  i = startI + 1;
  while (i < maxI && image->Get(centerJ, i)) {
    stateCount[2]++;
    i++;
  }
  if (i == maxI) {
    return FXSYS_nan();
  }
  while (i < maxI && !image->Get(centerJ, i) && stateCount[3] < maxCount) {
    stateCount[3]++;
    i++;
  }
  if (i == maxI || stateCount[3] >= maxCount) {
    return FXSYS_nan();
  }
  while (i < maxI && image->Get(centerJ, i) && stateCount[4] < maxCount) {
    stateCount[4]++;
    i++;
  }
  if (stateCount[4] >= maxCount) {
    return FXSYS_nan();
  }
  int32_t stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] +
                            stateCount[3] + stateCount[4];
  if (5 * FXSYS_abs(stateCountTotal - originalStateCountTotal) >=
      originalStateCountTotal) {
    return FXSYS_nan();
  }
  return FoundPatternCross(stateCount) ? CenterFromEnd(stateCount, i)
                                       : FXSYS_nan();
}
FX_FLOAT CBC_QRFinderPatternFinder::CrossCheckHorizontal(
    int32_t startJ,
    int32_t centerI,
    int32_t maxCount,
    int32_t originalStateCountTotal) {
  CBC_CommonBitMatrix* image = m_image;
  int32_t maxJ = image->GetWidth();
  CFX_Int32Array& stateCount = GetCrossCheckStateCount();
  int32_t j = startJ;
  while (j >= 0 && image->Get(j, centerI)) {
    stateCount[2]++;
    j--;
  }
  if (j < 0) {
    return FXSYS_nan();
  }
  while (j >= 0 && !image->Get(j, centerI) && stateCount[1] <= maxCount) {
    stateCount[1]++;
    j--;
  }
  if (j < 0 || stateCount[1] > maxCount) {
    return FXSYS_nan();
  }
  while (j >= 0 && image->Get(j, centerI) && stateCount[0] <= maxCount) {
    stateCount[0]++;
    j--;
  }
  if (stateCount[0] > maxCount) {
    return FXSYS_nan();
  }
  j = startJ + 1;
  while (j < maxJ && image->Get(j, centerI)) {
    stateCount[2]++;
    j++;
  }
  if (j == maxJ) {
    return FXSYS_nan();
  }
  while (j < maxJ && !image->Get(j, centerI) && stateCount[3] < maxCount) {
    stateCount[3]++;
    j++;
  }
  if (j == maxJ || stateCount[3] >= maxCount) {
    return FXSYS_nan();
  }
  while (j < maxJ && image->Get(j, centerI) && stateCount[4] < maxCount) {
    stateCount[4]++;
    j++;
  }
  if (stateCount[4] >= maxCount) {
    return FXSYS_nan();
  }
  int32_t stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] +
                            stateCount[3] + stateCount[4];
  if (5 * FXSYS_abs(stateCountTotal - originalStateCountTotal) >=
      originalStateCountTotal) {
    return FXSYS_nan();
  }
  return FoundPatternCross(stateCount) ? CenterFromEnd(stateCount, j)
                                       : FXSYS_nan();
}
FX_BOOL CBC_QRFinderPatternFinder::HandlePossibleCenter(
    const CFX_Int32Array& stateCount,
    int32_t i,
    int32_t j) {
  int32_t stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] +
                            stateCount[3] + stateCount[4];
  FX_FLOAT centerJ = CenterFromEnd(stateCount, j);
  FX_FLOAT centerI =
      CrossCheckVertical(i, (int32_t)centerJ, stateCount[2], stateCountTotal);
  if (!FXSYS_isnan(centerI)) {
    centerJ = CrossCheckHorizontal((int32_t)centerJ, (int32_t)centerI,
                                   stateCount[2], stateCountTotal);
    if (!FXSYS_isnan(centerJ)) {
      FX_FLOAT estimatedModuleSize = (FX_FLOAT)stateCountTotal / 7.0f;
      FX_BOOL found = FALSE;
      int32_t max = m_possibleCenters.GetSize();
      for (int32_t index = 0; index < max; index++) {
        CBC_QRFinderPattern* center =
            (CBC_QRFinderPattern*)(m_possibleCenters[index]);
        if (center->AboutEquals(estimatedModuleSize, centerI, centerJ)) {
          center->IncrementCount();
          found = TRUE;
          break;
        }
      }
      if (!found) {
        m_possibleCenters.Add(
            new CBC_QRFinderPattern(centerJ, centerI, estimatedModuleSize));
      }
      return TRUE;
    }
  }
  return FALSE;
}
int32_t CBC_QRFinderPatternFinder::FindRowSkip() {
  int32_t max = m_possibleCenters.GetSize();
  if (max <= 1) {
    return 0;
  }
  FinderPattern* firstConfirmedCenter = NULL;
  for (int32_t i = 0; i < max; i++) {
    CBC_QRFinderPattern* center = (CBC_QRFinderPattern*)m_possibleCenters[i];
    if (center->GetCount() >= CENTER_QUORUM) {
      if (firstConfirmedCenter == NULL) {
        firstConfirmedCenter = center;
      } else {
        m_hasSkipped = TRUE;
        return (int32_t)((fabs(firstConfirmedCenter->GetX() - center->GetX()) -
                          fabs(firstConfirmedCenter->GetY() - center->GetY())) /
                         2);
      }
    }
  }
  return 0;
}
FX_BOOL CBC_QRFinderPatternFinder::HaveMultiplyConfirmedCenters() {
  int32_t confirmedCount = 0;
  FX_FLOAT totalModuleSize = 0.0f;
  int32_t max = m_possibleCenters.GetSize();
  int32_t i;
  for (i = 0; i < max; i++) {
    CBC_QRFinderPattern* pattern = (CBC_QRFinderPattern*)m_possibleCenters[i];
    if (pattern->GetCount() >= CENTER_QUORUM) {
      confirmedCount++;
      totalModuleSize += pattern->GetEstimatedModuleSize();
    }
  }
  if (confirmedCount < 3) {
    return FALSE;
  }
  FX_FLOAT average = totalModuleSize / (FX_FLOAT)max;
  FX_FLOAT totalDeviation = 0.0f;
  for (i = 0; i < max; i++) {
    CBC_QRFinderPattern* pattern = (CBC_QRFinderPattern*)m_possibleCenters[i];
    totalDeviation += fabs(pattern->GetEstimatedModuleSize() - average);
  }
  return totalDeviation <= 0.05f * totalModuleSize;
}
inline FX_BOOL centerComparator(void* a, void* b) {
  return ((CBC_QRFinderPattern*)b)->GetCount() <
         ((CBC_QRFinderPattern*)a)->GetCount();
}
CFX_PtrArray* CBC_QRFinderPatternFinder::SelectBestpatterns(int32_t& e) {
  int32_t startSize = m_possibleCenters.GetSize();
  if (m_possibleCenters.GetSize() < 3) {
    e = BCExceptionRead;
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  }
  FX_FLOAT average = 0.0f;
  if (startSize > 3) {
    FX_FLOAT totalModuleSize = 0.0f;
    for (int32_t i = 0; i < startSize; i++) {
      totalModuleSize += ((CBC_QRFinderPattern*)m_possibleCenters[i])
                             ->GetEstimatedModuleSize();
    }
    average = totalModuleSize / (FX_FLOAT)startSize;
    for (int32_t j = 0;
         j < m_possibleCenters.GetSize() && m_possibleCenters.GetSize() > 3;
         j++) {
      CBC_QRFinderPattern* pattern = (CBC_QRFinderPattern*)m_possibleCenters[j];
      if (fabs(pattern->GetEstimatedModuleSize() - average) > 0.2f * average) {
        delete pattern;
        m_possibleCenters.RemoveAt(j);
        j--;
      }
    }
  }
  if (m_possibleCenters.GetSize() > 3) {
    BC_FX_PtrArray_Sort(m_possibleCenters, centerComparator);
  }
  CFX_PtrArray* vec = new CFX_PtrArray();
  vec->SetSize(3);
  (*vec)[0] = ((CBC_QRFinderPattern*)m_possibleCenters[0])->Clone();
  (*vec)[1] = ((CBC_QRFinderPattern*)m_possibleCenters[1])->Clone();
  (*vec)[2] = ((CBC_QRFinderPattern*)m_possibleCenters[2])->Clone();
  return vec;
}
