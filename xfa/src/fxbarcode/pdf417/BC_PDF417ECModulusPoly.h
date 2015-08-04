// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_PDF417ECMODULUSPOLY_H_
#define _BC_PDF417ECMODULUSPOLY_H_
class CBC_PDF417ECModulusGF;
class CBC_PDF417ECModulusPoly {
 public:
  CBC_PDF417ECModulusPoly(CBC_PDF417ECModulusGF* field,
                          CFX_Int32Array& coefficients,
                          int32_t& e);
  virtual ~CBC_PDF417ECModulusPoly();
  CFX_Int32Array& getCoefficients();
  CBC_PDF417ECModulusGF* getField();
  int32_t getDegree();
  FX_BOOL isZero();
  int32_t getCoefficient(int32_t degree);
  int32_t evaluateAt(int32_t a);
  CBC_PDF417ECModulusPoly* add(CBC_PDF417ECModulusPoly* other, int32_t& e);
  CBC_PDF417ECModulusPoly* subtract(CBC_PDF417ECModulusPoly* other, int32_t& e);
  CBC_PDF417ECModulusPoly* multiply(CBC_PDF417ECModulusPoly* other, int32_t& e);
  CBC_PDF417ECModulusPoly* negative(int32_t& e);
  CBC_PDF417ECModulusPoly* multiply(int32_t scalar, int32_t& e);
  CBC_PDF417ECModulusPoly* multiplyByMonomial(int32_t degree,
                                              int32_t coefficient,
                                              int32_t& e);
  CFX_PtrArray* divide(CBC_PDF417ECModulusPoly* other, int32_t& e);
  CFX_ByteString toString();

 private:
  CBC_PDF417ECModulusGF* m_field;
  CFX_Int32Array m_coefficients;
};
#endif
