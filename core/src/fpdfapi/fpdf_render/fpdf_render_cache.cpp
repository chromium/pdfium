// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "render_int.h"

#include "core/include/fpdfapi/fpdf_pageobj.h"
#include "core/include/fpdfapi/fpdf_render.h"
#include "core/include/fxge/fx_ge.h"
#include "core/src/fpdfapi/fpdf_page/pageint.h"

struct CACHEINFO {
  FX_DWORD time;
  CPDF_Stream* pStream;
};

extern "C" {
static int compare(const void* data1, const void* data2) {
  return ((CACHEINFO*)data1)->time - ((CACHEINFO*)data2)->time;
}
}  // extern "C"

CPDF_PageRenderCache::~CPDF_PageRenderCache() {
  for (const auto& it : m_ImageCache)
    delete it.second;
}
void CPDF_PageRenderCache::CacheOptimization(int32_t dwLimitCacheSize) {
  if (m_nCacheSize <= (FX_DWORD)dwLimitCacheSize)
    return;

  size_t nCount = m_ImageCache.size();
  CACHEINFO* pCACHEINFO = FX_Alloc(CACHEINFO, nCount);
  size_t i = 0;
  for (const auto& it : m_ImageCache) {
    pCACHEINFO[i].time = it.second->GetTimeCount();
    pCACHEINFO[i++].pStream = it.second->GetStream();
  }
  FXSYS_qsort(pCACHEINFO, nCount, sizeof(CACHEINFO), compare);
  FX_DWORD nTimeCount = m_nTimeCount;

  // Check if time value is about to roll over and reset all entries.
  // The comparision is legal because FX_DWORD is an unsigned type.
  if (nTimeCount + 1 < nTimeCount) {
    for (i = 0; i < nCount; i++)
      m_ImageCache[pCACHEINFO[i].pStream]->m_dwTimeCount = i;
    m_nTimeCount = nCount;
  }

  i = 0;
  while (i + 15 < nCount)
    ClearImageCacheEntry(pCACHEINFO[i++].pStream);

  while (i < nCount && m_nCacheSize > (FX_DWORD)dwLimitCacheSize)
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
FX_DWORD CPDF_PageRenderCache::EstimateSize() {
  FX_DWORD dwSize = 0;
  for (const auto& it : m_ImageCache)
    dwSize += it.second->EstimateSize();

  m_nCacheSize = dwSize;
  return dwSize;
}
void CPDF_PageRenderCache::GetCachedBitmap(CPDF_Stream* pStream,
                                           CFX_DIBSource*& pBitmap,
                                           CFX_DIBSource*& pMask,
                                           FX_DWORD& MatteColor,
                                           FX_BOOL bStdCS,
                                           FX_DWORD GroupFamily,
                                           FX_BOOL bLoadMask,
                                           CPDF_RenderStatus* pRenderStatus,
                                           int32_t downsampleWidth,
                                           int32_t downsampleHeight) {
  CPDF_ImageCacheEntry* pEntry;
  const auto it = m_ImageCache.find(pStream);
  FX_BOOL bFound = it != m_ImageCache.end();
  if (bFound)
    pEntry = it->second;
  else
    pEntry = new CPDF_ImageCacheEntry(m_pPage->m_pDocument, pStream);

  m_nTimeCount++;
  FX_BOOL bAlreadyCached = pEntry->GetCachedBitmap(
      pBitmap, pMask, MatteColor, m_pPage->m_pPageResources, bStdCS,
      GroupFamily, bLoadMask, pRenderStatus, downsampleWidth, downsampleHeight);

  if (!bFound)
    m_ImageCache[pStream] = pEntry;

  if (!bAlreadyCached)
    m_nCacheSize += pEntry->EstimateSize();
}
FX_BOOL CPDF_PageRenderCache::StartGetCachedBitmap(
    CPDF_Stream* pStream,
    FX_BOOL bStdCS,
    FX_DWORD GroupFamily,
    FX_BOOL bLoadMask,
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
    return TRUE;

  m_nTimeCount++;
  if (!m_bCurFindCache)
    m_ImageCache[pStream] = m_pCurImageCacheEntry;

  if (!ret)
    m_nCacheSize += m_pCurImageCacheEntry->EstimateSize();

  return FALSE;
}
FX_BOOL CPDF_PageRenderCache::Continue(IFX_Pause* pPause) {
  int ret = m_pCurImageCacheEntry->Continue(pPause);
  if (ret == 2)
    return TRUE;
  m_nTimeCount++;
  if (!m_bCurFindCache)
    m_ImageCache[m_pCurImageCacheEntry->GetStream()] = m_pCurImageCacheEntry;
  if (!ret)
    m_nCacheSize += m_pCurImageCacheEntry->EstimateSize();
  return FALSE;
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
      m_pCurBitmap(NULL),
      m_pCurMask(NULL),
      m_MatteColor(0),
      m_pRenderStatus(NULL),
      m_pDocument(pDoc),
      m_pStream(pStream),
      m_pCachedBitmap(NULL),
      m_pCachedMask(NULL),
      m_dwCacheSize(0) {}
CPDF_ImageCacheEntry::~CPDF_ImageCacheEntry() {
  delete m_pCachedBitmap;
  delete m_pCachedMask;
}
void CPDF_ImageCacheEntry::Reset(const CFX_DIBitmap* pBitmap) {
  delete m_pCachedBitmap;
  m_pCachedBitmap = NULL;
  if (pBitmap) {
    m_pCachedBitmap = pBitmap->Clone();
  }
  CalcSize();
}
void CPDF_PageRenderCache::ClearImageData() {
  for (const auto& it : m_ImageCache)
    it.second->ClearImageData();
}
void CPDF_ImageCacheEntry::ClearImageData() {
  if (m_pCachedBitmap && !m_pCachedBitmap->GetBuffer()) {
    ((CPDF_DIBSource*)m_pCachedBitmap)->ClearImageData();
  }
}
static FX_DWORD FPDF_ImageCache_EstimateImageSize(const CFX_DIBSource* pDIB) {
  return pDIB && pDIB->GetBuffer()
             ? (FX_DWORD)pDIB->GetHeight() * pDIB->GetPitch() +
                   (FX_DWORD)pDIB->GetPaletteSize() * 4
             : 0;
}
FX_BOOL CPDF_ImageCacheEntry::GetCachedBitmap(CFX_DIBSource*& pBitmap,
                                              CFX_DIBSource*& pMask,
                                              FX_DWORD& MatteColor,
                                              CPDF_Dictionary* pPageResources,
                                              FX_BOOL bStdCS,
                                              FX_DWORD GroupFamily,
                                              FX_BOOL bLoadMask,
                                              CPDF_RenderStatus* pRenderStatus,
                                              int32_t downsampleWidth,
                                              int32_t downsampleHeight) {
  if (m_pCachedBitmap) {
    pBitmap = m_pCachedBitmap;
    pMask = m_pCachedMask;
    MatteColor = m_MatteColor;
    return TRUE;
  }
  if (!pRenderStatus) {
    return FALSE;
  }
  CPDF_RenderContext* pContext = pRenderStatus->GetContext();
  CPDF_PageRenderCache* pPageRenderCache = pContext->GetPageCache();
  m_dwTimeCount = pPageRenderCache->GetTimeCount();
  CPDF_DIBSource* pSrc = new CPDF_DIBSource;
  CPDF_DIBSource* pMaskSrc = NULL;
  if (!pSrc->Load(m_pDocument, m_pStream, &pMaskSrc, &MatteColor,
                  pRenderStatus->m_pFormResource, pPageResources, bStdCS,
                  GroupFamily, bLoadMask)) {
    delete pSrc;
    pBitmap = NULL;
    return FALSE;
  }
  m_MatteColor = MatteColor;
  if (pSrc->GetPitch() * pSrc->GetHeight() < FPDF_HUGE_IMAGE_SIZE) {
    m_pCachedBitmap = pSrc->Clone();
    delete pSrc;
  } else {
    m_pCachedBitmap = pSrc;
  }
  if (pMaskSrc) {
    m_pCachedMask = pMaskSrc->Clone();
    delete pMaskSrc;
  }

  pBitmap = m_pCachedBitmap;
  pMask = m_pCachedMask;
  CalcSize();
  return FALSE;
}
CFX_DIBSource* CPDF_ImageCacheEntry::DetachBitmap() {
  CFX_DIBSource* pDIBSource = m_pCurBitmap;
  m_pCurBitmap = NULL;
  return pDIBSource;
}
CFX_DIBSource* CPDF_ImageCacheEntry::DetachMask() {
  CFX_DIBSource* pDIBSource = m_pCurMask;
  m_pCurMask = NULL;
  return pDIBSource;
}
int CPDF_ImageCacheEntry::StartGetCachedBitmap(CPDF_Dictionary* pFormResources,
                                               CPDF_Dictionary* pPageResources,
                                               FX_BOOL bStdCS,
                                               FX_DWORD GroupFamily,
                                               FX_BOOL bLoadMask,
                                               CPDF_RenderStatus* pRenderStatus,
                                               int32_t downsampleWidth,
                                               int32_t downsampleHeight) {
  if (m_pCachedBitmap) {
    m_pCurBitmap = m_pCachedBitmap;
    m_pCurMask = m_pCachedMask;
    return 1;
  }
  if (!pRenderStatus) {
    return 0;
  }
  m_pRenderStatus = pRenderStatus;
  m_pCurBitmap = new CPDF_DIBSource;
  int ret =
      ((CPDF_DIBSource*)m_pCurBitmap)
          ->StartLoadDIBSource(m_pDocument, m_pStream, TRUE, pFormResources,
                               pPageResources, bStdCS, GroupFamily, bLoadMask);
  if (ret == 2) {
    return ret;
  }
  if (!ret) {
    delete m_pCurBitmap;
    m_pCurBitmap = NULL;
    return 0;
  }
  ContinueGetCachedBitmap();
  return 0;
}
void CPDF_ImageCacheEntry::ContinueGetCachedBitmap() {
  m_MatteColor = ((CPDF_DIBSource*)m_pCurBitmap)->m_MatteColor;
  m_pCurMask = ((CPDF_DIBSource*)m_pCurBitmap)->DetachMask();
  CPDF_RenderContext* pContext = m_pRenderStatus->GetContext();
  CPDF_PageRenderCache* pPageRenderCache = pContext->GetPageCache();
  m_dwTimeCount = pPageRenderCache->GetTimeCount();
  if (m_pCurBitmap->GetPitch() * m_pCurBitmap->GetHeight() <
      FPDF_HUGE_IMAGE_SIZE) {
    m_pCachedBitmap = m_pCurBitmap->Clone();
    delete m_pCurBitmap;
    m_pCurBitmap = NULL;
  } else {
    m_pCachedBitmap = m_pCurBitmap;
  }
  if (m_pCurMask) {
    m_pCachedMask = m_pCurMask->Clone();
    delete m_pCurMask;
    m_pCurMask = NULL;
  }
  m_pCurBitmap = m_pCachedBitmap;
  m_pCurMask = m_pCachedMask;
  CalcSize();
}
int CPDF_ImageCacheEntry::Continue(IFX_Pause* pPause) {
  int ret = ((CPDF_DIBSource*)m_pCurBitmap)->ContinueLoadDIBSource(pPause);
  if (ret == 2) {
    return ret;
  }
  if (!ret) {
    delete m_pCurBitmap;
    m_pCurBitmap = NULL;
    return 0;
  }
  ContinueGetCachedBitmap();
  return 0;
}
void CPDF_ImageCacheEntry::CalcSize() {
  m_dwCacheSize = FPDF_ImageCache_EstimateImageSize(m_pCachedBitmap) +
                  FPDF_ImageCache_EstimateImageSize(m_pCachedMask);
}
void CPDF_Document::ClearRenderFont() {
  if (m_pDocRender) {
    CFX_FontCache* pCache = m_pDocRender->GetFontCache();
    if (pCache) {
      pCache->FreeCache(FALSE);
    }
  }
}
