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

#include "xfa/fxbarcode/common/reedsolomon/BC_ReedSolomonDecoder.h"

#include <memory>
#include <utility>

#include "xfa/fxbarcode/common/reedsolomon/BC_ReedSolomonGF256.h"
#include "xfa/fxbarcode/common/reedsolomon/BC_ReedSolomonGF256Poly.h"

CBC_ReedSolomonDecoder::CBC_ReedSolomonDecoder(CBC_ReedSolomonGF256* field) {
  m_field = field;
}
CBC_ReedSolomonDecoder::~CBC_ReedSolomonDecoder() {}
void CBC_ReedSolomonDecoder::Decode(CFX_Int32Array* received,
                                    int32_t twoS,
                                    int32_t& e) {
  CBC_ReedSolomonGF256Poly poly;
  poly.Init(m_field, received, e);
  BC_EXCEPTION_CHECK_ReturnVoid(e);
  CFX_Int32Array syndromeCoefficients;
  syndromeCoefficients.SetSize(twoS);
  FX_BOOL dataMatrix = FALSE;
  FX_BOOL noError = TRUE;
  for (int32_t i = 0; i < twoS; i++) {
    int32_t eval = poly.EvaluateAt(m_field->Exp(dataMatrix ? i + 1 : i));
    syndromeCoefficients[twoS - 1 - i] = eval;
    if (eval != 0) {
      noError = FALSE;
    }
  }
  if (noError) {
    return;
  }
  CBC_ReedSolomonGF256Poly syndrome;
  syndrome.Init(m_field, &syndromeCoefficients, e);
  BC_EXCEPTION_CHECK_ReturnVoid(e);
  std::unique_ptr<CBC_ReedSolomonGF256Poly> temp(
      m_field->BuildMonomial(twoS, 1, e));
  BC_EXCEPTION_CHECK_ReturnVoid(e);
  std::unique_ptr<CFX_ArrayTemplate<CBC_ReedSolomonGF256Poly*>> sigmaOmega(
      RunEuclideanAlgorithm(temp.get(), &syndrome, twoS, e));
  BC_EXCEPTION_CHECK_ReturnVoid(e);
  std::unique_ptr<CBC_ReedSolomonGF256Poly> sigma((*sigmaOmega)[0]);
  std::unique_ptr<CBC_ReedSolomonGF256Poly> omega((*sigmaOmega)[1]);
  std::unique_ptr<CFX_Int32Array> errorLocations(
      FindErrorLocations(sigma.get(), e));
  BC_EXCEPTION_CHECK_ReturnVoid(e);
  std::unique_ptr<CFX_Int32Array> errorMagnitudes(
      FindErrorMagnitudes(omega.get(), errorLocations.get(), dataMatrix, e));
  BC_EXCEPTION_CHECK_ReturnVoid(e);
  for (int32_t k = 0; k < errorLocations->GetSize(); k++) {
    int32_t position =
        received->GetSize() - 1 - m_field->Log((*errorLocations)[k], e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    if (position < 0) {
      e = BCExceptionBadErrorLocation;
      BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
    (*received)[position] = CBC_ReedSolomonGF256::AddOrSubtract(
        (*received)[position], (*errorMagnitudes)[k]);
  }
}

CFX_ArrayTemplate<CBC_ReedSolomonGF256Poly*>*
CBC_ReedSolomonDecoder::RunEuclideanAlgorithm(CBC_ReedSolomonGF256Poly* a,
                                              CBC_ReedSolomonGF256Poly* b,
                                              int32_t R,
                                              int32_t& e) {
  if (a->GetDegree() < b->GetDegree()) {
    CBC_ReedSolomonGF256Poly* temp = a;
    a = b;
    b = temp;
  }
  std::unique_ptr<CBC_ReedSolomonGF256Poly> rLast(a->Clone(e));
  BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
  std::unique_ptr<CBC_ReedSolomonGF256Poly> r(b->Clone(e));
  BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
  std::unique_ptr<CBC_ReedSolomonGF256Poly> sLast(m_field->GetOne()->Clone(e));
  BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
  std::unique_ptr<CBC_ReedSolomonGF256Poly> s(m_field->GetZero()->Clone(e));
  BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
  std::unique_ptr<CBC_ReedSolomonGF256Poly> tLast(m_field->GetZero()->Clone(e));
  BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
  std::unique_ptr<CBC_ReedSolomonGF256Poly> t(m_field->GetOne()->Clone(e));
  BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
  while (r->GetDegree() >= R / 2) {
    std::unique_ptr<CBC_ReedSolomonGF256Poly> rLastLast = std::move(rLast);
    std::unique_ptr<CBC_ReedSolomonGF256Poly> sLastLast = std::move(sLast);
    std::unique_ptr<CBC_ReedSolomonGF256Poly> tLastlast = std::move(tLast);
    rLast = std::move(r);
    sLast = std::move(s);
    tLast = std::move(t);
    if (rLast->IsZero()) {
      e = BCExceptionR_I_1IsZero;
      BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
    }
    r.reset(rLastLast->Clone(e));
    BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
    std::unique_ptr<CBC_ReedSolomonGF256Poly> q(m_field->GetZero()->Clone(e));
    BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
    int32_t denominatorLeadingTerm = rLast->GetCoefficients(rLast->GetDegree());
    int32_t dltInverse = m_field->Inverse(denominatorLeadingTerm, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
    while (r->GetDegree() >= rLast->GetDegree() && !(r->IsZero())) {
      int32_t degreeDiff = r->GetDegree() - rLast->GetDegree();
      int32_t scale =
          m_field->Multiply(r->GetCoefficients(r->GetDegree()), dltInverse);
      std::unique_ptr<CBC_ReedSolomonGF256Poly> build(
          m_field->BuildMonomial(degreeDiff, scale, e));
      BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
      q.reset(q->AddOrSubtract(build.get(), e));
      BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
      std::unique_ptr<CBC_ReedSolomonGF256Poly> multiply(
          rLast->MultiplyByMonomial(degreeDiff, scale, e));
      BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
      r.reset(r->AddOrSubtract(multiply.get(), e));
      BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
    }
    std::unique_ptr<CBC_ReedSolomonGF256Poly> temp1(
        q->Multiply(sLast.get(), e));
    BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
    s.reset(temp1->AddOrSubtract(sLastLast.get(), e));
    BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
    std::unique_ptr<CBC_ReedSolomonGF256Poly> temp5(
        q->Multiply(tLast.get(), e));
    BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
    t.reset(temp5->AddOrSubtract(tLastlast.get(), e));
    BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
  }
  int32_t sigmaTildeAtZero = t->GetCoefficients(0);
  if (sigmaTildeAtZero == 0) {
    e = BCExceptionIsZero;
    BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
  }
  int32_t inverse = m_field->Inverse(sigmaTildeAtZero, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
  std::unique_ptr<CBC_ReedSolomonGF256Poly> sigma(t->Multiply(inverse, e));
  BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
  std::unique_ptr<CBC_ReedSolomonGF256Poly> omega(r->Multiply(inverse, e));
  BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
  CFX_ArrayTemplate<CBC_ReedSolomonGF256Poly*>* temp =
      new CFX_ArrayTemplate<CBC_ReedSolomonGF256Poly*>();
  temp->Add(sigma.release());
  temp->Add(omega.release());
  return temp;
}
CFX_Int32Array* CBC_ReedSolomonDecoder::FindErrorLocations(
    CBC_ReedSolomonGF256Poly* errorLocator,
    int32_t& e) {
  int32_t numErrors = errorLocator->GetDegree();
  if (numErrors == 1) {
    std::unique_ptr<CFX_Int32Array> temp(new CFX_Int32Array);
    temp->Add(errorLocator->GetCoefficients(1));
    return temp.release();
  }
  CFX_Int32Array* tempT = new CFX_Int32Array;
  tempT->SetSize(numErrors);
  std::unique_ptr<CFX_Int32Array> result(tempT);
  int32_t ie = 0;
  for (int32_t i = 1; i < 256 && ie < numErrors; i++) {
    if (errorLocator->EvaluateAt(i) == 0) {
      (*result)[ie] = m_field->Inverse(i, ie);
      BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
      ie++;
    }
  }
  if (ie != numErrors) {
    e = BCExceptionDegreeNotMatchRoots;
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  }
  return result.release();
}
CFX_Int32Array* CBC_ReedSolomonDecoder::FindErrorMagnitudes(
    CBC_ReedSolomonGF256Poly* errorEvaluator,
    CFX_Int32Array* errorLocations,
    FX_BOOL dataMatrix,
    int32_t& e) {
  int32_t s = errorLocations->GetSize();
  CFX_Int32Array* temp = new CFX_Int32Array;
  temp->SetSize(s);
  std::unique_ptr<CFX_Int32Array> result(temp);
  for (int32_t i = 0; i < s; i++) {
    int32_t xiInverse = m_field->Inverse(errorLocations->operator[](i), e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    int32_t denominator = 1;
    for (int32_t j = 0; j < s; j++) {
      if (i != j) {
        denominator = m_field->Multiply(
            denominator, CBC_ReedSolomonGF256::AddOrSubtract(
                             1, m_field->Multiply(errorLocations->operator[](j),
                                                  xiInverse)));
      }
    }
    int32_t temp = m_field->Inverse(denominator, temp);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    (*result)[i] =
        m_field->Multiply(errorEvaluator->EvaluateAt(xiInverse), temp);
  }
  return result.release();
}
