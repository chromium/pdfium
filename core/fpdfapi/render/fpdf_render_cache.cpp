// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <memory>
#include <utility>

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/pageint.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/render/cpdf_pagerendercache.h"
#include "core/fpdfapi/render/cpdf_rendercontext.h"
#include "core/fpdfapi/render/cpdf_renderstatus.h"
#include "core/fpdfapi/render/render_int.h"

struct CACHEINFO {
  uint32_t time;
  CPDF_Stream* pStream;
};

extern "C" {
static int compare(const void* data1, const void* data2) {
  return ((CACHEINFO*)data1)->time - ((CACHEINFO*)data2)->time;
}
}  // extern "C"

CPDF_PageRenderCache::CPDF_PageRenderCache(CPDF_Page* pPage)
    : m_pPage(pPage),
      m_pCurImageCacheEntry(nullptr),
      m_nTimeCount(0),
      m_nCacheSize(0),
      m_bCurFindCache(false) {}

CPDF_PageRenderCache::~CPDF_PageRenderCache() {
  for (const auto& it : m_ImageCache)
    delete it.second;
}
void CPDF_PageRenderCache::CacheOptimization(int32_t dwLimitCacheSize) {
  if (m_nCacheSize <= (uint32_t)dwLimitCacheSize)
    return;

  size_t nCount = m_ImageCache.size();
  CACHEINFO* pCACHEINFO = FX_Alloc(CACHEINFO, nCount);
  size_t i = 0;
  for (const auto& it : m_ImageCache) {
    pCACHEINFO[i].time = it.second->GetTimeCount();
    pCACHEINFO[i++].pStream = it.second->GetStream();
  }
  FXSYS_qsort(pCACHEINFO, nCount, sizeof(CACHEINFO), compare);
  uint32_t nTimeCount = m_nTimeCount;

  // Check if time value is about to roll over and reset all entries.
  // The comparision is legal because uint32_t is an unsigned type.
  if (nTimeCount + 1 < nTimeCount) {
    for (i = 0; i < nCount; i++)
      m_ImageCache[pCACHEINFO[i].pStream]->m_dwTimeCount = i;
    m_nTimeCount = nCount;
  }

  i = 0;
  while (i + 15 < nCount)
    ClearImageCacheEntry(pCACHEINFO[i++].pStream);

  while (i < nCount && m_nCacheSize > (uint32_t)dwLimitCacheSize)
    ClearImageCacheEntry(pCACHEINFO[i++].pStream);

  FX_Free(pCACHEINFO);
}
void CPDF_PageRenderCache::ClearImageCacheEntry(CPDF_Stream* pStream) {
  auto it = m_ImageCache.find(pStream);
  if (it == m_ImageCache.end())
    return;

  m_nCacheSize -= it->second->EstimateSize();
  delete it->second;
  m_ImageCache.erase(it);
}
uint32_t CPDF_PageRenderCache::EstimateSize() {
  uint32_t dwSize = 0;
  for (const auto& it : m_ImageCache)
    dwSize += it.second->EstimateSize();

  m_nCacheSize = dwSize;
  return dwSize;
}
void CPDF_PageRenderCache::GetCachedBitmap(CPDF_Stream* pStream,
                                           CFX_DIBSource*& pBitmap,
                                           CFX_DIBSource*& pMask,
                                           uint32_t& MatteColor,
                                           bool bStdCS,
                                           uint32_t GroupFamily,
                                           bool bLoadMask,
                                           CPDF_RenderStatus* pRenderStatus,
                                           int32_t downsampleWidth,
                                           int32_t downsampleHeight) {
  CPDF_ImageCacheEntry* pEntry;
  const auto it = m_ImageCache.find(pStream);
  bool bFound = it != m_ImageCache.end();
  if (bFound)
    pEntry = it->second;
  else
    pEntry = new CPDF_ImageCacheEntry(m_pPage->m_pDocument, pStream);

  m_nTimeCount++;
  bool bAlreadyCached = pEntry->GetCachedBitmap(
      pBitmap, pMask, MatteColor, m_pPage->m_pPageResources, bStdCS,
      GroupFamily, bLoadMask, pRenderStatus, downsampleWidth, downsampleHeight);

  if (!bFound)
    m_ImageCache[pStream] = pEntry;

  if (!bAlreadyCached)
    m_nCacheSize += pEntry->EstimateSize();
}
bool CPDF_PageRenderCache::StartGetCachedBitmap(
    CPDF_Stream* pStream,
    bool bStdCS,
    uint32_t GroupFamily,
    bool bLoadMask,
    CPDF_RenderStatus* pRenderStatus,
    int32_t downsampleWidth,
    int32_t downsampleHeight) {
  const auto it = m_ImageCache.find(pStream);
  m_bCurFindCache = it != m_ImageCache.end();
  if (m_bCurFindCache) {
    m_pCurImageCacheEntry = it->second;
  } else {
    m_pCurImageCacheEntry =
        new CPDF_ImageCacheEntry(m_pPage->m_pDocument, pStream);
  }
  int ret = m_pCurImageCacheEntry->StartGetCachedBitmap(
      pRenderStatus->m_pFormResource, m_pPage->m_pPageResources, bStdCS,
      GroupFamily, bLoadMask, pRenderStatus, downsampleWidth, downsampleHeight);
  if (ret == 2)
    return true;

  m_nTimeCount++;
  if (!m_bCurFindCache)
    m_ImageCache[pStream] = m_pCurImageCacheEntry;

  if (!ret)
    m_nCacheSize += m_pCurImageCacheEntry->EstimateSize();

  return false;
}
bool CPDF_PageRenderCache::Continue(IFX_Pause* pPause) {
  int ret = m_pCurImageCacheEntry->Continue(pPause);
  if (ret == 2)
    return true;
  m_nTimeCount++;
  if (!m_bCurFindCache)
    m_ImageCache[m_pCurImageCacheEntry->GetStream()] = m_pCurImageCacheEntry;
  if (!ret)
    m_nCacheSize += m_pCurImageCacheEntry->EstimateSize();
  return false;
}
void CPDF_PageRenderCache::ResetBitmap(CPDF_Stream* pStream,
                                       const CFX_DIBitmap* pBitmap) {
  CPDF_ImageCacheEntry* pEntry;
  const auto it = m_ImageCache.find(pStream);
  if (it == m_ImageCache.end()) {
    if (!pBitmap)
      return;
    pEntry = new CPDF_ImageCacheEntry(m_pPage->m_pDocument, pStream);
    m_ImageCache[pStream] = pEntry;
  } else {
    pEntry = it->second;
  }
  m_nCacheSize -= pEntry->EstimateSize();
  pEntry->Reset(pBitmap);
  m_nCacheSize += pEntry->EstimateSize();
}
CPDF_ImageCacheEntry::CPDF_ImageCacheEntry(CPDF_Document* pDoc,
                                           CPDF_Stream* pStream)
    : m_dwTimeCount(0),
      m_pCurBitmap(nullptr),
      m_pCurMask(nullptr),
      m_MatteColor(0),
      m_pRenderStatus(nullptr),
      m_pDocument(pDoc),
      m_pStream(pStream),
      m_dwCacheSize(0) {}

