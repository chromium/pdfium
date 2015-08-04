// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_READSOLOMONGF256POLY_H_
#define _BC_READSOLOMONGF256POLY_H_
class CBC_ReedSolomonGF256;
class CBC_ReedSolomonGF256Poly {
 public:
  CBC_ReedSolomonGF256Poly(CBC_ReedSolomonGF256* field, int32_t coefficients);
  CBC_ReedSolomonGF256Poly();
  virtual ~CBC_ReedSolomonGF256Poly();
  int32_t GetCoefficients(int32_t degree);
  CFX_Int32Array* GetCoefficients();
  int32_t GetDegree();
  FX_BOOL IsZero();
  int32_t EvaluateAt(int32_t a);
  CBC_ReedSolomonGF256Poly* AddOrSubtract(CBC_ReedSolomonGF256Poly* other,
                                          int32_t& e);
  CBC_ReedSolomonGF256Poly* Multiply(CBC_ReedSolomonGF256Poly* other,
                                     int32_t& e);
  CBC_ReedSolomonGF256Poly* Multiply(int32_t scalar, int32_t& e);
  CBC_ReedSolomonGF256Poly* MultiplyByMonomial(int32_t degree,
                                               int32_t coefficient,
                                               int32_t& e);
  CFX_PtrArray* Divide(CBC_ReedSolomonGF256Poly* other, int32_t& e);
  CBC_ReedSolomonGF256Poly* Clone(int32_t& e);
  virtual void Init(CBC_ReedSolomonGF256* field,
                    CFX_Int32Array* coefficients,
                    int32_t& e);

 private:
  CBC_ReedSolomonGF256* m_field;
  CFX_Int32Array m_coefficients;
};
#endif
