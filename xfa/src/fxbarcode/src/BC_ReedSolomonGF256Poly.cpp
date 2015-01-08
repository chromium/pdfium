// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2007 ZXing authors
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
#include "include/BC_ReedSolomonGF256.h"
#include "include/BC_ReedSolomonGF256Poly.h"
CBC_ReedSolomonGF256Poly::CBC_ReedSolomonGF256Poly(CBC_ReedSolomonGF256* field, FX_INT32 coefficients)
{
    if(field == NULL) {
        return;
    }
    m_field = field;
    m_coefficients.Add(coefficients);
}
CBC_ReedSolomonGF256Poly::CBC_ReedSolomonGF256Poly()
{
    m_field = NULL;
}
void CBC_ReedSolomonGF256Poly::Init(CBC_ReedSolomonGF256* field, CFX_Int32Array* coefficients, FX_INT32 &e)
{
    if(coefficients == NULL || coefficients->GetSize() == 0) {
        e = BCExceptionCoefficientsSizeIsNull;
        BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
    m_field = field;
    FX_INT32 coefficientsLength = coefficients->GetSize();
    if((coefficientsLength > 1 && (*coefficients)[0] == 0)) {
        FX_INT32 firstNonZero = 1;
        while((firstNonZero < coefficientsLength) && ((*coefficients)[firstNonZero] == 0)) {
            firstNonZero++;
        }
        if(firstNonZero == coefficientsLength) {
            m_coefficients.Copy( *(m_field->GetZero()->GetCoefficients()));
        } else {
            m_coefficients.SetSize(coefficientsLength - firstNonZero);
            for(FX_INT32 i = firstNonZero, j = 0; i < coefficientsLength; i++, j++) {
                m_coefficients[j] = coefficients->operator [](i);
            }
        }
    } else {
        m_coefficients.Copy(*coefficients);
    }
}
CFX_Int32Array* CBC_ReedSolomonGF256Poly::GetCoefficients()
{
    return &m_coefficients;
}
FX_INT32 CBC_ReedSolomonGF256Poly::GetDegree()
{
    return m_coefficients.GetSize() - 1;
}
FX_BOOL CBC_ReedSolomonGF256Poly::IsZero()
{
    return m_coefficients[0] == 0;
}
FX_INT32 CBC_ReedSolomonGF256Poly::GetCoefficients(FX_INT32 degree)
{
    return m_coefficients[m_coefficients.GetSize() - 1 - degree];
}
FX_INT32 CBC_ReedSolomonGF256Poly::EvaluateAt(FX_INT32 a)
{
    if(a == 0) {
        return GetCoefficients(0);
    }
    FX_INT32 size = m_coefficients.GetSize();
    if(a == 1) {
        FX_INT32 result = 0;
        for(FX_INT32 i = 0; i < size; i++) {
            result = CBC_ReedSolomonGF256::AddOrSubtract(result, m_coefficients[i]);
        }
        return result;
    }
    FX_INT32 result = m_coefficients[0];
    for(FX_INT32 j = 1; j < size; j++) {
        result = CBC_ReedSolomonGF256::AddOrSubtract(
                     m_field->Multiply(a, result),
                     m_coefficients[j]);
    }
    return result;
}
CBC_ReedSolomonGF256Poly *CBC_ReedSolomonGF256Poly::Clone(FX_INT32 &e)
{
    CBC_ReedSolomonGF256Poly *temp  = FX_NEW CBC_ReedSolomonGF256Poly();
    temp->Init(m_field, &m_coefficients, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return temp;
}
CBC_ReedSolomonGF256Poly* CBC_ReedSolomonGF256Poly::AddOrSubtract(CBC_ReedSolomonGF256Poly* other, FX_INT32 &e)
{
    if(IsZero()) {
        return other->Clone(e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    if(other->IsZero()) {
        return this->Clone(e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    CFX_Int32Array smallerCoefficients;
    smallerCoefficients.Copy(m_coefficients);
    CFX_Int32Array largerCoefficients;
    largerCoefficients.Copy( *(other->GetCoefficients()));
    if(smallerCoefficients.GetSize() > largerCoefficients.GetSize()) {
        CFX_Int32Array temp;
        temp.Copy(smallerCoefficients);
        smallerCoefficients.Copy(largerCoefficients);
        largerCoefficients.Copy(temp);
    }
    CFX_Int32Array sumDiff;
    sumDiff.SetSize(largerCoefficients.GetSize() );
    FX_INT32 lengthDiff = largerCoefficients.GetSize() - smallerCoefficients.GetSize();
    for(FX_INT32 i = 0; i < lengthDiff; i++) {
        sumDiff[i] = largerCoefficients[i];
    }
    for(FX_INT32 j = lengthDiff; j < largerCoefficients.GetSize(); j++) {
        sumDiff[j] = (CBC_ReedSolomonGF256::AddOrSubtract(smallerCoefficients[j - lengthDiff],
                      largerCoefficients[j]));
    }
    CBC_ReedSolomonGF256Poly *temp = FX_NEW CBC_ReedSolomonGF256Poly();
    temp->Init(m_field, &sumDiff, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return temp;
}
CBC_ReedSolomonGF256Poly* CBC_ReedSolomonGF256Poly::Multiply(CBC_ReedSolomonGF256Poly* other, FX_INT32 &e)
{
    if(IsZero() || other->IsZero()) {
        CBC_ReedSolomonGF256Poly *temp = m_field->GetZero()->Clone(e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        return temp;
    }
    CFX_Int32Array aCoefficients ;
    aCoefficients.Copy(m_coefficients);
    FX_INT32 aLength = m_coefficients.GetSize();
    CFX_Int32Array bCoefficients;
    bCoefficients.Copy(*(other->GetCoefficients()));
    FX_INT32 bLength = other->GetCoefficients()->GetSize();
    CFX_Int32Array product;
    product.SetSize(aLength + bLength - 1);
    for(FX_INT32 i = 0; i < aLength; i++) {
        FX_INT32 aCoeff = m_coefficients[i];
        for(FX_INT32 j = 0; j < bLength; j++) {
            product[i + j] = CBC_ReedSolomonGF256::AddOrSubtract(
                                 product[i + j],
                                 m_field->Multiply(aCoeff, other->GetCoefficients()->operator [](j)));
        }
    }
    CBC_ReedSolomonGF256Poly *temp = FX_NEW CBC_ReedSolomonGF256Poly();
    temp->Init(m_field, &product, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return temp;
}
CBC_ReedSolomonGF256Poly* CBC_ReedSolomonGF256Poly::Multiply(FX_INT32 scalar, FX_INT32 &e)
{
    if(scalar == 0) {
        CBC_ReedSolomonGF256Poly *temp = m_field->GetZero()->Clone(e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        return temp;
    }
    if(scalar == 1) {
        return this->Clone(e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    FX_INT32 size = m_coefficients.GetSize();
    CFX_Int32Array product;
    product.SetSize(size);
    for(FX_INT32 i = 0; i < size; i++) {
        product[i] = m_field->Multiply(m_coefficients[i], scalar);
    }
    CBC_ReedSolomonGF256Poly *temp = FX_NEW CBC_ReedSolomonGF256Poly();
    temp->Init(m_field, &product, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return temp;
}
CBC_ReedSolomonGF256Poly* CBC_ReedSolomonGF256Poly::MultiplyByMonomial(FX_INT32 degree, FX_INT32 coefficient, FX_INT32 &e)
{
    if(degree < 0) {
        e = BCExceptionDegreeIsNegative;
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    if(coefficient == 0) {
        CBC_ReedSolomonGF256Poly *temp = m_field->GetZero()->Clone(e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        return temp;
    }
    FX_INT32 size = m_coefficients.GetSize();
    CFX_Int32Array product;
    product.SetSize(size + degree);
    for(FX_INT32 i = 0; i < size; i++) {
        product[i] = (m_field->Multiply(m_coefficients[i], coefficient));
    }
    CBC_ReedSolomonGF256Poly *temp = FX_NEW CBC_ReedSolomonGF256Poly();
    temp->Init(m_field, &product, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return temp;
}
CFX_PtrArray* CBC_ReedSolomonGF256Poly::Divide(CBC_ReedSolomonGF256Poly *other, FX_INT32 &e)
{
    if(other->IsZero()) {
        e = BCExceptionDivideByZero;
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    CBC_ReedSolomonGF256Poly* rsg1 = m_field->GetZero()->Clone(e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    CBC_AutoPtr<CBC_ReedSolomonGF256Poly> quotient(rsg1);
    CBC_ReedSolomonGF256Poly* rsg2 = this->Clone(e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    CBC_AutoPtr<CBC_ReedSolomonGF256Poly> remainder(rsg2);
    FX_INT32 denominatorLeadingTerm = other->GetCoefficients(other->GetDegree());
    FX_INT32 inverseDenominatorLeadingTeam = m_field->Inverse(denominatorLeadingTerm, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    FX_BOOL bFirst = TRUE;
    while(remainder->GetDegree() >= other->GetDegree() && !remainder->IsZero()) {
        FX_INT32 degreeDifference = remainder->GetDegree() - other->GetDegree();
        FX_INT32 scale = m_field->Multiply(remainder->GetCoefficients((remainder->GetDegree())),
                                           inverseDenominatorLeadingTeam);
        CBC_ReedSolomonGF256Poly* rsg3 = other->MultiplyByMonomial(degreeDifference, scale, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        CBC_AutoPtr<CBC_ReedSolomonGF256Poly> term(rsg3);
        CBC_ReedSolomonGF256Poly* rsg4 = m_field->BuildMonomial(degreeDifference, scale, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        CBC_AutoPtr<CBC_ReedSolomonGF256Poly> iteratorQuotient(rsg4);
        CBC_ReedSolomonGF256Poly* rsg5 = quotient->AddOrSubtract(iteratorQuotient.get(), e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        CBC_AutoPtr<CBC_ReedSolomonGF256Poly> temp(rsg5);
        quotient = temp;
        CBC_ReedSolomonGF256Poly* rsg6 = remainder->AddOrSubtract(term.get(), e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        CBC_AutoPtr<CBC_ReedSolomonGF256Poly> temp1(rsg6);
        remainder = temp1;
    }
    CFX_PtrArray* tempPtrA = FX_NEW CFX_PtrArray;
    tempPtrA->Add(quotient.release());
    tempPtrA->Add(remainder.release());
    return tempPtrA;
}
CBC_ReedSolomonGF256Poly::~CBC_ReedSolomonGF256Poly()
{
    m_coefficients.RemoveAll();
}
