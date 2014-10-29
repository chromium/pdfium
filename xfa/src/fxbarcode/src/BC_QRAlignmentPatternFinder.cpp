// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_ResultPoint.h"
#include "include/BC_QRAlignmentPattern.h"
#include "include/BC_QRAlignmentPatternFinder.h"
#include "include/BC_CommonBitMatrix.h"
CBC_QRAlignmentPatternFinder::CBC_QRAlignmentPatternFinder(CBC_CommonBitMatrix *image,
        FX_INT32 startX,
        FX_INT32 startY,
        FX_INT32 width,
        FX_INT32 height,
        FX_FLOAT moduleSize): m_image(image),
    m_startX(startX),
    m_startY(startY),
    m_width(width),
    m_height(height),
    m_moduleSize(moduleSize)

{
    m_crossCheckStateCount.SetSize(3);
}
CBC_QRAlignmentPatternFinder::~CBC_QRAlignmentPatternFinder()
{
    for (FX_INT32 i = 0; i < m_possibleCenters.GetSize(); i++) {
        delete (CBC_QRAlignmentPattern*)m_possibleCenters[i];
    }
    m_possibleCenters.RemoveAll();
}
CBC_QRAlignmentPattern  *CBC_QRAlignmentPatternFinder::Find(FX_INT32 &e)
{
    FX_INT32 startX = m_startX;
    FX_INT32 height = m_height;
    FX_INT32 maxJ = startX + m_width;
    FX_INT32 middleI = m_startY + (height >> 1);
    CFX_Int32Array stateCount;
    stateCount.SetSize(3);
    for (FX_INT32 iGen = 0; iGen < height; iGen++) {
        FX_INT32 i = middleI + ((iGen & 0x01) == 0 ? ((iGen + 1) >> 1) : -((iGen + 1) >> 1));
        stateCount[0] = 0;
        stateCount[1] = 0;
        stateCount[2] = 0;
        FX_INT32 j = startX;
        while (j < maxJ && !m_image->Get(j, i)) {
            j++;
        }
        FX_INT32 currentState = 0;
        while (j < maxJ) {
            if (m_image->Get(j, i)) {
                if (currentState == 1) {
                    stateCount[currentState]++;
                } else {
                    if (currentState == 2) {
                        if (FoundPatternCross(stateCount)) {
                            CBC_QRAlignmentPattern  *confirmed = HandlePossibleCenter(stateCount, i, j);
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
            CBC_QRAlignmentPattern *confirmed = HandlePossibleCenter(stateCount, i, maxJ);
            if (confirmed != NULL) {
                return confirmed;
            }
        }
    }
    if (m_possibleCenters.GetSize() != 0) {
        return	((CBC_QRAlignmentPattern*) (m_possibleCenters[0]) )->Clone();
    }
    e = BCExceptionRead;
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return NULL;
}
FX_FLOAT CBC_QRAlignmentPatternFinder::CenterFromEnd(const CFX_Int32Array &stateCount, FX_INT32 end)
{
    return (FX_FLOAT) (end - stateCount[2]) - stateCount[1] / 2.0f;
}
FX_BOOL CBC_QRAlignmentPatternFinder::FoundPatternCross(const CFX_Int32Array &stateCount)
{
    FX_FLOAT moduleSize = m_moduleSize;
    FX_FLOAT maxVariance = moduleSize / 2.0f;
    for (FX_INT32 i = 0; i < 3; i++) {
        if (fabs(moduleSize - stateCount[i]) >= maxVariance) {
            return false;
        }
    }
    return TRUE;
}
FX_FLOAT CBC_QRAlignmentPatternFinder::CrossCheckVertical(FX_INT32 startI, FX_INT32 centerJ, FX_INT32 maxCount, FX_INT32 originalStateCountTotal)
{
    CBC_CommonBitMatrix *image = m_image;
    FX_INT32 maxI = m_image->GetHeight();
    CFX_Int32Array stateCount;
    stateCount.Copy(m_crossCheckStateCount);
    stateCount[0] = 0;
    stateCount[1] = 0;
    stateCount[2] = 0;
    FX_INT32 i = startI;
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
    FX_INT32 stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2];
    if (5 * abs(stateCountTotal - originalStateCountTotal) >= originalStateCountTotal) {
        return FXSYS_nan();
    }
    return FoundPatternCross(stateCount) ? CenterFromEnd(stateCount, i) : FXSYS_nan();
}
CBC_QRAlignmentPattern *CBC_QRAlignmentPatternFinder::HandlePossibleCenter(const CFX_Int32Array &stateCount, FX_INT32 i, FX_INT32 j)
{
    FX_INT32 stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2];
    FX_FLOAT centerJ = CenterFromEnd(stateCount, j);
    FX_FLOAT centerI = CrossCheckVertical(i, (FX_INT32) centerJ, 2 * stateCount[1], stateCountTotal);
    if (!FXSYS_isnan(centerI)) {
        FX_FLOAT estimatedModuleSize = (FX_FLOAT) (stateCount[0] + stateCount[1] + stateCount[2]) / 3.0f;
        FX_INT32 max = m_possibleCenters.GetSize();
        for (FX_INT32 index = 0; index < max; index++) {
            CBC_QRAlignmentPattern *center = (CBC_QRAlignmentPattern *)(m_possibleCenters[index]);
            if (center->AboutEquals(estimatedModuleSize, centerI, centerJ)) {
                return FX_NEW CBC_QRAlignmentPattern(centerJ, centerI, estimatedModuleSize);
            }
        }
        m_possibleCenters.Add(FX_NEW CBC_QRAlignmentPattern(centerJ, centerI, estimatedModuleSize));
    }
    return NULL;
}
