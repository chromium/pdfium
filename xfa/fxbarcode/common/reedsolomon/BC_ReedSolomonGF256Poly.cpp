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

#include "xfa/fxbarcode/common/reedsolomon/BC_ReedSolomonGF256Poly.h"

#include <memory>
#include <utility>

#include "third_party/base/stl_util.h"
#include "xfa/fxbarcode/common/reedsolomon/BC_ReedSolomonGF256.h"

CBC_ReedSolomonGF256Poly::CBC_ReedSolomonGF256Poly(CBC_ReedSolomonGF256* field,
                                                   int32_t coefficients) {
  if (!field)
    return;

  m_field = field;
  m_coefficients.push_back(coefficients);
}

CBC_ReedSolomonGF256Poly::CBC_ReedSolomonGF256Poly() {
  m_field = nullptr;
}

void CBC_ReedSolomonGF256Poly::Init(CBC_ReedSolomonGF256* field,
                                    std::vector<int32_t>* coefficients,
                                    int32_t& e) {
  if (!coefficients || coefficients->empty()) {
    e = BCExceptionCoefficientsSizeIsNull;
    return;
  }
  m_field = field;
  size_t coefficientsLength = coefficients->size();
  if (coefficientsLength > 1 && coefficients->front() == 0) {
    size_t firstNonZero = 1;
    while (firstNonZero < coefficientsLength &&
           (*coefficients)[firstNonZero] == 0) {
      firstNonZero++;
    }
    if (firstNonZero == coefficientsLength) {
      m_coefficients = *(m_field->GetZero()->GetCoefficients());
    } else {
      m_coefficients.resize(coefficientsLength - firstNonZero);
      for (size_t i = firstNonZero, j = 0; i < coefficientsLength; i++, j++)
        m_coefficients[j] = (*coefficients)[i];
    }
  } else {
    m_coefficients = *coefficients;
  }
}

std::vector<int32_t>* CBC_ReedSolomonGF256Poly::GetCoefficients() {
  return &m_coefficients;
}

int32_t CBC_ReedSolomonGF256Poly::GetDegree() {
  return pdfium::CollectionSize<int32_t>(m_coefficients) - 1;
}

bool CBC_ReedSolomonGF256Poly::IsZero() {
  return m_coefficients.front() == 0;
}

int32_t CBC_ReedSolomonGF256Poly::GetCoefficients(int32_t degree) {
  return m_coefficients[m_coefficients.size() - 1 - degree];
}

int32_t CBC_ReedSolomonGF256Poly::EvaluateAt(int32_t a) {
  if (a == 0) {
    return GetCoefficients(0);
  }
  size_t size = m_coefficients.size();
  if (a == 1) {
    int32_t result = 0;
    for (size_t i = 0; i < size; i++)
      result = CBC_ReedSolomonGF256::AddOrSubtract(result, m_coefficients[i]);
    return result;
  }
  int32_t result = m_coefficients[0];
  for (size_t j = 1; j < size; j++) {
    result = CBC_ReedSolomonGF256::AddOrSubtract(m_field->Multiply(a, result),
                                                 m_coefficients[j]);
  }
  return result;
}

