// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_ReedSolomonGF256Poly.h"
#include "include/BC_ReedSolomonGF256.h"
CBC_ReedSolomonGF256 *CBC_ReedSolomonGF256::QRCodeFild = NULL;
CBC_ReedSolomonGF256 *CBC_ReedSolomonGF256::DataMatrixField = NULL;
void CBC_ReedSolomonGF256::Initialize()
{
    QRCodeFild = FX_NEW CBC_ReedSolomonGF256(0x011D);
    QRCodeFild->Init();
    DataMatrixField = FX_NEW CBC_ReedSolomonGF256(0x012D);
    DataMatrixField->Init();
}
void CBC_ReedSolomonGF256::Finalize()
{
    if (QRCodeFild) {
        delete QRCodeFild;
    }
    QRCodeFild = NULL;
    if (DataMatrixField) {
        delete DataMatrixField;
    }
    DataMatrixField = NULL;
}
CBC_ReedSolomonGF256::CBC_ReedSolomonGF256(FX_INT32 primitive)
{
    FX_INT32 x = 1;
    for(FX_INT32 j = 0; j < 256; j++) {
        m_expTable[j] = x;
        x <<= 1;
        if(x >= 0x100) {
            x ^= primitive;
        }
    }
    for(FX_INT32 i = 0; i < 255; i++) {
        m_logTable[m_expTable[i]] = i;
    }
    m_logTable[0] = 0;
}
void CBC_ReedSolomonGF256::Init()
{
    m_zero = FX_NEW CBC_ReedSolomonGF256Poly(this, 0);
    m_one = FX_NEW CBC_ReedSolomonGF256Poly(this, 1);
}
CBC_ReedSolomonGF256::~CBC_ReedSolomonGF256()
{
    if(m_zero != NULL) {
        delete m_zero;
        m_zero = NULL;
    }
    if(m_one != NULL) {
        delete m_one;
        m_one = NULL;
    }
}
CBC_ReedSolomonGF256Poly* CBC_ReedSolomonGF256::GetZero()
{
    return m_zero;
}
CBC_ReedSolomonGF256Poly* CBC_ReedSolomonGF256::GetOne()
{
    return m_one;
}
CBC_ReedSolomonGF256Poly* CBC_ReedSolomonGF256::BuildMonomial(FX_INT32 degree, FX_INT32 coefficient, FX_INT32 &e)
{
    if(degree < 0) {
        e = BCExceptionDegreeIsNegative;
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    if(coefficient == 0) {
        CBC_ReedSolomonGF256Poly* temp = m_zero->Clone(e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        return temp;
    }
    CFX_Int32Array  coefficients;
    coefficients.SetSize(degree + 1);
    coefficients[0] = coefficient;
    CBC_ReedSolomonGF256Poly *temp = FX_NEW CBC_ReedSolomonGF256Poly();
    temp->Init(this, &coefficients, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return temp;
}
FX_INT32 CBC_ReedSolomonGF256::AddOrSubtract(FX_INT32 a, FX_INT32 b)
{
    return a ^ b;
}
FX_INT32 CBC_ReedSolomonGF256::Exp(FX_INT32 a)
{
    return m_expTable[a];
}
FX_INT32 CBC_ReedSolomonGF256::Log(FX_INT32 a, FX_INT32 &e)
{
    if(a == 0) {
        e = BCExceptionAIsZero;
        BC_EXCEPTION_CHECK_ReturnValue(e, 0);
    }
    return m_logTable[a];
}
FX_INT32 CBC_ReedSolomonGF256::Inverse(FX_INT32 a, FX_INT32 &e)
{
    if(a == 0) {
        e = BCExceptionAIsZero;
        BC_EXCEPTION_CHECK_ReturnValue(e, 0);
    }
    return m_expTable[255 - m_logTable[a]];
}
FX_INT32 CBC_ReedSolomonGF256::Multiply(FX_INT32 a, FX_INT32 b)
{
    if(a == 0 || b == 0) {
        return 0;
    }
    if(a == 1) {
        return b;
    }
    if(b == 1) {
        return a;
    }
    return m_expTable[(m_logTable[a] + m_logTable[b]) % 255];
}
