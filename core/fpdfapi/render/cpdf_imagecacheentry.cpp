// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_imagecacheentry.h"

#include <memory>
#include <utility>

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/render/cpdf_dibsource.h"
#include "core/fpdfapi/render/cpdf_pagerendercache.h"
#include "core/fpdfapi/render/cpdf_rendercontext.h"
#include "core/fpdfapi/render/cpdf_renderstatus.h"

CPDF_ImageCacheEntry::CPDF_ImageCacheEntry(CPDF_Document* pDoc,
                                           CPDF_Stream* pStream)
    : m_dwTimeCount(0),
      m_MatteColor(0),
      m_pRenderStatus(nullptr),
      m_pDocument(pDoc),
      m_pStream(pStream),
      m_pCurBitmap(nullptr),
      m_pCurMask(nullptr),
      m_dwCacheSize(0) {}

CPDF_ImageCacheEntry::~CPDF_ImageCacheEntry() {}

void CPDF_ImageCacheEntry::Reset(const CFX_RetainPtr<CFX_DIBitmap>& pBitmap) {
  m_pCachedBitmap.Reset();
  if (pBitmap)
    m_pCachedBitmap = pBitmap->Clone();
  CalcSize();
}

static uint32_t FPDF_ImageCache_EstimateImageSize(
    const CFX_RetainPtr<CFX_DIBSource>& pDIB) {
  return pDIB && pDIB->GetBuffer()
             ? (uint32_t)pDIB->GetHeight() * pDIB->GetPitch() +
                   (uint32_t)pDIB->GetPaletteSize() * 4
             : 0;
}

CFX_RetainPtr<CFX_DIBSource> CPDF_ImageCacheEntry::DetachBitmap() {
  return std::move(m_pCurBitmap);
}

CFX_RetainPtr<CFX_DIBSource> CPDF_ImageCacheEntry::DetachMask() {
  return std::move(m_pCurMask);
}

int CPDF_ImageCacheEntry::StartGetCachedBitmap(CPDF_Dictionary* pFormResources,
                                               CPDF_Dictionary* pPageResources,
                                               bool bStdCS,
                                               uint32_t GroupFamily,
                                               bool bLoadMask,
                                               CPDF_RenderStatus* pRenderStatus,
                                               int32_t downsampleWidth,
                                               int32_t downsampleHeight) {
  if (m_pCachedBitmap) {
    m_pCurBitmap = m_pCachedBitmap;
    m_pCurMask = m_pCachedMask;
    return 1;
  }
  if (!pRenderStatus)
    return 0;

  m_pRenderStatus = pRenderStatus;
  m_pCurBitmap = pdfium::MakeRetain<CPDF_DIBSource>();
  int ret = m_pCurBitmap.As<CPDF_DIBSource>()->StartLoadDIBSource(
      m_pDocument, m_pStream, true, pFormResources, pPageResources, bStdCS,
      GroupFamily, bLoadMask);
  if (ret == 2)
    return ret;

  if (!ret) {
    m_pCurBitmap.Reset();
    return 0;
  }
  ContinueGetCachedBitmap();
  return 0;
}

void CPDF_ImageCacheEntry::ContinueGetCachedBitmap() {
  m_MatteColor = m_pCurBitmap.As<CPDF_DIBSource>()->GetMatteColor();
  m_pCurMask = m_pCurBitmap.As<CPDF_DIBSource>()->DetachMask();
  CPDF_RenderContext* pContext = m_pRenderStatus->GetContext();
  CPDF_PageRenderCache* pPageRenderCache = pContext->GetPageCache();
  m_dwTimeCount = pPageRenderCache->GetTimeCount();
  if (m_pCurBitmap->GetPitch() * m_pCurBitmap->GetHeight() <
      FPDF_HUGE_IMAGE_SIZE) {
    m_pCachedBitmap = m_pCurBitmap->Clone();
    m_pCurBitmap.Reset();
  } else {
    m_pCachedBitmap = m_pCurBitmap;
  }
  if (m_pCurMask) {
    m_pCachedMask = m_pCurMask->Clone();
    m_pCurMask.Reset();
  }
  m_pCurBitmap = m_pCachedBitmap;
  m_pCurMask = m_pCachedMask;
  CalcSize();
}

int CPDF_ImageCacheEntry::Continue(IFX_Pause* pPause) {
  int ret = m_pCurBitmap.As<CPDF_DIBSource>()->ContinueLoadDIBSource(pPause);
  if (!ret) {
    m_pCurBitmap.Reset();
    return 0;
  }
  if (ret == 2)
    return ret;

  ContinueGetCachedBitmap();
  return 0;
}

void CPDF_ImageCacheEntry::CalcSize() {
  m_dwCacheSize = FPDF_ImageCache_EstimateImageSize(m_pCachedBitmap) +
                  FPDF_ImageCache_EstimateImageSize(m_pCachedMask);
}
