// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRDETECTOR_H_
#define _BC_QRDETECTOR_H_
class CBC_ResultPoint;
class CBC_CommonBitMatrix;
class CBC_QRDetectorResult;
class CBC_QRFinderPatternInfo;
class CBC_QRAlignmentPattern;
class CBC_QRFinderPatternFinder;
class CBC_QRCoderVersion;
class CBC_QRGridSampler;
class CBC_QRAlignmentPatternFinder;
class CBC_QRAlignmentPattern;
class CBC_QRDetector;
class CBC_QRDetector {
 private:
  CBC_CommonBitMatrix* m_image;

 public:
  CBC_QRDetector(CBC_CommonBitMatrix* image);
  virtual ~CBC_QRDetector();

  CBC_CommonBitMatrix* GetImage();
  CBC_QRDetectorResult* Detect(int32_t hints, int32_t& e);
  CBC_QRDetectorResult* ProcessFinderPatternInfo(CBC_QRFinderPatternInfo* info,
                                                 int32_t& e);
  FX_FLOAT CalculateModuleSize(CBC_ResultPoint* topLeft,
                               CBC_ResultPoint* topRight,
                               CBC_ResultPoint* bottomLeft);
  FX_FLOAT CalculateModuleSizeOneWay(CBC_ResultPoint* pattern,
                                     CBC_ResultPoint* otherPattern);
  FX_FLOAT SizeOfBlackWhiteBlackRunBothWays(int32_t fromX,
                                            int32_t fromY,
                                            int32_t toX,
                                            int32_t toY);
  FX_FLOAT SizeOfBlackWhiteBlackRun(int32_t fromX,
                                    int32_t fromY,
                                    int32_t toX,
                                    int32_t toY);
  CBC_QRAlignmentPattern* FindAlignmentInRegion(FX_FLOAT overallEstModuleSize,
                                                int32_t estAlignmentX,
                                                int32_t estAlignmentY,
                                                FX_FLOAT allowanceFactor,
                                                int32_t& e);
  static int32_t Round(FX_FLOAT d);
  static int32_t ComputeDimension(CBC_ResultPoint* topLeft,
                                  CBC_ResultPoint* topRight,
                                  CBC_ResultPoint* bottomLeft,
                                  FX_FLOAT moduleSize,
                                  int32_t& e);
  static CBC_CommonBitMatrix* SampleGrid(CBC_CommonBitMatrix* image,
                                         CBC_ResultPoint* topLeft,
                                         CBC_ResultPoint* topRight,
                                         CBC_ResultPoint* bottomLeft,
                                         CBC_ResultPoint* alignmentPattern,
                                         int32_t dimension,
                                         int32_t& e);
};
#endif
