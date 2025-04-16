// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_pageimagecache.h"

#include <stddef.h>
#include <stdint.h>

#include <algorithm>
#include <utility>
#include <vector>

#include "core/fpdfapi/page/cpdf_dib.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/dib/cfx_dibbase.h"
#include "core/fxge/dib/cfx_dibitmap.h"

#if defined(PDF_USE_SKIA)
#include "core/fxcrt/data_vector.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "third_party/skia/include/core/SkImage.h"   // nogncheck
#include "third_party/skia/include/core/SkRefCnt.h"  // nogncheck
#endif

namespace {

struct CacheInfo {
  CacheInfo(uint32_t t, RetainPtr<const CPDF_Stream> stream)
      : time(t), pStream(std::move(stream)) {}

  uint32_t time;
  RetainPtr<const CPDF_Stream> pStream;

  bool operator<(const CacheInfo& other) const { return time < other.time; }
};

#if defined(PDF_USE_SKIA)
// Wrapper around a `CFX_DIBBase` that memoizes `RealizeSkImage()`. This is only
// safe if the underlying `CFX_DIBBase` is not mutable.
class CachedImage final : public CFX_DIBBase {
 public:
  explicit CachedImage(RetainPtr<CFX_DIBBase> image)
      : image_(std::move(image)) {
    SetFormat(image_->GetFormat());
    SetWidth(image_->GetWidth());
    SetHeight(image_->GetHeight());
    SetPitch(image_->GetPitch());

    if (image_->HasPalette()) {
      pdfium::span<const uint32_t> palette = image_->GetPaletteSpan();
      palette_ = DataVector<uint32_t>(palette.begin(), palette.end());
    }
  }

  pdfium::span<const uint8_t> GetScanline(int line) const override {
    // TODO(crbug.com/pdfium/2050): Still needed for `Realize()` call in
    // `CPDF_ImageRenderer`.
    return image_->GetScanline(line);
  }

  bool SkipToScanline(int line, PauseIndicatorIface* pause) const override {
    return image_->SkipToScanline(line, pause);
  }

  size_t GetEstimatedImageMemoryBurden() const override {
    // A better estimate would account for realizing the `SkImage`.
    return image_->GetEstimatedImageMemoryBurden();
  }

#if BUILDFLAG(IS_WIN) || defined(PDF_USE_SKIA)
  RetainPtr<const CFX_DIBitmap> RealizeIfNeeded() const override {
    return image_->RealizeIfNeeded();
  }
#endif

  sk_sp<SkImage> RealizeSkImage() const override {
    if (!cached_skia_image_) {
      cached_skia_image_ = image_->RealizeSkImage();
    }
    return cached_skia_image_;
  }

 private:
  RetainPtr<CFX_DIBBase> image_;
  mutable sk_sp<SkImage> cached_skia_image_;
};
#endif  // defined(PDF_USE_SKIA)

// Makes a `CachedImage` backed by `image` if Skia is the default renderer,
// otherwise return the image itself. `realize_hint` indicates whether it would
// be beneficial to realize `image` before caching.
RetainPtr<CFX_DIBBase> MakeCachedImage(RetainPtr<CFX_DIBBase> image,
                                       bool realize_hint) {
#if defined(PDF_USE_SKIA)
  if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
    // Ignore `realize_hint`, as `RealizeSkImage()` doesn't benefit from it.
    return pdfium::MakeRetain<CachedImage>(std::move(image));
  }
#endif  // defined(PDF_USE_SKIA)
  return realize_hint ? image->Realize() : image;
}

}  // namespace

CPDF_PageImageCache::CPDF_PageImageCache(CPDF_Page* pPage) : page_(pPage) {}

CPDF_PageImageCache::~CPDF_PageImageCache() = default;

