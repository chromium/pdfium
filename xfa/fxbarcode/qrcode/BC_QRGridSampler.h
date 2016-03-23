// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_QRCODE_BC_QRGRIDSAMPLER_H_
#define XFA_FXBARCODE_QRCODE_BC_QRGRIDSAMPLER_H_

#include "core/fxcrt/include/fx_basic.h"

class CBC_CommonBitMatrix;

class CBC_QRGridSampler {
 private:
  static CBC_QRGridSampler m_gridSampler;

 public:
  CBC_QRGridSampler();
  virtual ~CBC_QRGridSampler();
  virtual CBC_CommonBitMatrix* SampleGrid(CBC_CommonBitMatrix* image,
                                          int32_t dimensionX,
                                          int32_t dimensionY,
                                          FX_FLOAT p1ToX,
                                          FX_FLOAT p1ToY,
                                          FX_FLOAT p2ToX,
                                          FX_FLOAT p2ToY,
                                          FX_FLOAT p3ToX,
                                          FX_FLOAT p3ToY,
                                          FX_FLOAT p4ToX,
                                          FX_FLOAT p4ToY,
                                          FX_FLOAT p1FromX,
                                          FX_FLOAT p1FromY,
                                          FX_FLOAT p2FromX,
                                          FX_FLOAT p2FromY,
                                          FX_FLOAT p3FromX,
                                          FX_FLOAT p3FromY,
                                          FX_FLOAT p4FromX,
                                          FX_FLOAT p4FromY,
                                          int32_t& e);

  static CBC_QRGridSampler& GetInstance();
  static void CheckAndNudgePoints(CBC_CommonBitMatrix* image,
                                  CFX_FloatArray* points,
                                  int32_t& e);
};

#endif  // XFA_FXBARCODE_QRCODE_BC_QRGRIDSAMPLER_H_
