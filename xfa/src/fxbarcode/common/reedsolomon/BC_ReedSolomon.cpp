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

#include "xfa/src/fxbarcode/barcode.h"
#include "BC_ReedSolomonGF256.h"
#include "BC_ReedSolomonGF256Poly.h"
#include "BC_ReedSolomon.h"
CBC_ReedSolomonEncoder::CBC_ReedSolomonEncoder(CBC_ReedSolomonGF256* field) {
  m_field = field;
}
void CBC_ReedSolomonEncoder::Init() {
  m_cachedGenerators.Add(new CBC_ReedSolomonGF256Poly(m_field, 1));
}
CBC_ReedSolomonGF256Poly* CBC_ReedSolomonEncoder::BuildGenerator(int32_t degree,
                                                                 int32_t& e) {
  if (degree >= m_cachedGenerators.GetSize()) {
    CBC_ReedSolomonGF256Poly* lastGenerator =
        (CBC_ReedSolomonGF256Poly*)(m_cachedGenerators
                                        [m_cachedGenerators.GetSize() - 1]);
    for (int32_t d = m_cachedGenerators.GetSize(); d <= degree; d++) {
      CFX_Int32Array temp;
      temp.Add(1);
      temp.Add(m_field->Exp(d - 1));
      CBC_ReedSolomonGF256Poly temp_poly;
      temp_poly.Init(m_field, &temp, e);
      BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
      CBC_ReedSolomonGF256Poly* nextGenerator =
          lastGenerator->Multiply(&temp_poly, e);
      BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
      m_cachedGenerators.Add(nextGenerator);
      lastGenerator = nextGenerator;
    }
  }
  return (CBC_ReedSolomonGF256Poly*)(m_cachedGenerators[degree]);
}
void CBC_ReedSolomonEncoder::Encode(CFX_Int32Array* toEncode,
                                    int32_t ecBytes,
                                    int32_t& e) {
  if (ecBytes == 0) {
    e = BCExceptionNoCorrectionBytes;
    BC_EXCEPTION_CHECK_ReturnVoid(e);
  }
  int32_t dataBytes = toEncode->GetSize() - ecBytes;
  if (dataBytes <= 0) {
    e = BCExceptionNoDataBytesProvided;
    BC_EXCEPTION_CHECK_ReturnVoid(e);
  }
  CBC_ReedSolomonGF256Poly* generator = BuildGenerator(ecBytes, e);
  BC_EXCEPTION_CHECK_ReturnVoid(e);
  CFX_Int32Array infoCoefficients;
  infoCoefficients.SetSize(dataBytes);
  for (int32_t x = 0; x < dataBytes; x++) {
    infoCoefficients[x] = toEncode->operator[](x);
  }
  CBC_ReedSolomonGF256Poly info;
  info.Init(m_field, &infoCoefficients, e);
  BC_EXCEPTION_CHECK_ReturnVoid(e);
  CBC_ReedSolomonGF256Poly* rsg = info.MultiplyByMonomial(ecBytes, 1, e);
  BC_EXCEPTION_CHECK_ReturnVoid(e);
  CBC_AutoPtr<CBC_ReedSolomonGF256Poly> infoTemp(rsg);
  CFX_PtrArray* pa = infoTemp->Divide(generator, e);
  BC_EXCEPTION_CHECK_ReturnVoid(e);
  CBC_AutoPtr<CFX_PtrArray> temp(pa);
  CBC_ReedSolomonGF256Poly* remainder =
      (CBC_ReedSolomonGF256Poly*)(temp->operator[](1));
  CFX_Int32Array* coefficients = remainder->GetCoefficients();
  int32_t numZeroCoefficients = ecBytes - coefficients->GetSize();
  for (int32_t i = 0; i < numZeroCoefficients; i++) {
    (*toEncode)[dataBytes + i] = 0;
  }
  for (int32_t y = 0; y < coefficients->GetSize(); y++) {
    (*toEncode)[dataBytes + numZeroCoefficients + y] =
        coefficients->operator[](y);
  }
  for (int32_t k = 0; k < temp->GetSize(); k++) {
    delete (CBC_ReedSolomonGF256Poly*)(*temp)[k];
  }
}
CBC_ReedSolomonEncoder::~CBC_ReedSolomonEncoder() {
  for (int32_t i = 0; i < m_cachedGenerators.GetSize(); i++) {
    delete (CBC_ReedSolomonGF256Poly*)m_cachedGenerators[i];
  }
}
