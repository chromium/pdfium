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

#include "barcode.h"
#include "include/BC_PDF417Common.h"
#include "include/BC_PDF417ECModulusGF.h"
#include "include/BC_PDF417ECModulusPoly.h"
CBC_PDF417ECModulusPoly::CBC_PDF417ECModulusPoly(CBC_PDF417ECModulusGF* field, CFX_Int32Array &coefficients, FX_INT32 &e)
{
    if (coefficients.GetSize() == 0) {
        e = BCExceptionIllegalArgument;
    }
    m_field = field;
    FX_INT32 coefficientsLength = coefficients.GetSize();
    if (coefficientsLength > 1 && coefficients[0] == 0) {
        FX_INT32 firstNonZero = 1;
        while (firstNonZero < coefficientsLength && coefficients[firstNonZero] == 0) {
            firstNonZero++;
        }
        if (firstNonZero == coefficientsLength) {
            m_coefficients = field->getZero()->m_coefficients;
        } else {
            m_coefficients.SetSize(coefficientsLength - firstNonZero);
            FX_INT32 l = 0;
            for (FX_INT32 i = firstNonZero; i < firstNonZero + m_coefficients.GetSize(); i++) {
                m_coefficients.SetAt(l, coefficients.GetAt(i));
                l++;
            }
        }
    } else {
        m_coefficients.Copy(coefficients);
    }
}
CBC_PDF417ECModulusPoly::~CBC_PDF417ECModulusPoly()
{
}
CFX_Int32Array& CBC_PDF417ECModulusPoly::getCoefficients()
{
    return m_coefficients;
}
CBC_PDF417ECModulusGF* CBC_PDF417ECModulusPoly::getField()
{
    return m_field;
}
FX_INT32 CBC_PDF417ECModulusPoly::getDegree()
{
    return m_coefficients.GetSize() - 1;
}
FX_BOOL CBC_PDF417ECModulusPoly::isZero()
{
    return m_coefficients[0] == 0;
}
FX_INT32 CBC_PDF417ECModulusPoly::getCoefficient(FX_INT32 degree)
{
    return m_coefficients[m_coefficients.GetSize() - 1 - degree];
}
FX_INT32 CBC_PDF417ECModulusPoly::evaluateAt(FX_INT32 a)
{
    if (a == 0) {
        return getCoefficient(0);
    }
    FX_INT32 size = m_coefficients.GetSize();
    if (a == 1) {
        FX_INT32 result = 0;
        for (FX_INT32 l = 0; l < m_coefficients.GetSize(); l++) {
            FX_INT32 coefficient = m_coefficients.GetAt(l);
            result = m_field->add(result, coefficient);
        }
        return result;
    }
    FX_INT32 result = m_coefficients[0];
    for (FX_INT32 i = 1; i < size; i++) {
        result = m_field->add(m_field->multiply(a, result), m_coefficients[i]);
    }
    return result;
}
CBC_PDF417ECModulusPoly* CBC_PDF417ECModulusPoly::add(CBC_PDF417ECModulusPoly* other, FX_INT32 &e)
{
    CBC_PDF417ECModulusPoly* modulusPoly = NULL;
    if (isZero()) {
        modulusPoly = FX_NEW CBC_PDF417ECModulusPoly(other->getField(), other->getCoefficients(), e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        return modulusPoly;
    }
    if (other->isZero()) {
        modulusPoly = FX_NEW CBC_PDF417ECModulusPoly(m_field, m_coefficients, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        return modulusPoly;
    }
    CFX_Int32Array smallerCoefficients;
    smallerCoefficients.Copy(m_coefficients);
    CFX_Int32Array largerCoefficients;
    largerCoefficients.Copy(other->m_coefficients);
    if (smallerCoefficients.GetSize() > largerCoefficients.GetSize()) {
        CFX_Int32Array temp;
        temp.Copy(smallerCoefficients);
        smallerCoefficients.Copy(largerCoefficients);
        largerCoefficients.Copy(temp);
    }
    CFX_Int32Array sumDiff;
    sumDiff.SetSize(largerCoefficients.GetSize());
    FX_INT32 lengthDiff = largerCoefficients.GetSize() - smallerCoefficients.GetSize();
    for (FX_INT32 l = 0; l < lengthDiff; l++) {
        sumDiff.SetAt(l, largerCoefficients.GetAt(l));
    }
    for (FX_INT32 i = lengthDiff; i < largerCoefficients.GetSize(); i++) {
        sumDiff[i] = m_field->add(smallerCoefficients[i - lengthDiff], largerCoefficients[i]);
    }
    modulusPoly = FX_NEW CBC_PDF417ECModulusPoly(m_field, sumDiff, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return modulusPoly;
}
CBC_PDF417ECModulusPoly* CBC_PDF417ECModulusPoly::subtract(CBC_PDF417ECModulusPoly* other, FX_INT32 &e)
{
    CBC_PDF417ECModulusPoly* modulusPoly = NULL;
    if (other->isZero()) {
        modulusPoly = FX_NEW CBC_PDF417ECModulusPoly(m_field, m_coefficients, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        return modulusPoly;
    }
    CBC_PDF417ECModulusPoly* poly  = other->negative(e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    modulusPoly = add(poly, e);
    delete poly;
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return modulusPoly;
}
CBC_PDF417ECModulusPoly* CBC_PDF417ECModulusPoly::multiply(CBC_PDF417ECModulusPoly* other, FX_INT32 &e)
{
    CBC_PDF417ECModulusPoly* modulusPoly = NULL;
    if (isZero() || other->isZero()) {
        modulusPoly = FX_NEW CBC_PDF417ECModulusPoly(m_field->getZero()->getField(), m_field->getZero()->getCoefficients(), e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        return modulusPoly;
    }
    CFX_Int32Array aCoefficients;
    aCoefficients.Copy(m_coefficients);
    FX_INT32 aLength = aCoefficients.GetSize();
    CFX_Int32Array bCoefficients;
    bCoefficients.Copy(other->m_coefficients);
    FX_INT32 bLength = bCoefficients.GetSize();
    CFX_Int32Array product;
    product.SetSize(aLength + bLength - 1);
    for (FX_INT32 i = 0; i < aLength; i++) {
        FX_INT32 aCoeff = aCoefficients[i];
        for (FX_INT32 j = 0; j < bLength; j++) {
            product[i + j] = m_field->add(product[i + j], m_field->multiply(aCoeff, bCoefficients[j]));
        }
    }
    modulusPoly = FX_NEW CBC_PDF417ECModulusPoly(m_field, product, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return modulusPoly;
}
CBC_PDF417ECModulusPoly* CBC_PDF417ECModulusPoly::negative(FX_INT32 &e)
{
    FX_INT32 size = m_coefficients.GetSize();
    CFX_Int32Array negativeCoefficients;
    negativeCoefficients.SetSize(size);
    for (FX_INT32 i = 0; i < size; i++) {
        negativeCoefficients[i] = m_field->subtract(0, m_coefficients[i]);
    }
    CBC_PDF417ECModulusPoly* modulusPoly = FX_NEW CBC_PDF417ECModulusPoly(m_field, negativeCoefficients, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return modulusPoly;
}
CBC_PDF417ECModulusPoly* CBC_PDF417ECModulusPoly::multiply(FX_INT32 scalar, FX_INT32 &e)
{
    CBC_PDF417ECModulusPoly* modulusPoly = NULL;
    if (scalar == 0) {
        modulusPoly = FX_NEW CBC_PDF417ECModulusPoly(m_field->getZero()->getField(), m_field->getZero()->getCoefficients(), e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        return modulusPoly;
    }
    if (scalar == 1) {
        modulusPoly = FX_NEW CBC_PDF417ECModulusPoly(m_field, m_coefficients, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        return modulusPoly;
    }
    FX_INT32 size = m_coefficients.GetSize();
    CFX_Int32Array product;
    product.SetSize(size);
    for (FX_INT32 i = 0; i < size; i++) {
        product[i] = m_field->multiply(m_coefficients[i], scalar);
    }
    modulusPoly = FX_NEW CBC_PDF417ECModulusPoly(m_field, product, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return modulusPoly;
}
CBC_PDF417ECModulusPoly* CBC_PDF417ECModulusPoly::multiplyByMonomial(FX_INT32 degree, FX_INT32 coefficient, FX_INT32 &e)
{
    if (degree < 0) {
        e = BCExceptionIllegalArgument;
        return NULL;
    }
    CBC_PDF417ECModulusPoly* modulusPoly = NULL;
    if (coefficient == 0) {
        modulusPoly = FX_NEW CBC_PDF417ECModulusPoly(m_field->getZero()->m_field, m_field->getZero()->m_coefficients, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        return modulusPoly;
    }
    FX_INT32 size = m_coefficients.GetSize();
    CFX_Int32Array product;
    product.SetSize(size + degree);
    for (FX_INT32 i = 0; i < size; i++) {
        product[i] = m_field->multiply(m_coefficients[i], coefficient);
    }
    modulusPoly = FX_NEW CBC_PDF417ECModulusPoly(m_field, product, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return modulusPoly;
}
CFX_PtrArray* CBC_PDF417ECModulusPoly::divide(CBC_PDF417ECModulusPoly* other, FX_INT32 &e)
{
    if (other->isZero()) {
        e = BCExceptionDivideByZero;
        return NULL;
    }
    CBC_PDF417ECModulusPoly* quotient = FX_NEW CBC_PDF417ECModulusPoly(m_field->getZero()->m_field, m_field->getZero()->m_coefficients, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    CBC_PDF417ECModulusPoly* remainder = FX_NEW CBC_PDF417ECModulusPoly(m_field, m_coefficients, e);
    if (e != BCExceptionNO) {
        delete quotient;
        return NULL;
    }
    FX_INT32 denominatorLeadingTerm = other->getCoefficient(other->getDegree());
    FX_INT32 inverseDenominatorLeadingTerm = m_field->inverse(denominatorLeadingTerm, e);
    if (e != BCExceptionNO) {
        delete quotient;
        delete remainder;
        return NULL;
    }
    while (remainder->getDegree() >= other->getDegree() && !remainder->isZero()) {
        FX_INT32 degreeDifference = remainder->getDegree() - other->getDegree();
        FX_INT32 scale = m_field->multiply(remainder->getCoefficient(remainder->getDegree()), inverseDenominatorLeadingTerm);
        CBC_PDF417ECModulusPoly* term = other->multiplyByMonomial(degreeDifference, scale, e);
        if (e != BCExceptionNO) {
            delete quotient;
            delete remainder;
            return NULL;
        }
        CBC_PDF417ECModulusPoly* iterationQuotient = m_field->buildMonomial(degreeDifference, scale, e);
        if (e != BCExceptionNO) {
            delete quotient;
            delete remainder;
            delete term;
            return NULL;
        }
        CBC_PDF417ECModulusPoly* temp = quotient;
        quotient = temp->add(iterationQuotient, e);
        delete iterationQuotient;
        delete temp;
        if (e != BCExceptionNO) {
            delete remainder;
            return NULL;
        }
        temp = remainder;
        remainder = temp->subtract(term, e);
        delete term;
        delete temp;
        if (e != BCExceptionNO) {
            delete quotient;
            return NULL;
        }
    }
    CFX_PtrArray* modulusPoly = FX_NEW CFX_PtrArray;
    modulusPoly->Add(quotient);
    modulusPoly->Add(remainder);
    return modulusPoly;
}
CFX_ByteString CBC_PDF417ECModulusPoly::toString()
{
    CFX_ByteString result;
    for (FX_INT32 degree = getDegree(); degree >= 0; degree--) {
        FX_INT32 coefficient = getCoefficient(degree);
        if (coefficient != 0) {
            if (coefficient < 0) {
                result += " - ";
                coefficient = -coefficient;
            } else {
                if (result.GetLength() > 0) {
                    result += " + ";
                }
            }
            if (degree == 0 || coefficient != 1) {
                result += coefficient;
            }
            if (degree != 0) {
                if (degree == 1) {
                    result += 'x';
                } else {
                    result += "x^";
                    result += degree;
                }
            }
        }
    }
    return result;
}
