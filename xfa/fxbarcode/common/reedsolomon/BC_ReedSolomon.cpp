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

#include "xfa/fxbarcode/common/reedsolomon/BC_ReedSolomon.h"

#include <memory>

#include "xfa/fxbarcode/common/reedsolomon/BC_ReedSolomonGF256.h"
#include "xfa/fxbarcode/common/reedsolomon/BC_ReedSolomonGF256Poly.h"

CBC_ReedSolomonEncoder::CBC_ReedSolomonEncoder(CBC_ReedSolomonGF256* field) {
  m_field = field;
}

void CBC_ReedSolomonEncoder::Init() {
  m_cachedGenerators.push_back(new CBC_ReedSolomonGF256Poly(m_field, 1));
}

CBC_ReedSolomonGF256Poly* CBC_ReedSolomonEncoder::BuildGenerator(size_t degree,
                                                                 int32_t& e) {
  if (degree >= m_cachedGenerators.size()) {
    CBC_ReedSolomonGF256Poly* lastGenerator = m_cachedGenerators.back();
    for (size_t d = m_cachedGenerators.size(); d <= degree; ++d) {
      std::vector<int32_t> temp = {1, m_field->Exp(d - 1)};
      CBC_ReedSolomonGF256Poly temp_poly;
      temp_poly.Init(m_field, &temp, e);
      if (e != BCExceptionNO)
        return nullptr;
      CBC_ReedSolomonGF256Poly* nextGenerator =
          lastGenerator->Multiply(&temp_poly, e);
      if (e != BCExceptionNO)
        return nullptr;
      m_cachedGenerators.push_back(nextGenerator);
      lastGenerator = nextGenerator;
    }
  }
  return m_cachedGenerators[degree];
}

void CBC_ReedSolomonEncoder::Encode(std::vector<int32_t>* toEncode,
                                    size_t ecBytes,
                                    int32_t& e) {
  if (ecBytes == 0) {
    e = BCExceptionNoCorrectionBytes;
    return;
  }
  if (toEncode->size() <= ecBytes) {
    e = BCExceptionNoDataBytesProvided;
    return;
  }
  CBC_ReedSolomonGF256Poly* generator = BuildGenerator(ecBytes, e);
  if (e != BCExceptionNO)
    return;
  size_t dataBytes = toEncode->size() - ecBytes;
  std::vector<int32_t> infoCoefficients(dataBytes);
  for (size_t x = 0; x < dataBytes; x++) {
    infoCoefficients[x] = (*toEncode)[x];
  }
  CBC_ReedSolomonGF256Poly info;
  info.Init(m_field, &infoCoefficients, e);
  if (e != BCExceptionNO)
    return;
  std::unique_ptr<CBC_ReedSolomonGF256Poly> infoTemp(
      info.MultiplyByMonomial(ecBytes, 1, e));
  if (e != BCExceptionNO)
    return;
  std::unique_ptr<std::vector<CBC_ReedSolomonGF256Poly*>> temp(
      infoTemp->Divide(generator, e));
  if (e != BCExceptionNO)
    return;
  CBC_ReedSolomonGF256Poly* remainder = (*temp)[1];
  std::vector<int32_t>* coefficients = remainder->GetCoefficients();
  size_t numZeroCoefficients =
      ecBytes > coefficients->size() ? ecBytes - coefficients->size() : 0;
  for (size_t i = 0; i < numZeroCoefficients; i++)
    (*toEncode)[dataBytes + i] = 0;
  for (size_t y = 0; y < coefficients->size(); y++)
    (*toEncode)[dataBytes + numZeroCoefficients + y] = (*coefficients)[y];
  for (size_t k = 0; k < temp->size(); k++)
    delete (*temp)[k];
}

CBC_ReedSolomonEncoder::~CBC_ReedSolomonEncoder() {
  for (size_t i = 0; i < m_cachedGenerators.size(); i++)
    delete m_cachedGenerators[i];
}
