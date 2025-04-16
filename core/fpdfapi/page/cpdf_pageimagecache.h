// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_PAGEIMAGECACHE_H_
#define CORE_FPDFAPI_PAGE_CPDF_PAGEIMAGECACHE_H_

#include <stdint.h>

#include <functional>
#include <map>
#include <memory>

#include "core/fpdfapi/page/cpdf_dib.h"
#include "core/fxcrt/maybe_owned.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CPDF_Dictionary;
class CPDF_Image;
class CPDF_Page;
class CPDF_Stream;
class PauseIndicatorIface;

class CPDF_PageImageCache {
 public:
  explicit CPDF_PageImageCache(CPDF_Page* pPage);
  ~CPDF_PageImageCache();

  void ResetBitmapForImage(RetainPtr<CPDF_Image> pImage);
  void CacheOptimization(int32_t dwLimitCacheSize);
  uint32_t GetTimeCount() const { return time_count_; }
  CPDF_Page* GetPage() const { return page_; }

  bool StartGetCachedBitmap(RetainPtr<CPDF_Image> pImage,
                            const CPDF_Dictionary* pFormResources,
                            const CPDF_Dictionary* pPageResources,
                            bool bStdCS,
                            CPDF_ColorSpace::Family eFamily,
                            bool bLoadMask,
                            const CFX_Size& max_size_required);

  bool Continue(PauseIndicatorIface* pPause);

  uint32_t GetCurMatteColor() const;
  RetainPtr<CFX_DIBBase> DetachCurBitmap();
  RetainPtr<CFX_DIBBase> DetachCurMask();

 private:
  class Entry {
   public:
    explicit Entry(RetainPtr<CPDF_Image> pImage);
    ~Entry();

    void Reset();
    uint32_t EstimateSize() const { return cache_size_; }
    uint32_t GetMatteColor() const { return matte_color_; }
    uint32_t GetTimeCount() const { return time_count_; }
    void SetTimeCount(uint32_t count) { time_count_ = count; }
    CPDF_Image* GetImage() const { return image_.Get(); }

    CPDF_DIB::LoadState StartGetCachedBitmap(
        CPDF_PageImageCache* pPageImageCache,
        const CPDF_Dictionary* pFormResources,
        const CPDF_Dictionary* pPageResources,
        bool bStdCS,
        CPDF_ColorSpace::Family eFamily,
        bool bLoadMask,
        const CFX_Size& max_size_required);

    // Returns whether to Continue() or not.
    bool Continue(PauseIndicatorIface* pPause,
                  CPDF_PageImageCache* pPageImageCache);

    RetainPtr<CFX_DIBBase> DetachBitmap();
    RetainPtr<CFX_DIBBase> DetachMask();

   private:
    void ContinueGetCachedBitmap(CPDF_PageImageCache* pPageImageCache);
    void CalcSize();
    bool IsCacheValid(const CFX_Size& max_size_required) const;

    uint32_t time_count_ = 0;
    uint32_t matte_color_ = 0;
    uint32_t cache_size_ = 0;
    RetainPtr<CPDF_Image> const image_;
    RetainPtr<CFX_DIBBase> cur_bitmap_;
    RetainPtr<CFX_DIBBase> cur_mask_;
    RetainPtr<CFX_DIBBase> cached_bitmap_;
    RetainPtr<CFX_DIBBase> cached_mask_;
    bool cached_set_max_size_required_ = false;
  };

  void ClearImageCacheEntry(const CPDF_Stream* pStream);

  UnownedPtr<CPDF_Page> const page_;
  std::map<RetainPtr<const CPDF_Stream>, std::unique_ptr<Entry>, std::less<>>
      image_cache_;
  MaybeOwned<Entry> cur_image_cache_entry_;
  uint32_t time_count_ = 0;
  uint32_t cache_size_ = 0;
  bool cur_find_cache_ = false;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_PAGEIMAGECACHE_H_
