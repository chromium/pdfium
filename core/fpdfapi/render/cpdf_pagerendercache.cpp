// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_pagerendercache.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "core/fpdfapi/page/cpdf_dib.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/render/cpdf_rendercontext.h"
#include "core/fpdfapi/render/cpdf_renderstatus.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/dib/cfx_dibitmap.h"

namespace {

struct CacheInfo {
  CacheInfo(uint32_t t, const CPDF_Stream* stream) : time(t), pStream(stream) {}

  uint32_t time;
  const CPDF_Stream* pStream;

  bool operator<(const CacheInfo& other) const { return time < other.time; }
};

}  // namespace

CPDF_PageRenderCache::CPDF_PageRenderCache(CPDF_Page* pPage) : m_pPage(pPage) {}

CPDF_PageRenderCache::~CPDF_PageRenderCache() = default;

void CPDF_PageRenderCache::CacheOptimization(int32_t dwLimitCacheSize) {
  if (m_nCacheSize <= (uint32_t)dwLimitCacheSize)
    return;

  uint32_t nCount = fxcrt::CollectionSize<uint32_t>(m_ImageCache);
  std::vector<CacheInfo> cache_info;
  cache_info.reserve(nCount);
  for (const auto& it : m_ImageCache) {
    cache_info.emplace_back(it.second->GetTimeCount(),
                            it.second->GetImage()->GetStream());
  }
  std::sort(cache_info.begin(), cache_info.end());

  // Check if time value is about to roll over and reset all entries.
  // The comparison is legal because uint32_t is an unsigned type.
  uint32_t nTimeCount = m_nTimeCount;
  if (nTimeCount + 1 < nTimeCount) {
    for (uint32_t i = 0; i < nCount; i++)
      m_ImageCache[cache_info[i].pStream]->SetTimeCount(i);
    m_nTimeCount = nCount;
  }

  size_t i = 0;
  while (i + 15 < nCount)
    ClearImageCacheEntry(cache_info[i++].pStream);

  while (i < nCount && m_nCacheSize > (uint32_t)dwLimitCacheSize)
    ClearImageCacheEntry(cache_info[i++].pStream);
}

void CPDF_PageRenderCache::ClearImageCacheEntry(const CPDF_Stream* pStream) {
  auto it = m_ImageCache.find(pStream);
  if (it == m_ImageCache.end())
    return;

  m_nCacheSize -= it->second->EstimateSize();
  m_ImageCache.erase(it);
}

bool CPDF_PageRenderCache::StartGetCachedBitmap(
    RetainPtr<CPDF_Image> pImage,
    const CPDF_RenderStatus* pRenderStatus,
    bool bStdCS) {
  const CPDF_Stream* pStream = pImage->GetStream();
  const auto it = m_ImageCache.find(pStream);
  m_bCurFindCache = it != m_ImageCache.end();
  if (m_bCurFindCache) {
    m_pCurImageCacheEntry = it->second.get();
  } else {
    m_pCurImageCacheEntry = std::make_unique<ImageCacheEntry>(
        m_pPage->GetDocument(), std::move(pImage));
  }
  CPDF_DIB::LoadState ret = m_pCurImageCacheEntry->StartGetCachedBitmap(
      m_pPage->GetPageResources(), pRenderStatus, bStdCS);
  if (ret == CPDF_DIB::LoadState::kContinue)
    return true;

  m_nTimeCount++;
  if (!m_bCurFindCache)
    m_ImageCache[pStream] = m_pCurImageCacheEntry.Release();

  if (ret == CPDF_DIB::LoadState::kFail)
    m_nCacheSize += m_pCurImageCacheEntry->EstimateSize();

  return false;
}

bool CPDF_PageRenderCache::Continue(PauseIndicatorIface* pPause,
                                    CPDF_RenderStatus* pRenderStatus) {
  bool ret = m_pCurImageCacheEntry->Continue(pPause, pRenderStatus);
  if (ret)
    return true;

  m_nTimeCount++;
  if (!m_bCurFindCache) {
    m_ImageCache[m_pCurImageCacheEntry->GetImage()->GetStream()] =
        m_pCurImageCacheEntry.Release();
  }
  m_nCacheSize += m_pCurImageCacheEntry->EstimateSize();
  return false;
}

void CPDF_PageRenderCache::ResetBitmapForImage(RetainPtr<CPDF_Image> pImage) {
  const CPDF_Stream* pStream = pImage->GetStream();
  const auto it = m_ImageCache.find(pStream);
  if (it == m_ImageCache.end())
    return;

  ImageCacheEntry* pEntry = it->second.get();
  m_nCacheSize -= pEntry->EstimateSize();
  pEntry->Reset();
  m_nCacheSize += pEntry->EstimateSize();
}

