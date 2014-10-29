// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_WHITERECTANLEDETECTOR_H_
#define _BC_WHITERECTANLEDETECTOR_H_
class CBC_CommonBitMatrix;
class CBC_ResultPoint;
class CBC_WhiteRectangleDetector;
class CBC_WhiteRectangleDetector : public CFX_Object
{
public:
    CBC_WhiteRectangleDetector(CBC_CommonBitMatrix *image);
    CBC_WhiteRectangleDetector(CBC_CommonBitMatrix *image, FX_INT32 initSize, FX_INT32 x, FX_INT32 y);
    virtual ~CBC_WhiteRectangleDetector();
    CFX_PtrArray *Detect(FX_INT32 &e);
    virtual void Init(FX_INT32 &e);
private:
    FX_INT32 Round(float d);
    CBC_ResultPoint *GetBlackPointOnSegment(FX_FLOAT aX, FX_FLOAT aY, FX_FLOAT bX, FX_FLOAT bY);
    FX_INT32 DistanceL2(FX_FLOAT aX, FX_FLOAT aY, FX_FLOAT bX, FX_FLOAT bY);
    CFX_PtrArray *CenterEdges(CBC_ResultPoint *y, CBC_ResultPoint *z,
                              CBC_ResultPoint *x, CBC_ResultPoint *t);
    FX_BOOL ContainsBlackPoint(FX_INT32 a, FX_INT32 b, FX_INT32 fixed, FX_BOOL horizontal);
    const static FX_INT32 INIT_SIZE;
    const static FX_INT32 CORR;

    CBC_CommonBitMatrix *m_image;
    FX_INT32 m_height;
    FX_INT32 m_width;
    FX_INT32 m_leftInit;
    FX_INT32 m_rightInit;
    FX_INT32 m_downInit;
    FX_INT32 m_upInit;
};
#endif