void CPDF_PageImageCache::CacheOptimization(int32_t dwLimitCacheSize) {
  if (cache_size_ <= (uint32_t)dwLimitCacheSize) {
    return;
  }

  uint32_t nCount = fxcrt::CollectionSize<uint32_t>(image_cache_);
  std::vector<CacheInfo> cache_info;
  cache_info.reserve(nCount);
  for (const auto& it : image_cache_) {
    cache_info.emplace_back(it.second->GetTimeCount(),
                            it.second->GetImage()->GetStream());
  }
  std::sort(cache_info.begin(), cache_info.end());

  // Check if time value is about to roll over and reset all entries.
  // The comparison is legal because uint32_t is an unsigned type.
  uint32_t nTimeCount = time_count_;
  if (nTimeCount + 1 < nTimeCount) {
    for (uint32_t i = 0; i < nCount; i++) {
      image_cache_[cache_info[i].pStream]->SetTimeCount(i);
    }
    time_count_ = nCount;
  }

  size_t i = 0;
  while (i + 15 < nCount) {
    ClearImageCacheEntry(cache_info[i++].pStream);
  }

  while (i < nCount && cache_size_ > (uint32_t)dwLimitCacheSize) {
    ClearImageCacheEntry(cache_info[i++].pStream);
  }
}

void CPDF_PageImageCache::ClearImageCacheEntry(const CPDF_Stream* pStream) {
  auto it = image_cache_.find(pStream);
  if (it == image_cache_.end()) {
    return;
  }

  cache_size_ -= it->second->EstimateSize();

  // Avoid leaving `cur_image_cache_entry_` as a dangling pointer when `it` is
  // about to be deleted.
  if (cur_image_cache_entry_.Get() == it->second.get()) {
    DCHECK(!cur_image_cache_entry_.IsOwned());
    cur_image_cache_entry_.Reset();
  }
  image_cache_.erase(it);
}

bool CPDF_PageImageCache::StartGetCachedBitmap(
    RetainPtr<CPDF_Image> pImage,
    const CPDF_Dictionary* pFormResources,
    const CPDF_Dictionary* pPageResources,
    bool bStdCS,
    CPDF_ColorSpace::Family eFamily,
    bool bLoadMask,
    const CFX_Size& max_size_required) {
  // A cross-document image may have come from the embedder.
  if (page_->GetDocument() != pImage->GetDocument()) {
    return false;
  }

  RetainPtr<const CPDF_Stream> pStream = pImage->GetStream();
  const auto it = image_cache_.find(pStream);
  cur_find_cache_ = it != image_cache_.end();
  if (cur_find_cache_) {
    cur_image_cache_entry_ = it->second.get();
  } else {
    cur_image_cache_entry_ = std::make_unique<Entry>(std::move(pImage));
  }
  CPDF_DIB::LoadState ret = cur_image_cache_entry_->StartGetCachedBitmap(
      this, pFormResources, pPageResources, bStdCS, eFamily, bLoadMask,
      max_size_required);
  if (ret == CPDF_DIB::LoadState::kContinue) {
    return true;
  }

  time_count_++;
  if (!cur_find_cache_) {
    image_cache_[pStream] = cur_image_cache_entry_.Release();
  }

  if (ret == CPDF_DIB::LoadState::kFail) {
    cache_size_ += cur_image_cache_entry_->EstimateSize();
  }

  return false;
}

bool CPDF_PageImageCache::Continue(PauseIndicatorIface* pPause) {
  bool ret = cur_image_cache_entry_->Continue(pPause, this);
  if (ret) {
    return true;
  }

  time_count_++;
  if (!cur_find_cache_) {
    image_cache_[cur_image_cache_entry_->GetImage()->GetStream()] =
        cur_image_cache_entry_.Release();
  }
  cache_size_ += cur_image_cache_entry_->EstimateSize();
  return false;
}

void CPDF_PageImageCache::ResetBitmapForImage(RetainPtr<CPDF_Image> pImage) {
  RetainPtr<const CPDF_Stream> pStream = pImage->GetStream();
  const auto it = image_cache_.find(pStream);
  if (it == image_cache_.end()) {
    return;
  }

  Entry* pEntry = it->second.get();
  cache_size_ -= pEntry->EstimateSize();
  pEntry->Reset();
  cache_size_ += pEntry->EstimateSize();
}

uint32_t CPDF_PageImageCache::GetCurMatteColor() const {
  return cur_image_cache_entry_->GetMatteColor();
}

RetainPtr<CFX_DIBBase> CPDF_PageImageCache::DetachCurBitmap() {
  return cur_image_cache_entry_->DetachBitmap();
}

RetainPtr<CFX_DIBBase> CPDF_PageImageCache::DetachCurMask() {
  return cur_image_cache_entry_->DetachMask();
}

CPDF_PageImageCache::Entry::Entry(RetainPtr<CPDF_Image> pImage)
    : image_(std::move(pImage)) {}

