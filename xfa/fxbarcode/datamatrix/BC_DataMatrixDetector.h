// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_DATAMATRIX_BC_DATAMATRIXDETECTOR_H_
#define XFA_FXBARCODE_DATAMATRIX_BC_DATAMATRIXDETECTOR_H_

#include "core/fxcrt/include/fx_basic.h"

class CBC_CommonBitMatrix;
class CBC_DataMatrixDetector;
class CBC_QRDetectorResult;
class CBC_ResultPoint;
class CBC_WhiteRectangleDetector;

class CBC_ResultPointsAndTransitions {
 public:
  CBC_ResultPointsAndTransitions(CBC_ResultPoint* from,
                                 CBC_ResultPoint* to,
                                 int32_t transitions) {
    m_from = from;
    m_to = to;
    m_transitions = transitions;
  }
  ~CBC_ResultPointsAndTransitions() {}
  CBC_ResultPoint* GetFrom() const { return m_from; }
  CBC_ResultPoint* GetTo() const { return m_to; }
  int32_t GetTransitions() const { return m_transitions; }

 private:
  CBC_ResultPoint* m_from;
  CBC_ResultPoint* m_to;
  int32_t m_transitions;
};

class CBC_DataMatrixDetector {
 public:
  CBC_DataMatrixDetector(CBC_CommonBitMatrix* image);
  virtual ~CBC_DataMatrixDetector();
  CBC_QRDetectorResult* Detect(int32_t& e);
  CBC_ResultPoint* CorrectTopRightRectangular(CBC_ResultPoint* bottomLeft,
                                              CBC_ResultPoint* bottomRight,
                                              CBC_ResultPoint* topLeft,
                                              CBC_ResultPoint* topRight,
                                              int32_t dimensionTop,
                                              int32_t dimensionRight);
  CBC_ResultPoint* CorrectTopRight(CBC_ResultPoint* bottomLeft,
                                   CBC_ResultPoint* bottomRight,
                                   CBC_ResultPoint* topLeft,
                                   CBC_ResultPoint* topRight,
                                   int32_t dimension);
  CBC_CommonBitMatrix* SampleGrid(CBC_CommonBitMatrix* image,
                                  CBC_ResultPoint* topLeft,
                                  CBC_ResultPoint* bottomLeft,
                                  CBC_ResultPoint* bottomRight,
                                  CBC_ResultPoint* topRight,
                                  int32_t dimensionX,
                                  int32_t dimensionY,
                                  int32_t& e);
  CBC_ResultPointsAndTransitions* TransitionsBetween(CBC_ResultPoint* from,
                                                     CBC_ResultPoint* to);
  FX_BOOL IsValid(CBC_ResultPoint* p);
  int32_t Distance(CBC_ResultPoint* a, CBC_ResultPoint* b);
  void Increment(CFX_MapPtrTemplate<CBC_ResultPoint*, int32_t>& table,
                 CBC_ResultPoint* key);
  int32_t Round(FX_FLOAT d);
  void OrderBestPatterns(CFX_ArrayTemplate<CBC_ResultPoint*>* patterns);
  virtual void Init(int32_t& e);

 private:
  static const int32_t INTEGERS[5];

  CBC_CommonBitMatrix* m_image;
  CBC_WhiteRectangleDetector* m_rectangleDetector;
};

#endif  // XFA_FXBARCODE_DATAMATRIX_BC_DATAMATRIXDETECTOR_H_
