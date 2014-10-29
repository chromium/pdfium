// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRALIGNMENTPATTERNFINDER_H_
#define _BC_QRALIGNMENTPATTERNFINDER_H_
class CBC_CommonBitMatrix;
class CBC_QRAlignmentPattern;
class CBC_QRAlignmentPatternFinder;
class CBC_QRAlignmentPatternFinder  : public CFX_Object
{
private:
    CBC_CommonBitMatrix *m_image;
    CFX_PtrArray m_possibleCenters;
    FX_INT32 m_startX;
    FX_INT32 m_startY;
    FX_INT32 m_width;
    FX_INT32 m_height;
    FX_FLOAT m_moduleSize;
    CFX_Int32Array m_crossCheckStateCount;
public:
    CBC_QRAlignmentPatternFinder(CBC_CommonBitMatrix *image, FX_INT32 startX, FX_INT32 startY, FX_INT32 width, FX_INT32 height, FX_FLOAT moduleSize);
    virtual ~CBC_QRAlignmentPatternFinder();
    FX_BOOL FoundPatternCross(const CFX_Int32Array &stateCount);
    FX_FLOAT CrossCheckVertical(FX_INT32 startI, FX_INT32 startJ, FX_INT32 maxCount, FX_INT32 originalStateCountTotal);
    CBC_QRAlignmentPattern* Find(FX_INT32 &e);
    CBC_QRAlignmentPattern *HandlePossibleCenter(const CFX_Int32Array &stateCount, FX_INT32 i, FX_INT32 j);
    static FX_FLOAT CenterFromEnd(const CFX_Int32Array &stateCount, FX_INT32 end);
};
#endif
