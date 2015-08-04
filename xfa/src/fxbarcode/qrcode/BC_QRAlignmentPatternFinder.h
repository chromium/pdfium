// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRALIGNMENTPATTERNFINDER_H_
#define _BC_QRALIGNMENTPATTERNFINDER_H_
class CBC_CommonBitMatrix;
class CBC_QRAlignmentPattern;
class CBC_QRAlignmentPatternFinder {
 private:
  CBC_CommonBitMatrix* m_image;
  CFX_PtrArray m_possibleCenters;
  int32_t m_startX;
  int32_t m_startY;
  int32_t m_width;
  int32_t m_height;
  FX_FLOAT m_moduleSize;
  CFX_Int32Array m_crossCheckStateCount;

 public:
  CBC_QRAlignmentPatternFinder(CBC_CommonBitMatrix* image,
                               int32_t startX,
                               int32_t startY,
                               int32_t width,
                               int32_t height,
                               FX_FLOAT moduleSize);
  virtual ~CBC_QRAlignmentPatternFinder();
  FX_BOOL FoundPatternCross(const CFX_Int32Array& stateCount);
  FX_FLOAT CrossCheckVertical(int32_t startI,
                              int32_t startJ,
                              int32_t maxCount,
                              int32_t originalStateCountTotal);
  CBC_QRAlignmentPattern* Find(int32_t& e);
  CBC_QRAlignmentPattern* HandlePossibleCenter(const CFX_Int32Array& stateCount,
                                               int32_t i,
                                               int32_t j);
  static FX_FLOAT CenterFromEnd(const CFX_Int32Array& stateCount, int32_t end);
};
#endif
