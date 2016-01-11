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

#include <algorithm>

#include "xfa/src/fxbarcode/barcode.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitMatrix.h"
#include "xfa/src/fxbarcode/BC_ResultPoint.h"
#include "BC_QRFinderPattern.h"
#include "BC_QRCoderVersion.h"
#include "BC_FinderPatternInfo.h"
#include "BC_QRGridSampler.h"
#include "BC_QRAlignmentPatternFinder.h"
#include "BC_QRFinderPatternFinder.h"
#include "BC_QRDetectorResult.h"
#include "BC_QRDetector.h"
CBC_QRDetector::CBC_QRDetector(CBC_CommonBitMatrix* image) : m_image(image) {}
CBC_QRDetector::~CBC_QRDetector() {}
CBC_QRDetectorResult* CBC_QRDetector::Detect(int32_t hints, int32_t& e) {
  CBC_QRFinderPatternFinder finder(m_image);
  CBC_QRFinderPatternInfo* qpi = finder.Find(hints, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  CBC_AutoPtr<CBC_QRFinderPatternInfo> info(qpi);
  CBC_QRDetectorResult* qdr = ProcessFinderPatternInfo(info.get(), e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return qdr;
}
CBC_QRDetectorResult* CBC_QRDetector::ProcessFinderPatternInfo(
    CBC_QRFinderPatternInfo* info,
    int32_t& e) {
  CBC_AutoPtr<CBC_QRFinderPattern> topLeft(info->GetTopLeft());
  CBC_AutoPtr<CBC_QRFinderPattern> topRight(info->GetTopRight());
  CBC_AutoPtr<CBC_QRFinderPattern> bottomLeft(info->GetBottomLeft());
  FX_FLOAT moduleSize =
      CalculateModuleSize(topLeft.get(), topRight.get(), bottomLeft.get());
  if (moduleSize < 1.0f) {
    e = BCExceptionRead;
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  }
  int32_t dimension = ComputeDimension(topLeft.get(), topRight.get(),
                                       bottomLeft.get(), moduleSize, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  CBC_QRCoderVersion* provisionalVersion =
      CBC_QRCoderVersion::GetProvisionalVersionForDimension(dimension, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  int32_t modulesBetweenFPCenters =
      provisionalVersion->GetDimensionForVersion() - 7;
  CBC_QRAlignmentPattern* alignmentPattern = NULL;
  if (provisionalVersion->GetAlignmentPatternCenters()->GetSize() > 0) {
    FX_FLOAT bottomRightX =
        topRight->GetX() - topLeft->GetX() + bottomLeft->GetX();
    FX_FLOAT bottomRightY =
        topRight->GetY() - topLeft->GetY() + bottomLeft->GetY();
    FX_FLOAT correctionToTopLeft =
        1.0f - 3.0f / (FX_FLOAT)modulesBetweenFPCenters;
    FX_FLOAT xtemp = (topLeft->GetX() +
                      correctionToTopLeft * (bottomRightX - topLeft->GetX()));
    int32_t estAlignmentX = (int32_t)xtemp;
    FX_FLOAT ytemp = (topLeft->GetY() +
                      correctionToTopLeft * (bottomRightY - topLeft->GetY()));
    int32_t estAlignmentY = (int32_t)ytemp;
    for (int32_t i = 4; i <= 16; i <<= 1) {
      CBC_QRAlignmentPattern* temp = FindAlignmentInRegion(
          moduleSize, estAlignmentX, estAlignmentY, (FX_FLOAT)i, e);
      alignmentPattern = temp;
      break;
    }
  }
  CBC_CommonBitMatrix* bits =
      SampleGrid(m_image, topLeft.get(), topRight.get(), bottomLeft.get(),
                 (CBC_ResultPoint*)(alignmentPattern), dimension, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  CFX_PtrArray* points = new CFX_PtrArray;
  if (alignmentPattern == NULL) {
    points->Add(bottomLeft.release());
    points->Add(topLeft.release());
    points->Add(topRight.release());
  } else {
    points->Add(bottomLeft.release());
    points->Add(topLeft.release());
    points->Add(topRight.release());
    points->Add(alignmentPattern);
  }
  return new CBC_QRDetectorResult(bits, points);
}
CBC_CommonBitMatrix* CBC_QRDetector::SampleGrid(
    CBC_CommonBitMatrix* image,
    CBC_ResultPoint* topLeft,
    CBC_ResultPoint* topRight,
    CBC_ResultPoint* bottomLeft,
    CBC_ResultPoint* alignmentPattern,
    int32_t dimension,
    int32_t& e) {
  FX_FLOAT dimMinusThree = (FX_FLOAT)dimension - 3.5f;
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
  CBC_QRGridSampler& sampler = CBC_QRGridSampler::GetInstance();
  CBC_CommonBitMatrix* cbm = sampler.SampleGrid(
      image, dimension, dimension, 3.5f, 3.5f, dimMinusThree, 3.5f,
      sourceBottomRightX, sourceBottomRightY, 3.5f, dimMinusThree,
      topLeft->GetX(), topLeft->GetY(), topRight->GetX(), topRight->GetY(),
      bottomRightX, bottomRightY, bottomLeft->GetX(), bottomLeft->GetY(), e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return cbm;
}
int32_t CBC_QRDetector::ComputeDimension(CBC_ResultPoint* topLeft,
                                         CBC_ResultPoint* topRight,
                                         CBC_ResultPoint* bottomLeft,
                                         FX_FLOAT moduleSize,
                                         int32_t& e) {
  int32_t tltrCentersDimension = Round(
      CBC_QRFinderPatternFinder::Distance(topLeft, topRight) / moduleSize);
  int32_t tlblCentersDimension = Round(
      CBC_QRFinderPatternFinder::Distance(topLeft, bottomLeft) / moduleSize);
  int32_t dimension = ((tltrCentersDimension + tlblCentersDimension) >> 1) + 7;
  switch (dimension & 0x03) {
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
FX_FLOAT CBC_QRDetector::CalculateModuleSize(CBC_ResultPoint* topLeft,
                                             CBC_ResultPoint* topRight,
                                             CBC_ResultPoint* bottomLeft) {
  return (CalculateModuleSizeOneWay(topLeft, topRight) +
          CalculateModuleSizeOneWay(topLeft, bottomLeft)) /
         2.0f;
}
FX_FLOAT CBC_QRDetector::CalculateModuleSizeOneWay(
    CBC_ResultPoint* pattern,
    CBC_ResultPoint* otherPattern) {
  FX_FLOAT moduleSizeEst1 = SizeOfBlackWhiteBlackRunBothWays(
      (int32_t)pattern->GetX(), (int32_t)pattern->GetY(),
      (int32_t)otherPattern->GetX(), (int32_t)otherPattern->GetY());
  FX_FLOAT moduleSizeEst2 = SizeOfBlackWhiteBlackRunBothWays(
      (int32_t)otherPattern->GetX(), (int32_t)otherPattern->GetY(),
      (int32_t)pattern->GetX(), (int32_t)pattern->GetY());
  if (FXSYS_isnan(moduleSizeEst1)) {
    return moduleSizeEst2;
  }
  if (FXSYS_isnan(moduleSizeEst2)) {
    return moduleSizeEst1;
  }
  return (moduleSizeEst1 + moduleSizeEst2) / 14.0f;
}
int32_t CBC_QRDetector::Round(FX_FLOAT d) {
  return (int32_t)(d + 0.5f);
}
FX_FLOAT CBC_QRDetector::SizeOfBlackWhiteBlackRunBothWays(int32_t fromX,
                                                          int32_t fromY,
                                                          int32_t toX,
                                                          int32_t toY) {
  FX_FLOAT result = SizeOfBlackWhiteBlackRun(fromX, fromY, toX, toY);
  int32_t otherToX = fromX - (toX - fromX);
  if (otherToX < 0) {
    otherToX = -1;
  } else if (otherToX >= m_image->GetWidth()) {
    otherToX = m_image->GetWidth();
  }
  int32_t otherToY = fromY - (toY - fromY);
  if (otherToY < 0) {
    otherToY = -1;
  } else if (otherToY >= m_image->GetHeight()) {
    otherToY = m_image->GetHeight();
  }
  result += SizeOfBlackWhiteBlackRun(fromX, fromY, otherToX, otherToY);
  return result - 1.0f;
}
FX_FLOAT CBC_QRDetector::SizeOfBlackWhiteBlackRun(int32_t fromX,
                                                  int32_t fromY,
                                                  int32_t toX,
                                                  int32_t toY) {
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
  int32_t state = 0;
  for (int32_t x = fromX, y = fromY; x != toX; x += xstep) {
    int32_t realX = steep ? y : x;
    int32_t realY = steep ? x : y;
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
      int32_t diffX = x - fromX;
      int32_t diffY = y - fromY;
      return (FX_FLOAT)sqrt((double)(diffX * diffX + diffY * diffY));
    }
    error += dy;
    if (error > 0) {
      y += ystep;
      error -= dx;
    }
  }
  int32_t diffX = toX - fromX;
  int32_t diffY = toY - fromY;
  return (FX_FLOAT)sqrt((double)(diffX * diffX + diffY * diffY));
}
CBC_QRAlignmentPattern* CBC_QRDetector::FindAlignmentInRegion(
    FX_FLOAT overallEstModuleSize,
    int32_t estAlignmentX,
    int32_t estAlignmentY,
    FX_FLOAT allowanceFactor,
    int32_t& e) {
  int32_t allowance = (int32_t)(allowanceFactor * overallEstModuleSize);
  int32_t alignmentAreaLeftX = std::max(0, estAlignmentX - allowance);
  int32_t alignmentAreaRightX =
      std::min(m_image->GetWidth() - 1, estAlignmentX + allowance);
  if (alignmentAreaRightX - alignmentAreaLeftX < overallEstModuleSize * 3) {
    e = BCExceptionRead;
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  }
  int32_t alignmentAreaTopY = std::max(0, estAlignmentY - allowance);
  int32_t alignmentAreaBottomY =
      std::min(m_image->GetHeight() - 1, estAlignmentY + allowance);
  CBC_QRAlignmentPatternFinder alignmentFinder(
      m_image, alignmentAreaLeftX, alignmentAreaTopY,
      alignmentAreaRightX - alignmentAreaLeftX,
      alignmentAreaBottomY - alignmentAreaTopY, overallEstModuleSize);
  CBC_QRAlignmentPattern* qap = alignmentFinder.Find(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return qap;
}
