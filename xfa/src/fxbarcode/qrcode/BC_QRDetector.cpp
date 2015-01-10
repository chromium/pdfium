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

#include "../barcode.h"
#include "../common/BC_CommonBitMatrix.h"
#include "../BC_ResultPoint.h"
#include "BC_QRFinderPattern.h"
#include "BC_QRCoderVersion.h"
#include "BC_FinderPatternInfo.h"
#include "BC_QRGridSampler.h"
#include "BC_QRAlignmentPatternFinder.h"
#include "BC_QRFinderPatternFinder.h"
#include "BC_QRDetectorResult.h"
#include "BC_QRDetector.h"
CBC_QRDetector::CBC_QRDetector(CBC_CommonBitMatrix *image): m_image(image)
{
}
CBC_QRDetector::~CBC_QRDetector()
{
}
CBC_QRDetectorResult *CBC_QRDetector::Detect(FX_INT32 hints, FX_INT32 &e)
{
    CBC_QRFinderPatternFinder finder(m_image);
    CBC_QRFinderPatternInfo* qpi = finder.Find(hints, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    CBC_AutoPtr<CBC_QRFinderPatternInfo> info(qpi);
    CBC_QRDetectorResult* qdr = ProcessFinderPatternInfo(info.get(), e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return qdr;
}
CBC_QRDetectorResult* CBC_QRDetector::ProcessFinderPatternInfo(CBC_QRFinderPatternInfo *info, FX_INT32 &e)
{
    CBC_AutoPtr<CBC_QRFinderPattern> topLeft(info->GetTopLeft());
    CBC_AutoPtr<CBC_QRFinderPattern> topRight(info->GetTopRight());
    CBC_AutoPtr<CBC_QRFinderPattern> bottomLeft(info->GetBottomLeft());
    FX_FLOAT moduleSize = CalculateModuleSize(topLeft.get(), topRight.get(), bottomLeft.get());
    if(moduleSize < 1.0f) {
        e = BCExceptionRead;
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    FX_INT32 dimension = ComputeDimension(topLeft.get(), topRight.get(), bottomLeft.get(), moduleSize, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    CBC_QRCoderVersion *provisionalVersion = CBC_QRCoderVersion::GetProvisionalVersionForDimension(dimension, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    FX_INT32 modulesBetweenFPCenters = provisionalVersion->GetDimensionForVersion() - 7;
    CBC_QRAlignmentPattern * alignmentPattern = NULL;
    if(provisionalVersion->GetAlignmentPatternCenters()->GetSize() > 0) {
        FX_FLOAT  bottomRightX = topRight->GetX() - topLeft->GetX() + bottomLeft->GetX();
        FX_FLOAT bottomRightY = topRight->GetY() - topLeft->GetY() + bottomLeft->GetY();
        FX_FLOAT correctionToTopLeft = 1.0f - 3.0f / (FX_FLOAT) modulesBetweenFPCenters;
        FX_FLOAT xtemp = (topLeft->GetX() + correctionToTopLeft * (bottomRightX - topLeft->GetX()));
        FX_INT32 estAlignmentX = (FX_INT32)xtemp ;
        FX_FLOAT ytemp =  (topLeft->GetY() + correctionToTopLeft * (bottomRightY - topLeft->GetY()));
        FX_INT32 estAlignmentY = (FX_INT32)ytemp;
        for(FX_INT32 i = 4; i <= 16; i <<= 1) {
            CBC_QRAlignmentPattern *temp = FindAlignmentInRegion(moduleSize, estAlignmentX, estAlignmentY, (FX_FLOAT) i, e);
            alignmentPattern = temp;
            break;
        }
    }
    CBC_CommonBitMatrix *bits = SampleGrid(m_image, topLeft.get(), topRight.get(), bottomLeft.get(), (CBC_ResultPoint*)(alignmentPattern), dimension, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    CFX_PtrArray *points = FX_NEW CFX_PtrArray;
    if(alignmentPattern == NULL) {
        points->Add(bottomLeft.release());
        points->Add(topLeft.release());
        points->Add(topRight.release());
    } else {
        points->Add(bottomLeft.release());
        points->Add(topLeft.release());
        points->Add(topRight.release());
        points->Add(alignmentPattern);
    }
    return FX_NEW CBC_QRDetectorResult(bits, points);
}
CBC_CommonBitMatrix *CBC_QRDetector::SampleGrid(CBC_CommonBitMatrix *image, CBC_ResultPoint *topLeft, CBC_ResultPoint *topRight,
        CBC_ResultPoint *bottomLeft, CBC_ResultPoint* alignmentPattern,
        FX_INT32 dimension, FX_INT32 &e)
{
    FX_FLOAT dimMinusThree = (FX_FLOAT) dimension - 3.5f;
    FX_FLOAT bottomRightX;
    FX_FLOAT bottomRightY;
    FX_FLOAT sourceBottomRightX;
    FX_FLOAT sourceBottomRightY;
    if (alignmentPattern != NULL) {
        bottomRightX = alignmentPattern->GetX();
        bottomRightY = alignmentPattern->GetY();
        sourceBottomRightX = sourceBottomRightY = dimMinusThree - 3.0f;
    } else {
        bottomRightX = (topRight->GetX() - topLeft->GetX()) + bottomLeft->GetX();
        bottomRightY = (topRight->GetY() - topLeft->GetY()) + bottomLeft->GetY();
        sourceBottomRightX = sourceBottomRightY = dimMinusThree;
    }
    CBC_QRGridSampler &sampler = CBC_QRGridSampler::GetInstance();
    CBC_CommonBitMatrix* cbm = sampler.SampleGrid(image,
                               dimension, dimension,
                               3.5f,
                               3.5f,
                               dimMinusThree,
                               3.5f,
                               sourceBottomRightX,
                               sourceBottomRightY,
                               3.5f,
                               dimMinusThree,
                               topLeft->GetX(),
                               topLeft->GetY(),
                               topRight->GetX(),
                               topRight->GetY(),
                               bottomRightX,
                               bottomRightY,
                               bottomLeft->GetX(),
                               bottomLeft->GetY(), e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return cbm;
}
FX_INT32 CBC_QRDetector::ComputeDimension(CBC_ResultPoint *topLeft, CBC_ResultPoint *topRight,
        CBC_ResultPoint *bottomLeft, FX_FLOAT moduleSize, FX_INT32 &e)
{
    FX_INT32 tltrCentersDimension = Round(CBC_QRFinderPatternFinder::Distance(topLeft, topRight) / moduleSize);
    FX_INT32 tlblCentersDimension = Round(CBC_QRFinderPatternFinder::Distance(topLeft, bottomLeft) / moduleSize);
    FX_INT32 dimension = ((tltrCentersDimension + tlblCentersDimension) >> 1) + 7;
    switch(dimension & 0x03) {
        case 0:
            dimension++;
            break;
        case 2:
            dimension--;
            break;
        case 3: {
                e = BCExceptionRead;
                BC_EXCEPTION_CHECK_ReturnValue(e, 0);
            }
    }
    return dimension;
}
FX_FLOAT CBC_QRDetector::CalculateModuleSize(CBC_ResultPoint *topLeft, CBC_ResultPoint *topRight, CBC_ResultPoint *bottomLeft)
{
    return (CalculateModuleSizeOneWay(topLeft, topRight) + CalculateModuleSizeOneWay(topLeft, bottomLeft)) / 2.0f;
}
FX_FLOAT CBC_QRDetector::CalculateModuleSizeOneWay(CBC_ResultPoint *pattern, CBC_ResultPoint *otherPattern)
{
    FX_FLOAT moduleSizeEst1 = SizeOfBlackWhiteBlackRunBothWays((FX_INT32) pattern->GetX(),
                              (FX_INT32) pattern->GetY(),
                              (FX_INT32) otherPattern->GetX(),
                              (FX_INT32) otherPattern->GetY());
    FX_FLOAT moduleSizeEst2 = SizeOfBlackWhiteBlackRunBothWays((FX_INT32) otherPattern->GetX(),
                              (FX_INT32) otherPattern->GetY(),
                              (FX_INT32) pattern->GetX(),
                              (FX_INT32) pattern->GetY());
    if (FXSYS_isnan(moduleSizeEst1)) {
        return moduleSizeEst2;
    }
    if (FXSYS_isnan(moduleSizeEst2)) {
        return moduleSizeEst1;
    }
    return (moduleSizeEst1 + moduleSizeEst2) / 14.0f;
}
FX_INT32 CBC_QRDetector::Round(FX_FLOAT d)
{
    return (FX_INT32)(d + 0.5f);
}
FX_FLOAT CBC_QRDetector::SizeOfBlackWhiteBlackRunBothWays(FX_INT32 fromX, FX_INT32 fromY, FX_INT32 toX, FX_INT32 toY)
{
    FX_FLOAT result = SizeOfBlackWhiteBlackRun(fromX, fromY, toX, toY);
    FX_INT32 otherToX = fromX - (toX - fromX);
    if (otherToX < 0) {
        otherToX = -1;
    } else if (otherToX >= m_image->GetWidth()) {
        otherToX = m_image->GetWidth();
    }
    FX_INT32 otherToY = fromY - (toY - fromY);
    if (otherToY < 0) {
        otherToY = -1;
    } else if (otherToY >= m_image->GetHeight()) {
        otherToY = m_image->GetHeight();
    }
    result += SizeOfBlackWhiteBlackRun(fromX, fromY, otherToX, otherToY);
    return result - 1.0f;
}
FX_FLOAT CBC_QRDetector::SizeOfBlackWhiteBlackRun(FX_INT32 fromX, FX_INT32 fromY, FX_INT32 toX, FX_INT32 toY)
{
    FX_BOOL steep = FXSYS_abs(toY - fromY) > FXSYS_abs(toX - fromX);
    if (steep) {
        FX_INT32 temp = fromX;
        fromX = fromY;
        fromY = temp;
        temp = toX;
        toX = toY;
        toY = temp;
    }
    FX_INT32 dx = FXSYS_abs(toX - fromX);
    FX_INT32 dy = FXSYS_abs(toY - fromY);
    FX_INT32 error = -dx >> 1;
    FX_INT32 ystep = fromY < toY ? 1 : -1;
    FX_INT32 xstep = fromX < toX ? 1 : -1;
    FX_INT32 state = 0;
    for (FX_INT32 x = fromX, y = fromY; x != toX; x += xstep) {
        FX_INT32 realX = steep ? y : x;
        FX_INT32 realY = steep ? x : y;
        if (state == 1) {
            if (m_image->Get(realX, realY)) {
                state++;
            }
        } else {
            if (!m_image->Get(realX, realY)) {
                state++;
            }
        }
        if (state == 3) {
            FX_INT32 diffX = x - fromX;
            FX_INT32 diffY = y - fromY;
            return (FX_FLOAT) sqrt((double) (diffX * diffX + diffY * diffY));
        }
        error += dy;
        if (error > 0) {
            y += ystep;
            error -= dx;
        }
    }
    FX_INT32 diffX = toX - fromX;
    FX_INT32 diffY = toY - fromY;
    return (FX_FLOAT) sqrt((double) (diffX * diffX + diffY * diffY));
}
CBC_QRAlignmentPattern *CBC_QRDetector::FindAlignmentInRegion(FX_FLOAT overallEstModuleSize, FX_INT32 estAlignmentX,
        FX_INT32 estAlignmentY, FX_FLOAT allowanceFactor, FX_INT32 &e)
{
    FX_INT32 allowance = (FX_INT32) (allowanceFactor * overallEstModuleSize);
    FX_INT32 alignmentAreaLeftX = FX_MAX(0, estAlignmentX - allowance);
    FX_INT32 alignmentAreaRightX = FX_MIN(m_image->GetWidth() - 1, estAlignmentX + allowance);
    if (alignmentAreaRightX - alignmentAreaLeftX < overallEstModuleSize * 3) {
        e = BCExceptionRead;
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    FX_INT32 alignmentAreaTopY = FX_MAX(0, estAlignmentY - allowance);
    FX_INT32 alignmentAreaBottomY = FX_MIN(m_image->GetHeight() - 1, estAlignmentY + allowance);
    CBC_QRAlignmentPatternFinder alignmentFinder(m_image,
            alignmentAreaLeftX,
            alignmentAreaTopY,
            alignmentAreaRightX - alignmentAreaLeftX,
            alignmentAreaBottomY - alignmentAreaTopY,
            overallEstModuleSize);
    CBC_QRAlignmentPattern *qap = alignmentFinder.Find(e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return qap;
}