CPDF_ImageCacheEntry::~CPDF_ImageCacheEntry() {}

void CPDF_ImageCacheEntry::Reset(const CFX_DIBitmap* pBitmap) {
  m_pCachedBitmap.reset();
  if (pBitmap)
    m_pCachedBitmap = pdfium::WrapUnique<CFX_DIBSource>(pBitmap->Clone());
  CalcSize();
}

static uint32_t FPDF_ImageCache_EstimateImageSize(const CFX_DIBSource* pDIB) {
  return pDIB && pDIB->GetBuffer()
             ? (uint32_t)pDIB->GetHeight() * pDIB->GetPitch() +
                   (uint32_t)pDIB->GetPaletteSize() * 4
             : 0;
}
bool CPDF_ImageCacheEntry::GetCachedBitmap(CFX_DIBSource*& pBitmap,
                                           CFX_DIBSource*& pMask,
                                           uint32_t& MatteColor,
                                           CPDF_Dictionary* pPageResources,
                                           bool bStdCS,
                                           uint32_t GroupFamily,
                                           bool bLoadMask,
                                           CPDF_RenderStatus* pRenderStatus,
                                           int32_t downsampleWidth,
                                           int32_t downsampleHeight) {
  if (m_pCachedBitmap) {
    pBitmap = m_pCachedBitmap.get();
    pMask = m_pCachedMask.get();
    MatteColor = m_MatteColor;
    return true;
  }
  if (!pRenderStatus) {
    return false;
  }
  CPDF_RenderContext* pContext = pRenderStatus->GetContext();
  CPDF_PageRenderCache* pPageRenderCache = pContext->GetPageCache();
  m_dwTimeCount = pPageRenderCache->GetTimeCount();
  std::unique_ptr<CPDF_DIBSource> pSrc = pdfium::MakeUnique<CPDF_DIBSource>();
  CPDF_DIBSource* pMaskSrc = nullptr;
  if (!pSrc->Load(m_pDocument, m_pStream, &pMaskSrc, &MatteColor,
                  pRenderStatus->m_pFormResource, pPageResources, bStdCS,
                  GroupFamily, bLoadMask)) {
    pBitmap = nullptr;
    return false;
  }
  m_MatteColor = MatteColor;
  m_pCachedBitmap = std::move(pSrc);
  if (pMaskSrc)
    m_pCachedMask = pdfium::WrapUnique<CFX_DIBSource>(pMaskSrc);

  pBitmap = m_pCachedBitmap.get();
  pMask = m_pCachedMask.get();
  CalcSize();
  return false;
}