uint32_t CPDF_PageRenderCache::GetCurMatteColor() const {
  return m_pCurImageCacheEntry->GetMatteColor();
}

RetainPtr<CFX_DIBBase> CPDF_PageRenderCache::DetachCurBitmap() {
  return m_pCurImageCacheEntry->DetachBitmap();
}

RetainPtr<CFX_DIBBase> CPDF_PageRenderCache::DetachCurMask() {
  return m_pCurImageCacheEntry->DetachMask();
}

CPDF_PageRenderCache::ImageCacheEntry::ImageCacheEntry(
    CPDF_Document* pDoc,
    RetainPtr<CPDF_Image> pImage)
    : m_pDocument(pDoc), m_pImage(std::move(pImage)) {}

CPDF_PageRenderCache::ImageCacheEntry::~ImageCacheEntry() = default;

void CPDF_PageRenderCache::ImageCacheEntry::Reset() {
  m_pCachedBitmap.Reset();
  CalcSize();
}

RetainPtr<CFX_DIBBase> CPDF_PageRenderCache::ImageCacheEntry::DetachBitmap() {
  return std::move(m_pCurBitmap);
}

RetainPtr<CFX_DIBBase> CPDF_PageRenderCache::ImageCacheEntry::DetachMask() {
  return std::move(m_pCurMask);
}

CPDF_DIB::LoadState CPDF_PageRenderCache::ImageCacheEntry::StartGetCachedBitmap(
    const CPDF_Dictionary* pPageResources,
    const CPDF_RenderStatus* pRenderStatus,
    bool bStdCS) {
  if (m_pCachedBitmap) {
    m_pCurBitmap = m_pCachedBitmap;
    m_pCurMask = m_pCachedMask;
    return CPDF_DIB::LoadState::kSuccess;
  }

  // A cross-document image may have come from the embedder.
  if (m_pDocument != m_pImage->GetDocument())
    return CPDF_DIB::LoadState::kFail;

  m_pCurBitmap = m_pImage->CreateNewDIB();
  CPDF_DIB::LoadState ret = m_pCurBitmap.As<CPDF_DIB>()->StartLoadDIBBase(
      true, pRenderStatus->GetFormResource(), pPageResources, bStdCS,
      pRenderStatus->GetGroupFamily(), pRenderStatus->GetLoadMask());
  if (ret == CPDF_DIB::LoadState::kContinue)
    return CPDF_DIB::LoadState::kContinue;

  if (ret == CPDF_DIB::LoadState::kSuccess)
    ContinueGetCachedBitmap(pRenderStatus);
  else
    m_pCurBitmap.Reset();
  return CPDF_DIB::LoadState::kFail;
}

bool CPDF_PageRenderCache::ImageCacheEntry::Continue(
    PauseIndicatorIface* pPause,
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

void CPDF_PageRenderCache::ImageCacheEntry::ContinueGetCachedBitmap(
    const CPDF_RenderStatus* pRenderStatus) {
  m_MatteColor = m_pCurBitmap.As<CPDF_DIB>()->GetMatteColor();
  m_pCurMask = m_pCurBitmap.As<CPDF_DIB>()->DetachMask();
  CPDF_RenderContext* pContext = pRenderStatus->GetContext();
  CPDF_PageRenderCache* pPageRenderCache = pContext->GetPageCache();
  m_dwTimeCount = pPageRenderCache->GetTimeCount();
  if (m_pCurBitmap->GetPitch() * m_pCurBitmap->GetHeight() < kHugeImageSize) {
    m_pCachedBitmap = m_pCurBitmap->Realize();
    m_pCurBitmap.Reset();
  } else {
    m_pCachedBitmap = m_pCurBitmap;
  }
  if (m_pCurMask) {
    m_pCachedMask = m_pCurMask->Realize();
    m_pCurMask.Reset();
  }
  m_pCurBitmap = m_pCachedBitmap;
  m_pCurMask = m_pCachedMask;
  CalcSize();
}

void CPDF_PageRenderCache::ImageCacheEntry::CalcSize() {
  m_dwCacheSize = 0;
  if (m_pCachedBitmap)
    m_dwCacheSize += m_pCachedBitmap->GetEstimatedImageMemoryBurden();
  if (m_pCachedMask)
    m_dwCacheSize += m_pCachedMask->GetEstimatedImageMemoryBurden();
}
