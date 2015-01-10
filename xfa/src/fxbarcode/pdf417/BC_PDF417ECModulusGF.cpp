// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2012 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../barcode.h"
#include "BC_PDF417Common.h"
#include "BC_PDF417ECModulusPoly.h"
#include "BC_PDF417ECModulusGF.h"
CBC_PDF417ECModulusGF* CBC_PDF417ECModulusGF::PDF417_GF = NULL;
void CBC_PDF417ECModulusGF::Initialize(FX_INT32 &e)
{
    PDF417_GF = FX_NEW CBC_PDF417ECModulusGF(CBC_PDF417Common::NUMBER_OF_CODEWORDS, 3, e);
}
void CBC_PDF417ECModulusGF::Finalize()
{
    delete PDF417_GF;
}
CBC_PDF417ECModulusGF::CBC_PDF417ECModulusGF(FX_INT32 modulus, FX_INT32 generator, FX_INT32 &e)
{
    m_modulus = modulus;
    m_expTable.SetSize(modulus);
    m_logTable.SetSize(modulus);
    FX_INT32 x = 1;
    for (FX_INT32 i = 0; i < modulus; i++) {
        m_expTable[i] = x;
        x = (x * generator) % modulus;
    }
    for (FX_INT32 j = 0; j < modulus - 1; j++) {
        m_logTable[m_expTable[j]] = j;
    }
    CFX_Int32Array zero;
    zero.Add(0);
    m_zero = FX_NEW CBC_PDF417ECModulusPoly(this, zero, e);
    CFX_Int32Array one;
    one.Add(1);
    m_one = FX_NEW CBC_PDF417ECModulusPoly(this, one, e);
}
CBC_PDF417ECModulusGF::~CBC_PDF417ECModulusGF()
{
    delete m_zero;
    delete m_one;
}
CBC_PDF417ECModulusPoly* CBC_PDF417ECModulusGF::getZero()
{
    return m_zero;
}
CBC_PDF417ECModulusPoly* CBC_PDF417ECModulusGF::getOne()
{
    return m_one;
}
CBC_PDF417ECModulusPoly* CBC_PDF417ECModulusGF::buildMonomial(FX_INT32 degree, FX_INT32 coefficient, FX_INT32 &e)
{
    if (degree < 0) {
        e = BCExceptionIllegalArgument;
        return NULL;
    }
    CBC_PDF417ECModulusPoly* modulusPoly = NULL;
    if (coefficient == 0) {
        modulusPoly = FX_NEW CBC_PDF417ECModulusPoly(m_zero->getField(), m_zero->getCoefficients(), e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        return modulusPoly;
    }
    CFX_Int32Array coefficients;
    coefficients.SetSize(degree + 1);
    coefficients[0] = coefficient;
    modulusPoly = FX_NEW CBC_PDF417ECModulusPoly(this, coefficients, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return modulusPoly;
}
FX_INT32 CBC_PDF417ECModulusGF::add(FX_INT32 a, FX_INT32 b)
{
    return (a + b) % m_modulus;
}
FX_INT32 CBC_PDF417ECModulusGF::subtract(FX_INT32 a, FX_INT32 b)
{
    return (m_modulus + a - b) % m_modulus;
}
FX_INT32 CBC_PDF417ECModulusGF::exp(FX_INT32 a)
{
    return m_expTable[a];
}
FX_INT32 CBC_PDF417ECModulusGF::log(FX_INT32 a, FX_INT32 &e)
{
    if (a == 0) {
        e = BCExceptionIllegalArgument;
        return -1;
    }
    return m_logTable[a];
}
FX_INT32 CBC_PDF417ECModulusGF::inverse(FX_INT32 a, FX_INT32 &e)
{
    if (a == 0) {
        e = BCExceptionIllegalArgument;
        return -1;
    }
    return m_expTable[m_modulus - m_logTable[a] - 1];
}
FX_INT32 CBC_PDF417ECModulusGF::multiply(FX_INT32 a, FX_INT32 b)
{
    if (a == 0 || b == 0) {
        return 0;
    }
    return m_expTable[(m_logTable[a] + m_logTable[b]) % (m_modulus - 1)];
}
FX_INT32 CBC_PDF417ECModulusGF::getSize()
{
    return m_modulus;
}
