// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRFINDERPATTERNFINDER_H_
#define _BC_QRFINDERPATTERNFINDER_H_
class CBC_CommonBitMatrix;
class CBC_QRFinderPattern;
class CBC_ResultPoint;
class CBC_QRFinderPatternInfo;
class CBC_QRFinderPatternFinder;
class CBC_QRFinderPatternFinder  : public CFX_Object
{
private:
    const static FX_INT32 CENTER_QUORUM;
    const static FX_INT32 MIN_SKIP;
    const static FX_INT32 MAX_MODULES;
    const static FX_INT32 INTEGER_MATH_SHIFT;
    FX_BOOL m_hasSkipped;
    CBC_CommonBitMatrix* m_image;
    CFX_Int32Array m_crossCheckStateCount;
    CFX_PtrArray m_possibleCenters;
public:
    CBC_QRFinderPatternFinder(CBC_CommonBitMatrix *image);
    virtual ~CBC_QRFinderPatternFinder();
    FX_INT32 FindRowSkip();
    CBC_CommonBitMatrix* GetImage();
    CBC_QRFinderPatternInfo* Find(FX_INT32 hint, FX_INT32 &e);

    CFX_Int32Array &GetCrossCheckStateCount();
    CFX_PtrArray *GetPossibleCenters();
    CFX_PtrArray *SelectBestpatterns(FX_INT32 &e);

    FX_BOOL HandlePossibleCenter(const CFX_Int32Array &stateCount, FX_INT32 i, FX_INT32 j);
    FX_BOOL HaveMultiplyConfirmedCenters();
    FX_FLOAT CenterFromEnd(const CFX_Int32Array &stateCount, FX_INT32 end);
    FX_FLOAT CrossCheckVertical(FX_INT32 startI, FX_INT32 centerJ, FX_INT32 maxCount, FX_INT32 originalStateCountTotal);
    FX_FLOAT CrossCheckHorizontal(FX_INT32 startJ, FX_INT32 CenterI, FX_INT32 maxCOunt, FX_INT32 originalStateCountTotal);
    static void OrderBestPatterns(CFX_PtrArray *patterns);
    static FX_BOOL FoundPatternCross(const CFX_Int32Array &stateCount);
    static FX_FLOAT Distance(CBC_ResultPoint* point1, CBC_ResultPoint* point2);
};
#endif
