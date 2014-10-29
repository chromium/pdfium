// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_READSOLOMONGF256POLY_H_
#define _BC_READSOLOMONGF256POLY_H_
class CBC_ReedSolomonGF256;
class CBC_ReedSolomonGF256Poly;
class CBC_ReedSolomonGF256Poly : public CFX_Object
{
public:
    CBC_ReedSolomonGF256Poly(CBC_ReedSolomonGF256* field, FX_INT32 coefficients);
    CBC_ReedSolomonGF256Poly();
    virtual ~CBC_ReedSolomonGF256Poly();
    FX_INT32 GetCoefficients(FX_INT32 degree);
    CFX_Int32Array* GetCoefficients();
    FX_INT32		GetDegree();
    FX_BOOL			IsZero();
    FX_INT32		EvaluateAt(FX_INT32 a);
    CBC_ReedSolomonGF256Poly* AddOrSubtract(CBC_ReedSolomonGF256Poly* other, FX_INT32 &e);
    CBC_ReedSolomonGF256Poly* Multiply(CBC_ReedSolomonGF256Poly* other, FX_INT32 &e);
    CBC_ReedSolomonGF256Poly* Multiply(FX_INT32 scalar, FX_INT32 &e);
    CBC_ReedSolomonGF256Poly* MultiplyByMonomial(FX_INT32 degree, FX_INT32 coefficient, FX_INT32 &e);
    CFX_PtrArray* Divide(CBC_ReedSolomonGF256Poly *other, FX_INT32 &e);
    CBC_ReedSolomonGF256Poly* Clone(FX_INT32 &e);
    virtual void Init(CBC_ReedSolomonGF256* field, CFX_Int32Array* coefficients, FX_INT32 &e);
private:
    CBC_ReedSolomonGF256* m_field;
    CFX_Int32Array m_coefficients;
};
#endif
