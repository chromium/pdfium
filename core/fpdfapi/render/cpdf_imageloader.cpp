// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_imageloader.h"

#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/render/cpdf_pagerendercache.h"
#include "core/fpdfapi/render/cpdf_renderstatus.h"
#include "core/fpdfapi/render/render_int.h"
#include "core/fxcrt/fx_basic.h"

class CPDF_ImageLoaderHandle {
 public:
  CPDF_ImageLoaderHandle();
  ~CPDF_ImageLoaderHandle();

  bool Start(CPDF_ImageLoader* pImageLoader,
             const CPDF_ImageObject* pImage,
             CPDF_PageRenderCache* pCache,
             bool bStdCS,
             uint32_t GroupFamily,
             bool bLoadMask,
             CPDF_RenderStatus* pRenderStatus,
             int32_t nDownsampleWidth,
             int32_t nDownsampleHeight);
  bool Continue(IFX_Pause* pPause);

 private:
  void HandleFailure();

  CPDF_ImageLoader* m_pImageLoader;
  CPDF_PageRenderCache* m_pCache;
  CPDF_ImageObject* m_pImage;
  int32_t m_nDownsampleWidth;
  int32_t m_nDownsampleHeight;
};

CPDF_ImageLoaderHandle::CPDF_ImageLoaderHandle() {
  m_pImageLoader = nullptr;
  m_pCache = nullptr;
  m_pImage = nullptr;
}

CPDF_ImageLoaderHandle::~CPDF_ImageLoaderHandle() {}

bool CPDF_ImageLoaderHandle::Start(CPDF_ImageLoader* pImageLoader,
                                   const CPDF_ImageObject* pImage,
                                   CPDF_PageRenderCache* pCache,
                                   bool bStdCS,
                                   uint32_t GroupFamily,
                                   bool bLoadMask,
                                   CPDF_RenderStatus* pRenderStatus,
                                   int32_t nDownsampleWidth,
                                   int32_t nDownsampleHeight) {
  m_pImageLoader = pImageLoader;
  m_pCache = pCache;
  m_pImage = const_cast<CPDF_ImageObject*>(pImage);
  m_nDownsampleWidth = nDownsampleWidth;
  m_nDownsampleHeight = nDownsampleHeight;
  bool ret;
  if (pCache) {
    ret = pCache->StartGetCachedBitmap(
        m_pImage->GetImage()->GetStream(), bStdCS, GroupFamily, bLoadMask,
        pRenderStatus, m_nDownsampleWidth, m_nDownsampleHeight);
  } else {
    ret = m_pImage->GetImage()->StartLoadDIBSource(
        pRenderStatus->m_pFormResource, pRenderStatus->m_pPageResource, bStdCS,
        GroupFamily, bLoadMask);
  }
  if (!ret)
    HandleFailure();
  return ret;
}

bool CPDF_ImageLoaderHandle::Continue(IFX_Pause* pPause) {
  bool ret = m_pCache ? m_pCache->Continue(pPause)
                      : m_pImage->GetImage()->Continue(pPause);
  if (!ret)
    HandleFailure();
  return ret;
}

void CPDF_ImageLoaderHandle::HandleFailure() {
  if (m_pCache) {
    CPDF_ImageCacheEntry* entry = m_pCache->GetCurImageCacheEntry();
    m_pImageLoader->m_bCached = true;
    m_pImageLoader->m_pBitmap = entry->DetachBitmap();
    m_pImageLoader->m_pMask = entry->DetachMask();
    m_pImageLoader->m_MatteColor = entry->m_MatteColor;
    return;
  }
  CPDF_Image* pImage = m_pImage->GetImage();
  m_pImageLoader->m_bCached = false;
  m_pImageLoader->m_pBitmap = pImage->DetachBitmap();
  m_pImageLoader->m_pMask = pImage->DetachMask();
  m_pImageLoader->m_MatteColor = pImage->m_MatteColor;
}

CPDF_ImageLoader::CPDF_ImageLoader()
    : m_pBitmap(nullptr),
      m_pMask(nullptr),
      m_MatteColor(0),
      m_bCached(false),
      m_nDownsampleWidth(0),
      m_nDownsampleHeight(0) {}

CPDF_ImageLoader::~CPDF_ImageLoader() {
  if (!m_bCached) {
    delete m_pBitmap;
    delete m_pMask;
  }
}

bool CPDF_ImageLoader::Start(const CPDF_ImageObject* pImage,
                             CPDF_PageRenderCache* pCache,
                             bool bStdCS,
                             uint32_t GroupFamily,
                             bool bLoadMask,
                             CPDF_RenderStatus* pRenderStatus,
                             int32_t nDownsampleWidth,
                             int32_t nDownsampleHeight) {
  m_nDownsampleWidth = nDownsampleWidth;
  m_nDownsampleHeight = nDownsampleHeight;
  m_pLoadHandle.reset(new CPDF_ImageLoaderHandle);
  return m_pLoadHandle->Start(this, pImage, pCache, bStdCS, GroupFamily,
                              bLoadMask, pRenderStatus, m_nDownsampleWidth,
                              m_nDownsampleHeight);
}

bool CPDF_ImageLoader::Continue(IFX_Pause* pPause) {
  return m_pLoadHandle->Continue(pPause);
}
