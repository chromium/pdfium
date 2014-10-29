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
class CBC_QRDetector
{
private:
    CBC_CommonBitMatrix *m_image;
public:
    CBC_QRDetector(CBC_CommonBitMatrix *image);
    virtual ~CBC_QRDetector();

    CBC_CommonBitMatrix* GetImage();
    CBC_QRDetectorResult* Detect(FX_INT32 hints, FX_INT32 &e);
    CBC_QRDetectorResult* ProcessFinderPatternInfo(CBC_QRFinderPatternInfo *info, FX_INT32 &e);
    FX_FLOAT CalculateModuleSize(CBC_ResultPoint *topLeft, CBC_ResultPoint *topRight, CBC_ResultPoint *bottomLeft);
    FX_FLOAT CalculateModuleSizeOneWay(CBC_ResultPoint *pattern, CBC_ResultPoint *otherPattern);
    FX_FLOAT SizeOfBlackWhiteBlackRunBothWays(FX_INT32 fromX, FX_INT32 fromY, FX_INT32 toX, FX_INT32 toY);
    FX_FLOAT SizeOfBlackWhiteBlackRun(FX_INT32 fromX, FX_INT32 fromY, FX_INT32 toX, FX_INT32 toY);
    CBC_QRAlignmentPattern* FindAlignmentInRegion(FX_FLOAT overallEstModuleSize, FX_INT32 estAlignmentX, FX_INT32 estAlignmentY, FX_FLOAT allowanceFactor, FX_INT32 &e);
    static FX_INT32 Round(FX_FLOAT d);
    static FX_INT32 ComputeDimension(CBC_ResultPoint *topLeft, CBC_ResultPoint *topRight, CBC_ResultPoint *bottomLeft, FX_FLOAT moduleSize, FX_INT32 &e);
    static CBC_CommonBitMatrix* SampleGrid(CBC_CommonBitMatrix *image, CBC_ResultPoint *topLeft, CBC_ResultPoint *topRight,
                                           CBC_ResultPoint *bottomLeft, CBC_ResultPoint* alignmentPattern, FX_INT32 dimension, FX_INT32 &e);
};
#endif
