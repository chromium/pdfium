// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_function.h"

#include "core/fpdfapi/page/cpdf_expintfunc.h"
#include "core/fpdfapi/page/cpdf_psfunc.h"
#include "core/fpdfapi/page/cpdf_sampledfunc.h"
#include "core/fpdfapi/page/cpdf_stitchfunc.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "third_party/base/ptr_util.h"

// static
std::unique_ptr<CPDF_Function> CPDF_Function::Load(CPDF_Object* pFuncObj) {
  std::unique_ptr<CPDF_Function> pFunc;
  if (!pFuncObj)
    return pFunc;

  int iType = -1;
  if (CPDF_Stream* pStream = pFuncObj->AsStream())
    iType = pStream->GetDict()->GetIntegerFor("FunctionType");
  else if (CPDF_Dictionary* pDict = pFuncObj->AsDictionary())
    iType = pDict->GetIntegerFor("FunctionType");

  Type type = IntegerToFunctionType(iType);
  if (type == Type::kType0Sampled)
    pFunc = pdfium::MakeUnique<CPDF_SampledFunc>();
  else if (type == Type::kType2ExpotentialInterpolation)
    pFunc = pdfium::MakeUnique<CPDF_ExpIntFunc>();
  else if (type == Type::kType3Stitching)
    pFunc = pdfium::MakeUnique<CPDF_StitchFunc>();
  else if (type == Type::kType4PostScript)
    pFunc = pdfium::MakeUnique<CPDF_PSFunc>();

  if (!pFunc || !pFunc->Init(pFuncObj))
    return nullptr;

  return pFunc;
}

// static
CPDF_Function::Type CPDF_Function::IntegerToFunctionType(int iType) {
  switch (iType) {
    case 0:
    case 2:
    case 3:
    case 4:
      return static_cast<Type>(iType);
    default:
      return Type::kTypeInvalid;
  }
}

CPDF_Function::CPDF_Function(Type type)
    : m_pDomains(nullptr), m_pRanges(nullptr), m_Type(type) {}

CPDF_Function::~CPDF_Function() {
  FX_Free(m_pDomains);
  FX_Free(m_pRanges);
}

bool CPDF_Function::Init(CPDF_Object* pObj) {
  CPDF_Stream* pStream = pObj->AsStream();
  CPDF_Dictionary* pDict = pStream ? pStream->GetDict() : pObj->AsDictionary();

  CPDF_Array* pDomains = pDict->GetArrayFor("Domain");
  if (!pDomains)
    return false;

  m_nInputs = pDomains->GetCount() / 2;
  if (m_nInputs == 0)
    return false;

  m_pDomains = FX_Alloc2D(float, m_nInputs, 2);
  for (uint32_t i = 0; i < m_nInputs * 2; i++) {
    m_pDomains[i] = pDomains->GetFloatAt(i);
  }
  CPDF_Array* pRanges = pDict->GetArrayFor("Range");
  m_nOutputs = 0;
  if (pRanges) {
    m_nOutputs = pRanges->GetCount() / 2;
    m_pRanges = FX_Alloc2D(float, m_nOutputs, 2);
    for (uint32_t i = 0; i < m_nOutputs * 2; i++)
      m_pRanges[i] = pRanges->GetFloatAt(i);
  }
  uint32_t old_outputs = m_nOutputs;
  if (!v_Init(pObj))
    return false;
  if (m_pRanges && m_nOutputs > old_outputs) {
    m_pRanges = FX_Realloc(float, m_pRanges, m_nOutputs * 2);
    if (m_pRanges) {
      memset(m_pRanges + (old_outputs * 2), 0,
             sizeof(float) * (m_nOutputs - old_outputs) * 2);
    }
  }
  return true;
}

bool CPDF_Function::Call(float* inputs,
                         uint32_t ninputs,
                         float* results,
                         int* nresults) const {
  if (m_nInputs != ninputs)
    return false;

  *nresults = m_nOutputs;
  for (uint32_t i = 0; i < m_nInputs; i++) {
    inputs[i] =
        pdfium::clamp(inputs[i], m_pDomains[i * 2], m_pDomains[i * 2 + 1]);
  }
  v_Call(inputs, results);
  if (!m_pRanges)
    return true;

  for (uint32_t i = 0; i < m_nOutputs; i++) {
    results[i] =
        pdfium::clamp(results[i], m_pRanges[i * 2], m_pRanges[i * 2 + 1]);
  }
  return true;
}

// See PDF Reference 1.7, page 170.
float CPDF_Function::Interpolate(float x,
                                 float xmin,
                                 float xmax,
                                 float ymin,
                                 float ymax) const {
  float divisor = xmax - xmin;
  return ymin + (divisor ? (x - xmin) * (ymax - ymin) / divisor : 0);
}

const CPDF_SampledFunc* CPDF_Function::ToSampledFunc() const {
  return m_Type == Type::kType0Sampled
             ? static_cast<const CPDF_SampledFunc*>(this)
             : nullptr;
}

const CPDF_ExpIntFunc* CPDF_Function::ToExpIntFunc() const {
  return m_Type == Type::kType2ExpotentialInterpolation
             ? static_cast<const CPDF_ExpIntFunc*>(this)
             : nullptr;
}

const CPDF_StitchFunc* CPDF_Function::ToStitchFunc() const {
  return m_Type == Type::kType3Stitching
             ? static_cast<const CPDF_StitchFunc*>(this)
             : nullptr;
}
