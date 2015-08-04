// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_BOUNDINGBOX_H_
#define _BC_BOUNDINGBOX_H_
class CBC_CommonBitMatrix;
class CBC_ResultPoint;
class CBC_BoundingBox {
 public:
  CBC_BoundingBox(CBC_CommonBitMatrix* image,
                  CBC_ResultPoint* topLeft,
                  CBC_ResultPoint* bottomLeft,
                  CBC_ResultPoint* topRight,
                  CBC_ResultPoint* bottomRight,
                  int32_t& e);
  CBC_BoundingBox(CBC_BoundingBox* boundingBox);
  virtual ~CBC_BoundingBox();
  static CBC_BoundingBox* merge(CBC_BoundingBox* leftBox,
                                CBC_BoundingBox* rightBox,
                                int32_t& e);
  CBC_BoundingBox* addMissingRows(int32_t missingStartRows,
                                  int32_t missingEndRows,
                                  FX_BOOL isLeft,
                                  int32_t& e);
  void setTopRight(CBC_ResultPoint topRight);
  void setBottomRight(CBC_ResultPoint bottomRight);
  int32_t getMinX();
  int32_t getMaxX();
  int32_t getMinY();
  int32_t getMaxY();
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
  int32_t m_minX;
  int32_t m_maxX;
  int32_t m_minY;
  int32_t m_maxY;
  void init(CBC_CommonBitMatrix* image,
            CBC_ResultPoint* topLeft,
            CBC_ResultPoint* bottomLeft,
            CBC_ResultPoint* topRight,
            CBC_ResultPoint* bottomRight);
  void calculateMinMaxValues();
};
#endif
