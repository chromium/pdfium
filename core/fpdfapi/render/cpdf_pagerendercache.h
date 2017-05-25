// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_PAGERENDERCACHE_H_
#define CORE_FPDFAPI_RENDER_CPDF_PAGERENDERCACHE_H_

#include <map>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxcrt/fx_system.h"

class CFX_DIBitmap;
class CPDF_Image;
class CPDF_ImageCacheEntry;
class CPDF_Page;
class CPDF_RenderStatus;
class CPDF_Stream;
class IFX_Pause;

class CPDF_PageRenderCache {
 public:
  explicit CPDF_PageRenderCache(CPDF_Page* pPage);
  ~CPDF_PageRenderCache();

  void CacheOptimization(int32_t dwLimitCacheSize);
  uint32_t GetTimeCount() const { return m_nTimeCount; }
  void ResetBitmap(const CFX_RetainPtr<CPDF_Image>& pImage,
                   const CFX_RetainPtr<CFX_DIBitmap>& pBitmap);
  CPDF_Page* GetPage() const { return m_pPage.Get(); }
  CPDF_ImageCacheEntry* GetCurImageCacheEntry() const {
    return m_pCurImageCacheEntry;
  }

  bool StartGetCachedBitmap(const CFX_RetainPtr<CPDF_Image>& pImage,
                            bool bStdCS,
                            uint32_t GroupFamily,
                            bool bLoadMask,
                            CPDF_RenderStatus* pRenderStatus);

  bool Continue(IFX_Pause* pPause, CPDF_RenderStatus* pRenderStatus);

 private:
  void ClearImageCacheEntry(CPDF_Stream* pStream);

  CFX_UnownedPtr<CPDF_Page> const m_pPage;
  CPDF_ImageCacheEntry* m_pCurImageCacheEntry;
  std::map<CPDF_Stream*, CPDF_ImageCacheEntry*> m_ImageCache;
  uint32_t m_nTimeCount;
  uint32_t m_nCacheSize;
  bool m_bCurFindCache;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_PAGERENDERCACHE_H_
