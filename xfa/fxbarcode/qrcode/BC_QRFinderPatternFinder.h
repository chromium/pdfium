// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_QRCODE_BC_QRFINDERPATTERNFINDER_H_
#define XFA_FXBARCODE_QRCODE_BC_QRFINDERPATTERNFINDER_H_

#include "core/fxcrt/include/fx_basic.h"

class CBC_CommonBitMatrix;
class CBC_QRFinderPattern;
class CBC_ResultPoint;
class CBC_QRFinderPatternInfo;

class CBC_QRFinderPatternFinder {
 public:
  CBC_QRFinderPatternFinder(CBC_CommonBitMatrix* image);
  virtual ~CBC_QRFinderPatternFinder();
  int32_t FindRowSkip();
  CBC_CommonBitMatrix* GetImage();
  CBC_QRFinderPatternInfo* Find(int32_t hint, int32_t& e);

  CFX_Int32Array& GetCrossCheckStateCount();
  CFX_ArrayTemplate<CBC_QRFinderPattern*>* GetPossibleCenters();
  CFX_ArrayTemplate<CBC_QRFinderPattern*>* SelectBestpatterns(int32_t& e);

  FX_BOOL HandlePossibleCenter(const CFX_Int32Array& stateCount,
                               int32_t i,
                               int32_t j);
  FX_BOOL HaveMultiplyConfirmedCenters();
  FX_FLOAT CenterFromEnd(const CFX_Int32Array& stateCount, int32_t end);
  FX_FLOAT CrossCheckVertical(int32_t startI,
                              int32_t centerJ,
                              int32_t maxCount,
                              int32_t originalStateCountTotal);
  FX_FLOAT CrossCheckHorizontal(int32_t startJ,
                                int32_t CenterI,
                                int32_t maxCOunt,
                                int32_t originalStateCountTotal);
  static void OrderBestPatterns(
      CFX_ArrayTemplate<CBC_QRFinderPattern*>* patterns);
  static FX_BOOL FoundPatternCross(const CFX_Int32Array& stateCount);
  static FX_FLOAT Distance(CBC_ResultPoint* point1, CBC_ResultPoint* point2);

 private:
  static const int32_t CENTER_QUORUM;
  static const int32_t MIN_SKIP;
  static const int32_t MAX_MODULES;
  static const int32_t INTEGER_MATH_SHIFT;

  FX_BOOL m_hasSkipped;
  CBC_CommonBitMatrix* m_image;
  CFX_Int32Array m_crossCheckStateCount;
  CFX_ArrayTemplate<CBC_QRFinderPattern*> m_possibleCenters;
};

#endif  // XFA_FXBARCODE_QRCODE_BC_QRFINDERPATTERNFINDER_H_
