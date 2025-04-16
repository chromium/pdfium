// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_imageloader.h"

#include <utility>

#include "core/fpdfapi/page/cpdf_dib.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/page/cpdf_pageimagecache.h"
#include "core/fpdfapi/page/cpdf_transferfunc.h"
#include "core/fxcrt/check.h"
#include "core/fxge/dib/cfx_dibitmap.h"

CPDF_ImageLoader::CPDF_ImageLoader() = default;

CPDF_ImageLoader::~CPDF_ImageLoader() = default;

bool CPDF_ImageLoader::Start(const CPDF_ImageObject* pImage,
                             CPDF_PageImageCache* pPageImageCache,
                             const CPDF_Dictionary* pFormResource,
                             const CPDF_Dictionary* pPageResource,
                             bool bStdCS,
                             CPDF_ColorSpace::Family eFamily,
                             bool bLoadMask,
                             const CFX_Size& max_size_required) {
  cache_ = pPageImageCache;
  image_object_ = pImage;
  bool should_continue;
  if (cache_) {
    should_continue = cache_->StartGetCachedBitmap(
        image_object_->GetImage(), pFormResource, pPageResource, bStdCS,
        eFamily, bLoadMask, max_size_required);
  } else {
    should_continue = image_object_->GetImage()->StartLoadDIBBase(
        pFormResource, pPageResource, bStdCS, eFamily, bLoadMask,
        max_size_required);
  }
  if (!should_continue) {
    Finish();
  }
  return should_continue;
}

bool CPDF_ImageLoader::Continue(PauseIndicatorIface* pPause) {
  bool should_continue = cache_ ? cache_->Continue(pPause)
                                : image_object_->GetImage()->Continue(pPause);
  if (!should_continue) {
    Finish();
  }
  return should_continue;
}

RetainPtr<CFX_DIBBase> CPDF_ImageLoader::TranslateImage(
    RetainPtr<CPDF_TransferFunc> pTransferFunc) {
  DCHECK(pTransferFunc);
  DCHECK(!pTransferFunc->GetIdentity());
  bitmap_ = pTransferFunc->TranslateImage(std::move(bitmap_));
  if (cached_ && mask_) {
    mask_ = mask_->Realize();
  }
  cached_ = false;
  return bitmap_;
}

void CPDF_ImageLoader::Finish() {
  if (cache_) {
    cached_ = true;
    bitmap_ = cache_->DetachCurBitmap();
    mask_ = cache_->DetachCurMask();
    matte_color_ = cache_->GetCurMatteColor();
    return;
  }
  RetainPtr<CPDF_Image> pImage = image_object_->GetImage();
  cached_ = false;
  bitmap_ = pImage->DetachBitmap();
  mask_ = pImage->DetachMask();
  matte_color_ = pImage->GetMatteColor();
}
