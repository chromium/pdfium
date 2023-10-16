// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_docrenderdata.h"

#include <stdint.h>

#include <algorithm>
#include <array>
#include <memory>
#include <utility>

#include "core/fpdfapi/font/cpdf_type3font.h"
#include "core/fpdfapi/page/cpdf_dib.h"
#include "core/fpdfapi/page/cpdf_function.h"
#include "core/fpdfapi/page/cpdf_transferfunc.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/render/cpdf_type3cache.h"
#include "core/fxcrt/fixed_uninit_data_vector.h"

#if BUILDFLAG(IS_WIN)
#include "core/fxge/win32/cfx_psfonttracker.h"
#endif

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
    RetainPtr<const CPDF_Object> pObj) {
  if (!pObj)
    return nullptr;

  auto it = m_TransferFuncMap.find(pObj);
  if (it != m_TransferFuncMap.end() && it->second)
    return pdfium::WrapRetain(it->second.Get());

  auto pFunc = CreateTransferFunc(pObj);
  m_TransferFuncMap[pObj].Reset(pFunc.Get());
  return pFunc;
}

#if BUILDFLAG(IS_WIN)
CFX_PSFontTracker* CPDF_DocRenderData::GetPSFontTracker() {
  if (!m_PSFontTracker)
    m_PSFontTracker = std::make_unique<CFX_PSFontTracker>();
  return m_PSFontTracker.get();
}
#endif

RetainPtr<CPDF_TransferFunc> CPDF_DocRenderData::CreateTransferFunc(
    RetainPtr<const CPDF_Object> pObj) const {
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

  float output[kMaxOutputs];
  std::fill(std::begin(output), std::end(output), 0.0f);

  bool bIdentity = true;
  FixedUninitDataVector<uint8_t> samples_r(
      CPDF_TransferFunc::kChannelSampleSize);
  FixedUninitDataVector<uint8_t> samples_g(
      CPDF_TransferFunc::kChannelSampleSize);
  FixedUninitDataVector<uint8_t> samples_b(
      CPDF_TransferFunc::kChannelSampleSize);
  std::array<pdfium::span<uint8_t>, 3> samples = {samples_r.writable_span(),
                                                  samples_g.writable_span(),
                                                  samples_b.writable_span()};
  if (pArray) {
    for (size_t v = 0; v < CPDF_TransferFunc::kChannelSampleSize; ++v) {
      float input = static_cast<float>(v) / 255.0f;
      for (int i = 0; i < 3; ++i) {
        if (pFuncs[i]->CountOutputs() > kMaxOutputs) {
          samples[i][v] = v;
          continue;
        }
        pFuncs[i]->Call(pdfium::make_span(&input, 1u), output);
        size_t o = FXSYS_roundf(output[0] * 255);
        if (o != v)
          bIdentity = false;
        samples[i][v] = o;
      }
    }
  } else {
    for (size_t v = 0; v < CPDF_TransferFunc::kChannelSampleSize; ++v) {
      float input = static_cast<float>(v) / 255.0f;
      if (pFuncs[0]->CountOutputs() <= kMaxOutputs)
        pFuncs[0]->Call(pdfium::make_span(&input, 1u), output);
      size_t o = FXSYS_roundf(output[0] * 255);
      if (o != v)
        bIdentity = false;
      for (auto& channel : samples)
        channel[v] = o;
    }
  }

  return pdfium::MakeRetain<CPDF_TransferFunc>(bIdentity, std::move(samples_r),
                                               std::move(samples_g),
                                               std::move(samples_b));
}
