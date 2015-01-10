// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_BOUNDINGBOX_H_
#define _BC_BOUNDINGBOX_H_
class CBC_CommonBitMatrix;
class CBC_ResultPoint;
class CBC_BoundingBox;
class CBC_BoundingBox : public CFX_Object
{
public:
    CBC_BoundingBox(CBC_CommonBitMatrix* image, CBC_ResultPoint* topLeft, CBC_ResultPoint* bottomLeft, CBC_ResultPoint* topRight, CBC_ResultPoint* bottomRight, FX_INT32 &e);
    CBC_BoundingBox(CBC_BoundingBox* boundingBox);
    virtual ~CBC_BoundingBox();
    static CBC_BoundingBox* merge(CBC_BoundingBox* leftBox, CBC_BoundingBox* rightBox, FX_INT32 &e);
    CBC_BoundingBox* addMissingRows(FX_INT32 missingStartRows, FX_INT32 missingEndRows, FX_BOOL isLeft, FX_INT32 &e);
    void setTopRight(CBC_ResultPoint topRight);
    void setBottomRight(CBC_ResultPoint bottomRight);
    FX_INT32 getMinX();
    FX_INT32 getMaxX();
    FX_INT32 getMinY();
    FX_INT32 getMaxY();
    CBC_ResultPoint* getTopLeft();
    CBC_ResultPoint* getTopRight();
    CBC_ResultPoint* getBottomLeft();
    CBC_ResultPoint* getBottomRight();
private:
    CBC_CommonBitMatrix* m_image;
    CBC_ResultPoint* m_topLeft;
    CBC_ResultPoint* m_bottomLeft;
    CBC_ResultPoint* m_topRight;
    CBC_ResultPoint* m_bottomRight;
    FX_INT32 m_minX;
    FX_INT32 m_maxX;
    FX_INT32 m_minY;
    FX_INT32 m_maxY;
    void init(CBC_CommonBitMatrix* image, CBC_ResultPoint* topLeft, CBC_ResultPoint* bottomLeft, CBC_ResultPoint* topRight, CBC_ResultPoint* bottomRight);
    void calculateMinMaxValues();
};
#endif
