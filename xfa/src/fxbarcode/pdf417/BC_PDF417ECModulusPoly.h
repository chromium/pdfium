// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_PDF417ECMODULUSPOLY_H_
#define _BC_PDF417ECMODULUSPOLY_H_
class CBC_PDF417ECModulusGF;
class CBC_PDF417ECModulusPoly;
class CBC_PDF417ECModulusPoly : public CFX_Object
{
public:
    CBC_PDF417ECModulusPoly(CBC_PDF417ECModulusGF* field, CFX_Int32Array &coefficients, FX_INT32 &e);
    virtual ~CBC_PDF417ECModulusPoly();
    CFX_Int32Array& getCoefficients();
    CBC_PDF417ECModulusGF* getField();
    FX_INT32 getDegree();
    FX_BOOL isZero();
    FX_INT32 getCoefficient(FX_INT32 degree);
    FX_INT32 evaluateAt(FX_INT32 a);
    CBC_PDF417ECModulusPoly* add(CBC_PDF417ECModulusPoly* other, FX_INT32 &e);
    CBC_PDF417ECModulusPoly* subtract(CBC_PDF417ECModulusPoly* other, FX_INT32 &e);
    CBC_PDF417ECModulusPoly* multiply(CBC_PDF417ECModulusPoly* other, FX_INT32 &e);
    CBC_PDF417ECModulusPoly* negative(FX_INT32 &e);
    CBC_PDF417ECModulusPoly* multiply(FX_INT32 scalar, FX_INT32 &e);
    CBC_PDF417ECModulusPoly* multiplyByMonomial(FX_INT32 degree, FX_INT32 coefficient, FX_INT32 &e);
    CFX_PtrArray* divide(CBC_PDF417ECModulusPoly* other, FX_INT32 &e);
    CFX_ByteString toString();
private:
    CBC_PDF417ECModulusGF* m_field;
    CFX_Int32Array m_coefficients;
};
#endif
