// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_imagecacheentry.h"

#include <memory>
#include <utility>

#include "core/fpdfapi/page/cpdf_dib.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/render/cpdf_pagerendercache.h"
#include "core/fpdfapi/render/cpdf_rendercontext.h"
#include "core/fpdfapi/render/cpdf_renderstatus.h"
#include "core/fxge/dib/cfx_dibitmap.h"

namespace {

uint32_t GetEstimatedImageSize(const RetainPtr<CFX_DIBBase>& pDIB) {
  if (!pDIB || !pDIB->GetBuffer())
    return 0;

  int height = pDIB->GetHeight();
  ASSERT(pdfium::base::IsValueInRangeForNumericType<uint32_t>(height));
  return static_cast<uint32_t>(height) * pDIB->GetPitch() +
         pDIB->GetPaletteSize() * 4;
}

}  // namespace

CPDF_ImageCacheEntry::CPDF_ImageCacheEntry(CPDF_Document* pDoc,
                                           const RetainPtr<CPDF_Image>& pImage)
    : m_pDocument(pDoc), m_pImage(pImage) {}

CPDF_ImageCacheEntry::~CPDF_ImageCacheEntry() = default;

void CPDF_ImageCacheEntry::Reset() {
  m_pCachedBitmap.Reset();
  CalcSize();
}

RetainPtr<CFX_DIBBase> CPDF_ImageCacheEntry::DetachBitmap() {
  return std::move(m_pCurBitmap);
}

RetainPtr<CFX_DIBBase> CPDF_ImageCacheEntry::DetachMask() {
  return std::move(m_pCurMask);
}

CPDF_DIB::LoadState CPDF_ImageCacheEntry::StartGetCachedBitmap(
    CPDF_Dictionary* pPageResources,
    const CPDF_RenderStatus* pRenderStatus,
    bool bStdCS) {
  if (m_pCachedBitmap) {
    m_pCurBitmap = m_pCachedBitmap;
    m_pCurMask = m_pCachedMask;
    return CPDF_DIB::LoadState::kSuccess;
  }

  m_pCurBitmap = pdfium::MakeRetain<CPDF_DIB>();
  CPDF_DIB::LoadState ret = m_pCurBitmap.As<CPDF_DIB>()->StartLoadDIBBase(
      m_pDocument.Get(), m_pImage->GetStream(), true,
      pRenderStatus->GetFormResource(), pPageResources, bStdCS,
      pRenderStatus->GetGroupFamily(), pRenderStatus->GetLoadMask());
  if (ret == CPDF_DIB::LoadState::kContinue)
    return CPDF_DIB::LoadState::kContinue;

  if (ret == CPDF_DIB::LoadState::kSuccess)
    ContinueGetCachedBitmap(pRenderStatus);
  else
    m_pCurBitmap.Reset();
  return CPDF_DIB::LoadState::kFail;
}

bool CPDF_ImageCacheEntry::Continue(PauseIndicatorIface* pPause,
                                    CPDF_RenderStatus* pRenderStatus) {
  CPDF_DIB::LoadState ret =
      m_pCurBitmap.As<CPDF_DIB>()->ContinueLoadDIBBase(pPause);
  if (ret == CPDF_DIB::LoadState::kContinue)
    return true;

  if (ret == CPDF_DIB::LoadState::kSuccess)
    ContinueGetCachedBitmap(pRenderStatus);
  else
    m_pCurBitmap.Reset();
  return false;
}

void CPDF_ImageCacheEntry::ContinueGetCachedBitmap(
    const CPDF_RenderStatus* pRenderStatus) {
  m_MatteColor = m_pCurBitmap.As<CPDF_DIB>()->GetMatteColor();
  m_pCurMask = m_pCurBitmap.As<CPDF_DIB>()->DetachMask();
  CPDF_RenderContext* pContext = pRenderStatus->GetContext();
  CPDF_PageRenderCache* pPageRenderCache = pContext->GetPageCache();
  m_dwTimeCount = pPageRenderCache->GetTimeCount();
  if (m_pCurBitmap->GetPitch() * m_pCurBitmap->GetHeight() < kHugeImageSize) {
    m_pCachedBitmap = m_pCurBitmap->Clone(nullptr);
    m_pCurBitmap.Reset();
  } else {
    m_pCachedBitmap = m_pCurBitmap;
  }
  if (m_pCurMask) {
    m_pCachedMask = m_pCurMask->Clone(nullptr);
    m_pCurMask.Reset();
  }
  m_pCurBitmap = m_pCachedBitmap;
  m_pCurMask = m_pCachedMask;
  CalcSize();
}

void CPDF_ImageCacheEntry::CalcSize() {
  m_dwCacheSize = GetEstimatedImageSize(m_pCachedBitmap) +
                  GetEstimatedImageSize(m_pCachedMask);
}
