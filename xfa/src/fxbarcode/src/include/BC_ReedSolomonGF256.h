// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_REEDSOLOMONGF256_H_
#define _BC_REEDSOLOMONGF256_H_
class CBC_ReedSolomonGF256Poly;
class CBC_ReedSolomonGF256;
class CBC_ReedSolomonGF256 : public CFX_Object
{
public:
    static void Initialize();
    static void Finalize();
    static CBC_ReedSolomonGF256 *QRCodeFild;
    static CBC_ReedSolomonGF256 *DataMatrixField;
    CBC_ReedSolomonGF256(FX_INT32 primitive);
    virtual ~CBC_ReedSolomonGF256();
    CBC_ReedSolomonGF256Poly* GetZero();
    CBC_ReedSolomonGF256Poly* GetOne();
    CBC_ReedSolomonGF256Poly* BuildMonomial(FX_INT32 degree, FX_INT32 coefficient, FX_INT32 &e);
    static FX_INT32 AddOrSubtract(FX_INT32 a, FX_INT32 b);
    FX_INT32 Exp(FX_INT32 a);
    FX_INT32 Log(FX_INT32 a, FX_INT32 &e);
    FX_INT32 Inverse(FX_INT32 a, FX_INT32 &e);
    FX_INT32 Multiply(FX_INT32 a, FX_INT32 b);
    virtual void Init();
private:
    FX_INT32 m_expTable[256];
    FX_INT32 m_logTable[256];
    CBC_ReedSolomonGF256Poly *m_zero;
    CBC_ReedSolomonGF256Poly *m_one;
};
#endif
