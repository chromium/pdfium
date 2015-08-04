// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_PDF417ECERRORCORRECTION_H_
#define _BC_PDF417ECERRORCORRECTION_H_
class CBC_PDF417ECModulusGF;
class CBC_PDF417ECModulusPoly;
class CBC_PDF417ECErrorCorrection {
 public:
  CBC_PDF417ECErrorCorrection();
  virtual ~CBC_PDF417ECErrorCorrection();
  static void Initialize(int32_t& e);
  static void Finalize();
  static int32_t decode(CFX_Int32Array& received,
                        int32_t numECCodewords,
                        CFX_Int32Array& erasures,
                        int32_t& e);

 private:
  static CBC_PDF417ECModulusGF* m_field;
  static CFX_PtrArray* runEuclideanAlgorithm(CBC_PDF417ECModulusPoly* a,
                                             CBC_PDF417ECModulusPoly* b,
                                             int32_t R,
                                             int32_t& e);
  static CFX_Int32Array* findErrorLocations(
      CBC_PDF417ECModulusPoly* errorLocator,
      int32_t& e);
  static CFX_Int32Array* findErrorMagnitudes(
      CBC_PDF417ECModulusPoly* errorEvaluator,
      CBC_PDF417ECModulusPoly* errorLocator,
      CFX_Int32Array& errorLocations,
      int32_t& e);
};
#endif