CFX_DIBSource* CPDF_ImageCacheEntry::DetachBitmap() {
  CFX_DIBSource* pDIBSource = m_pCurBitmap;
  m_pCurBitmap = nullptr;
  return pDIBSource;
}
CFX_DIBSource* CPDF_ImageCacheEntry::DetachMask() {
  CFX_DIBSource* pDIBSource = m_pCurMask;
  m_pCurMask = nullptr;
  return pDIBSource;
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
    m_pCurBitmap = m_pCachedBitmap.get();
    m_pCurMask = m_pCachedMask.get();
    return 1;
  }
  if (!pRenderStatus)
    return 0;

  m_pRenderStatus = pRenderStatus;
  m_pCurBitmap = new CPDF_DIBSource;
  int ret =
      ((CPDF_DIBSource*)m_pCurBitmap)
          ->StartLoadDIBSource(m_pDocument, m_pStream, true, pFormResources,
                               pPageResources, bStdCS, GroupFamily, bLoadMask);
  if (ret == 2)
    return ret;

  if (!ret) {
    delete m_pCurBitmap;
    m_pCurBitmap = nullptr;
    return 0;
  }
  ContinueGetCachedBitmap();
  return 0;
}

void CPDF_ImageCacheEntry::ContinueGetCachedBitmap() {
  m_MatteColor = ((CPDF_DIBSource*)m_pCurBitmap)->GetMatteColor();
  m_pCurMask = ((CPDF_DIBSource*)m_pCurBitmap)->DetachMask();
  CPDF_RenderContext* pContext = m_pRenderStatus->GetContext();
  CPDF_PageRenderCache* pPageRenderCache = pContext->GetPageCache();
  m_dwTimeCount = pPageRenderCache->GetTimeCount();
  m_pCachedBitmap = pdfium::WrapUnique<CFX_DIBSource>(m_pCurBitmap);
  if (m_pCurMask)
    m_pCachedMask = pdfium::WrapUnique<CFX_DIBSource>(m_pCurMask);
  else
    m_pCurMask = m_pCachedMask.get();
  CalcSize();
}

int CPDF_ImageCacheEntry::Continue(IFX_Pause* pPause) {
  int ret = ((CPDF_DIBSource*)m_pCurBitmap)->ContinueLoadDIBSource(pPause);
  if (ret == 2) {
    return ret;
  }
  if (!ret) {
    delete m_pCurBitmap;
    m_pCurBitmap = nullptr;
    return 0;
  }
  ContinueGetCachedBitmap();
  return 0;
}
void CPDF_ImageCacheEntry::CalcSize() {
  m_dwCacheSize = FPDF_ImageCache_EstimateImageSize(m_pCachedBitmap.get()) +
                  FPDF_ImageCache_EstimateImageSize(m_pCachedMask.get());
}
