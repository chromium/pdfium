// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_PAGERENDERCACHE_H_
#define CORE_FPDFAPI_RENDER_CPDF_PAGERENDERCACHE_H_

#include <stdint.h>

#include <map>
#include <memory>

#include "core/fpdfapi/page/cpdf_dib.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fxcrt/maybe_owned.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Image;
class CPDF_Page;
class CPDF_RenderStatus;
class CPDF_Stream;
class PauseIndicatorIface;

class CPDF_PageRenderCache final : public CPDF_Page::RenderCacheIface {
 public:
  explicit CPDF_PageRenderCache(CPDF_Page* pPage);
  ~CPDF_PageRenderCache() override;

  // CPDF_Page::RenderCacheIface:
  void ResetBitmapForImage(RetainPtr<CPDF_Image> pImage) override;

  void CacheOptimization(int32_t dwLimitCacheSize);
  uint32_t GetTimeCount() const { return m_nTimeCount; }
  CPDF_Page* GetPage() const { return m_pPage.Get(); }

  bool StartGetCachedBitmap(RetainPtr<CPDF_Image> pImage,
                            const CPDF_RenderStatus* pRenderStatus,
                            bool bStdCS);

  bool Continue(PauseIndicatorIface* pPause, CPDF_RenderStatus* pRenderStatus);

  uint32_t GetCurMatteColor() const;
  RetainPtr<CFX_DIBBase> DetachCurBitmap();
  RetainPtr<CFX_DIBBase> DetachCurMask();

 private:
  class ImageCacheEntry {
   public:
    ImageCacheEntry(CPDF_Document* pDoc, RetainPtr<CPDF_Image> pImage);
    ~ImageCacheEntry();

    void Reset();
    uint32_t EstimateSize() const { return m_dwCacheSize; }
    uint32_t GetMatteColor() const { return m_MatteColor; }
    uint32_t GetTimeCount() const { return m_dwTimeCount; }
    void SetTimeCount(uint32_t count) { m_dwTimeCount = count; }
    CPDF_Image* GetImage() const { return m_pImage.Get(); }

    CPDF_DIB::LoadState StartGetCachedBitmap(
        const CPDF_Dictionary* pPageResources,
        const CPDF_RenderStatus* pRenderStatus,
        bool bStdCS);

    // Returns whether to Continue() or not.
    bool Continue(PauseIndicatorIface* pPause,
                  CPDF_RenderStatus* pRenderStatus);

    RetainPtr<CFX_DIBBase> DetachBitmap();
    RetainPtr<CFX_DIBBase> DetachMask();

   private:
    void ContinueGetCachedBitmap(const CPDF_RenderStatus* pRenderStatus);
    void CalcSize();

    uint32_t m_dwTimeCount = 0;
    uint32_t m_MatteColor = 0;
    uint32_t m_dwCacheSize = 0;
    UnownedPtr<CPDF_Document> const m_pDocument;
    RetainPtr<CPDF_Image> const m_pImage;
    RetainPtr<CFX_DIBBase> m_pCurBitmap;
    RetainPtr<CFX_DIBBase> m_pCurMask;
    RetainPtr<CFX_DIBBase> m_pCachedBitmap;
    RetainPtr<CFX_DIBBase> m_pCachedMask;
  };

  void ClearImageCacheEntry(const CPDF_Stream* pStream);

  UnownedPtr<CPDF_Page> const m_pPage;
  std::map<const CPDF_Stream*, std::unique_ptr<ImageCacheEntry>> m_ImageCache;
  MaybeOwned<ImageCacheEntry> m_pCurImageCacheEntry;
  uint32_t m_nTimeCount = 0;
  uint32_t m_nCacheSize = 0;
  bool m_bCurFindCache = false;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_PAGERENDERCACHE_H_