CPDF_PageImageCache::Entry::~Entry() = default;

void CPDF_PageImageCache::Entry::Reset() {
  cached_bitmap_.Reset();
  CalcSize();
}

RetainPtr<CFX_DIBBase> CPDF_PageImageCache::Entry::DetachBitmap() {
  return std::move(cur_bitmap_);
}

RetainPtr<CFX_DIBBase> CPDF_PageImageCache::Entry::DetachMask() {
  return std::move(cur_mask_);
}

CPDF_DIB::LoadState CPDF_PageImageCache::Entry::StartGetCachedBitmap(
    CPDF_PageImageCache* pPageImageCache,
    const CPDF_Dictionary* pFormResources,
    const CPDF_Dictionary* pPageResources,
    bool bStdCS,
    CPDF_ColorSpace::Family eFamily,
    bool bLoadMask,
    const CFX_Size& max_size_required) {
  if (cached_bitmap_ && IsCacheValid(max_size_required)) {
    cur_bitmap_ = cached_bitmap_;
    cur_mask_ = cached_mask_;
    return CPDF_DIB::LoadState::kSuccess;
  }

  cur_bitmap_ = image_->CreateNewDIB();
  CPDF_DIB::LoadState ret = cur_bitmap_.AsRaw<CPDF_DIB>()->StartLoadDIBBase(
      true, pFormResources, pPageResources, bStdCS, eFamily, bLoadMask,
      max_size_required);
  cached_set_max_size_required_ =
      (max_size_required.width != 0 && max_size_required.height != 0);
  if (ret == CPDF_DIB::LoadState::kContinue) {
    return CPDF_DIB::LoadState::kContinue;
  }

  if (ret == CPDF_DIB::LoadState::kSuccess) {
    ContinueGetCachedBitmap(pPageImageCache);
  } else {
    cur_bitmap_.Reset();
  }
  return CPDF_DIB::LoadState::kFail;
}

bool CPDF_PageImageCache::Entry::Continue(
    PauseIndicatorIface* pPause,
    CPDF_PageImageCache* pPageImageCache) {
  CPDF_DIB::LoadState ret =
      cur_bitmap_.AsRaw<CPDF_DIB>()->ContinueLoadDIBBase(pPause);
  if (ret == CPDF_DIB::LoadState::kContinue) {
    return true;
  }

  if (ret == CPDF_DIB::LoadState::kSuccess) {
    ContinueGetCachedBitmap(pPageImageCache);
  } else {
    cur_bitmap_.Reset();
  }
  return false;
}

void CPDF_PageImageCache::Entry::ContinueGetCachedBitmap(
    CPDF_PageImageCache* pPageImageCache) {
  matte_color_ = cur_bitmap_.AsRaw<CPDF_DIB>()->GetMatteColor();
  cur_mask_ = cur_bitmap_.AsRaw<CPDF_DIB>()->DetachMask();
  time_count_ = pPageImageCache->GetTimeCount();
  if (cur_bitmap_->GetPitch() * cur_bitmap_->GetHeight() < kHugeImageSize) {
    cached_bitmap_ = MakeCachedImage(cur_bitmap_, /*realize_hint=*/true);
    cur_bitmap_.Reset();
  } else {
    cached_bitmap_ = MakeCachedImage(cur_bitmap_, /*realize_hint=*/false);
  }
  if (cur_mask_) {
    cached_mask_ = MakeCachedImage(cur_mask_, /*realize_hint=*/true);
    cur_mask_.Reset();
  }
  cur_bitmap_ = cached_bitmap_;
  cur_mask_ = cached_mask_;
  CalcSize();
}

void CPDF_PageImageCache::Entry::CalcSize() {
  cache_size_ = 0;
  if (cached_bitmap_) {
    cache_size_ += cached_bitmap_->GetEstimatedImageMemoryBurden();
  }
  if (cached_mask_) {
    cache_size_ += cached_mask_->GetEstimatedImageMemoryBurden();
  }
}

bool CPDF_PageImageCache::Entry::IsCacheValid(
    const CFX_Size& max_size_required) const {
  if (!cached_set_max_size_required_) {
    return true;
  }
  if (max_size_required.width == 0 && max_size_required.height == 0) {
    return false;
  }

  return (cached_bitmap_->GetWidth() >= max_size_required.width) &&
         (cached_bitmap_->GetHeight() >= max_size_required.height);
}
