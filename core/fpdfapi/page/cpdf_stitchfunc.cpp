// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_stitchfunc.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxcrt/fx_memory.h"

CPDF_StitchFunc::CPDF_StitchFunc()
    : CPDF_Function(Type::kType3Stitching),
      m_pBounds(nullptr),
      m_pEncode(nullptr) {}

CPDF_StitchFunc::~CPDF_StitchFunc() {
  FX_Free(m_pBounds);
  FX_Free(m_pEncode);
}

bool CPDF_StitchFunc::v_Init(CPDF_Object* pObj) {
  CPDF_Dictionary* pDict = pObj->GetDict();
  if (!pDict) {
    return false;
  }
  if (m_nInputs != kRequiredNumInputs) {
    return false;
  }
  CPDF_Array* pArray = pDict->GetArrayFor("Functions");
  if (!pArray) {
    return false;
  }
  uint32_t nSubs = pArray->GetCount();
  if (nSubs == 0)
    return false;
  m_nOutputs = 0;
  for (uint32_t i = 0; i < nSubs; i++) {
    CPDF_Object* pSub = pArray->GetDirectObjectAt(i);
    if (pSub == pObj)
      return false;
    std::unique_ptr<CPDF_Function> pFunc(CPDF_Function::Load(pSub));
    if (!pFunc)
      return false;
    // Check that the input dimensionality is 1, and that all output
    // dimensionalities are the same.
    if (pFunc->CountInputs() != kRequiredNumInputs)
      return false;
    if (pFunc->CountOutputs() != m_nOutputs) {
      if (m_nOutputs)
        return false;

      m_nOutputs = pFunc->CountOutputs();
    }

    m_pSubFunctions.push_back(std::move(pFunc));
  }
  m_pBounds = FX_Alloc(float, nSubs + 1);
  m_pBounds[0] = m_pDomains[0];
  pArray = pDict->GetArrayFor("Bounds");
  if (!pArray)
    return false;
  for (uint32_t i = 0; i < nSubs - 1; i++)
    m_pBounds[i + 1] = pArray->GetFloatAt(i);
  m_pBounds[nSubs] = m_pDomains[1];
  m_pEncode = FX_Alloc2D(float, nSubs, 2);
  pArray = pDict->GetArrayFor("Encode");
  if (!pArray)
    return false;

  for (uint32_t i = 0; i < nSubs * 2; i++)
    m_pEncode[i] = pArray->GetFloatAt(i);
  return true;
}

bool CPDF_StitchFunc::v_Call(float* inputs, float* outputs) const {
  float input = inputs[0];
  size_t i;
  for (i = 0; i < m_pSubFunctions.size() - 1; i++) {
    if (input < m_pBounds[i + 1])
      break;
  }
  input = Interpolate(input, m_pBounds[i], m_pBounds[i + 1], m_pEncode[i * 2],
                      m_pEncode[i * 2 + 1]);
  int nresults;
  m_pSubFunctions[i]->Call(&input, kRequiredNumInputs, outputs, &nresults);
  return true;
}
