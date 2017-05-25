// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_expintfunc.h"

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxcrt/fx_memory.h"

CPDF_ExpIntFunc::CPDF_ExpIntFunc()
    : CPDF_Function(Type::kType2ExpotentialInterpolation),
      m_pBeginValues(nullptr),
      m_pEndValues(nullptr) {}

CPDF_ExpIntFunc::~CPDF_ExpIntFunc() {
  FX_Free(m_pBeginValues);
  FX_Free(m_pEndValues);
}

bool CPDF_ExpIntFunc::v_Init(CPDF_Object* pObj) {
  CPDF_Dictionary* pDict = pObj->GetDict();
  if (!pDict)
    return false;

  CPDF_Array* pArray0 = pDict->GetArrayFor("C0");
  if (m_nOutputs == 0) {
    m_nOutputs = 1;
    if (pArray0)
      m_nOutputs = pArray0->GetCount();
  }

  CPDF_Array* pArray1 = pDict->GetArrayFor("C1");
  m_pBeginValues = FX_Alloc2D(float, m_nOutputs, 2);
  m_pEndValues = FX_Alloc2D(float, m_nOutputs, 2);
  for (uint32_t i = 0; i < m_nOutputs; i++) {
    m_pBeginValues[i] = pArray0 ? pArray0->GetFloatAt(i) : 0.0f;
    m_pEndValues[i] = pArray1 ? pArray1->GetFloatAt(i) : 1.0f;
  }

  m_Exponent = pDict->GetFloatFor("N");
  m_nOrigOutputs = m_nOutputs;
  if (m_nOutputs && m_nInputs > INT_MAX / m_nOutputs)
    return false;

  m_nOutputs *= m_nInputs;
  return true;
}

bool CPDF_ExpIntFunc::v_Call(float* inputs, float* results) const {
  for (uint32_t i = 0; i < m_nInputs; i++)
    for (uint32_t j = 0; j < m_nOrigOutputs; j++) {
      results[i * m_nOrigOutputs + j] =
          m_pBeginValues[j] + FXSYS_pow(inputs[i], m_Exponent) *
                                  (m_pEndValues[j] - m_pBeginValues[j]);
    }
  return true;
}
