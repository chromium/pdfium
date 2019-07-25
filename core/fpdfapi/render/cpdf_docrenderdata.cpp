// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_docrenderdata.h"

#include <array>
#include <memory>
#include <utility>
#include <vector>

#include "core/fpdfapi/font/cpdf_type3font.h"
#include "core/fpdfapi/page/cpdf_dibbase.h"
#include "core/fpdfapi/page/cpdf_function.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/render/cpdf_transferfunc.h"
#include "core/fpdfapi/render/cpdf_type3cache.h"

namespace {

const int kMaxOutputs = 16;

}  // namespace

// static
CPDF_DocRenderData* CPDF_DocRenderData::FromDocument(
    const CPDF_Document* pDoc) {
  return static_cast<CPDF_DocRenderData*>(pDoc->GetRenderData());
}

CPDF_DocRenderData::CPDF_DocRenderData() = default;

CPDF_DocRenderData::~CPDF_DocRenderData() = default;

RetainPtr<CPDF_Type3Cache> CPDF_DocRenderData::GetCachedType3(
    CPDF_Type3Font* pFont) {
  auto it = m_Type3FaceMap.find(pFont);
  if (it != m_Type3FaceMap.end() && it->second)
    return pdfium::WrapRetain(it->second.Get());

  auto pCache = pdfium::MakeRetain<CPDF_Type3Cache>(pFont);
  m_Type3FaceMap[pFont].Reset(pCache.Get());
  return pCache;
}

RetainPtr<CPDF_TransferFunc> CPDF_DocRenderData::GetTransferFunc(
    const CPDF_Object* pObj) {
  if (!pObj)
    return nullptr;

  auto it = m_TransferFuncMap.find(pObj);
  if (it != m_TransferFuncMap.end() && it->second)
    return pdfium::WrapRetain(it->second.Get());

  auto pFunc = CreateTransferFunc(pObj);
  m_TransferFuncMap[pObj].Reset(pFunc.Get());
  return pFunc;
}

RetainPtr<CPDF_TransferFunc> CPDF_DocRenderData::CreateTransferFunc(
    const CPDF_Object* pObj) const {
  std::unique_ptr<CPDF_Function> pFuncs[3];
  const CPDF_Array* pArray = pObj->AsArray();
  if (pArray) {
    if (pArray->size() < 3)
      return nullptr;

    for (uint32_t i = 0; i < 3; ++i) {
      pFuncs[2 - i] = CPDF_Function::Load(pArray->GetDirectObjectAt(i));
      if (!pFuncs[2 - i])
        return nullptr;
    }
  } else {
    pFuncs[0] = CPDF_Function::Load(pObj);
    if (!pFuncs[0])
      return nullptr;
  }

  int noutput;
  float output[kMaxOutputs];
  memset(output, 0, sizeof(output));

  bool bIdentity = true;
  std::vector<uint8_t> samples_r(CPDF_TransferFunc::kChannelSampleSize);
  std::vector<uint8_t> samples_g(CPDF_TransferFunc::kChannelSampleSize);
  std::vector<uint8_t> samples_b(CPDF_TransferFunc::kChannelSampleSize);
  std::array<pdfium::span<uint8_t>, 3> samples = {samples_r, samples_g,
                                                  samples_b};
  for (size_t v = 0; v < CPDF_TransferFunc::kChannelSampleSize; ++v) {
    float input = static_cast<float>(v) / 255.0f;
    if (pArray) {
      for (int i = 0; i < 3; ++i) {
        if (pFuncs[i]->CountOutputs() > kMaxOutputs) {
          samples[i][v] = v;
          continue;
        }
        pFuncs[i]->Call(&input, 1, output, &noutput);
        size_t o = FXSYS_round(output[0] * 255);
        if (o != v)
          bIdentity = false;
        samples[i][v] = o;
      }
      continue;
    }
    if (pFuncs[0]->CountOutputs() <= kMaxOutputs)
      pFuncs[0]->Call(&input, 1, output, &noutput);
    size_t o = FXSYS_round(output[0] * 255);
    if (o != v)
      bIdentity = false;
    for (auto& channel : samples)
      channel[v] = o;
  }

  return pdfium::MakeRetain<CPDF_TransferFunc>(
      GetDocument(), bIdentity, std::move(samples_r), std::move(samples_g),
      std::move(samples_b));
}