CBC_ReedSolomonGF256Poly* CBC_ReedSolomonGF256Poly::Clone(int32_t& e) {
  CBC_ReedSolomonGF256Poly* temp = new CBC_ReedSolomonGF256Poly();
  temp->Init(m_field, &m_coefficients, e);
  if (e != BCExceptionNO)
    return nullptr;
  return temp;
}
CBC_ReedSolomonGF256Poly* CBC_ReedSolomonGF256Poly::AddOrSubtract(
    CBC_ReedSolomonGF256Poly* other,
    int32_t& e) {
  if (IsZero())
    return other->Clone(e);
  if (other->IsZero())
    return Clone(e);

  std::vector<int32_t> smallerCoefficients = m_coefficients;
  std::vector<int32_t> largerCoefficients = *(other->GetCoefficients());
  if (smallerCoefficients.size() > largerCoefficients.size()) {
    std::swap(smallerCoefficients, largerCoefficients);
  }
  std::vector<int32_t> sumDiff(largerCoefficients.size());
  size_t lengthDiff = largerCoefficients.size() - smallerCoefficients.size();
  for (size_t i = 0; i < lengthDiff; i++) {
    sumDiff[i] = largerCoefficients[i];
  }
  for (size_t j = lengthDiff; j < largerCoefficients.size(); j++) {
    sumDiff[j] = CBC_ReedSolomonGF256::AddOrSubtract(
        smallerCoefficients[j - lengthDiff], largerCoefficients[j]);
  }
  CBC_ReedSolomonGF256Poly* temp = new CBC_ReedSolomonGF256Poly();
  temp->Init(m_field, &sumDiff, e);
  if (e != BCExceptionNO)
    return nullptr;
  return temp;
}
CBC_ReedSolomonGF256Poly* CBC_ReedSolomonGF256Poly::Multiply(
    CBC_ReedSolomonGF256Poly* other,
    int32_t& e) {
  if (IsZero() || other->IsZero())
    return m_field->GetZero()->Clone(e);

  std::vector<int32_t> aCoefficients = m_coefficients;
  std::vector<int32_t> bCoefficients = *(other->GetCoefficients());
  size_t aLength = aCoefficients.size();
  size_t bLength = bCoefficients.size();
  std::vector<int32_t> product(aLength + bLength - 1);
  for (size_t i = 0; i < aLength; i++) {
    int32_t aCoeff = m_coefficients[i];
    for (size_t j = 0; j < bLength; j++) {
      product[i + j] = CBC_ReedSolomonGF256::AddOrSubtract(
          product[i + j],
          m_field->Multiply(aCoeff, (*other->GetCoefficients())[j]));
    }
  }
  CBC_ReedSolomonGF256Poly* temp = new CBC_ReedSolomonGF256Poly();
  temp->Init(m_field, &product, e);
  if (e != BCExceptionNO)
    return nullptr;
  return temp;
}
CBC_ReedSolomonGF256Poly* CBC_ReedSolomonGF256Poly::Multiply(int32_t scalar,
                                                             int32_t& e) {
  if (scalar == 0)
    return m_field->GetZero()->Clone(e);
  if (scalar == 1)
    return Clone(e);

  size_t size = m_coefficients.size();
  std::vector<int32_t> product(size);
  for (size_t i = 0; i < size; i++) {
    product[i] = m_field->Multiply(m_coefficients[i], scalar);
  }
  CBC_ReedSolomonGF256Poly* temp = new CBC_ReedSolomonGF256Poly();
  temp->Init(m_field, &product, e);
  if (e != BCExceptionNO)
    return nullptr;
  return temp;
}
CBC_ReedSolomonGF256Poly* CBC_ReedSolomonGF256Poly::MultiplyByMonomial(
    int32_t degree,
    int32_t coefficient,
    int32_t& e) {
  if (degree < 0) {
    e = BCExceptionDegreeIsNegative;
    return nullptr;
  }
  if (coefficient == 0)
    return m_field->GetZero()->Clone(e);

  size_t size = m_coefficients.size();
  std::vector<int32_t> product(size + degree);
  for (size_t i = 0; i < size; i++) {
    product[i] = m_field->Multiply(m_coefficients[i], coefficient);
  }
  CBC_ReedSolomonGF256Poly* temp = new CBC_ReedSolomonGF256Poly();
  temp->Init(m_field, &product, e);
  if (e != BCExceptionNO)
    return nullptr;
  return temp;
}

std::vector<CBC_ReedSolomonGF256Poly*>* CBC_ReedSolomonGF256Poly::Divide(
    CBC_ReedSolomonGF256Poly* other,
    int32_t& e) {
  if (other->IsZero()) {
    e = BCExceptionDivideByZero;
    return nullptr;
  }
  std::unique_ptr<CBC_ReedSolomonGF256Poly> quotient(
      m_field->GetZero()->Clone(e));
  if (e != BCExceptionNO)
    return nullptr;
  std::unique_ptr<CBC_ReedSolomonGF256Poly> remainder(Clone(e));
  if (e != BCExceptionNO)
    return nullptr;
  int32_t denominatorLeadingTerm = other->GetCoefficients(other->GetDegree());
  int32_t inverseDenominatorLeadingTeam =
      m_field->Inverse(denominatorLeadingTerm, e);
  if (e != BCExceptionNO)
    return nullptr;
  while (remainder->GetDegree() >= other->GetDegree() && !remainder->IsZero()) {
    int32_t degreeDifference = remainder->GetDegree() - other->GetDegree();
    int32_t scale =
        m_field->Multiply(remainder->GetCoefficients((remainder->GetDegree())),
                          inverseDenominatorLeadingTeam);
    std::unique_ptr<CBC_ReedSolomonGF256Poly> term(
        other->MultiplyByMonomial(degreeDifference, scale, e));
    if (e != BCExceptionNO)
      return nullptr;
    std::unique_ptr<CBC_ReedSolomonGF256Poly> iteratorQuotient(
        m_field->BuildMonomial(degreeDifference, scale, e));
    if (e != BCExceptionNO)
      return nullptr;
    quotient.reset(quotient->AddOrSubtract(iteratorQuotient.get(), e));
    if (e != BCExceptionNO)
      return nullptr;
    remainder.reset(remainder->AddOrSubtract(term.get(), e));
    if (e != BCExceptionNO)
      return nullptr;
  }
  std::vector<CBC_ReedSolomonGF256Poly*>* tempPtrA =
      new std::vector<CBC_ReedSolomonGF256Poly*>();
  tempPtrA->push_back(quotient.release());
  tempPtrA->push_back(remainder.release());
  return tempPtrA;
}

CBC_ReedSolomonGF256Poly::~CBC_ReedSolomonGF256Poly() {}
