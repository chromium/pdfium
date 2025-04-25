// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/agg/cfx_agg_cliprgn.h"

#include <stdint.h>

#include <utility>

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/dib/cfx_dibitmap.h"

CFX_AggClipRgn::CFX_AggClipRgn(int width, int height)
    : box_(0, 0, width, height) {}

CFX_AggClipRgn::CFX_AggClipRgn(const CFX_AggClipRgn& src) = default;

CFX_AggClipRgn::~CFX_AggClipRgn() = default;

void CFX_AggClipRgn::IntersectRect(const FX_RECT& rect) {
  if (type_ == kRectI) {
    box_.Intersect(rect);
    return;
  }
  IntersectMaskRect(rect, box_, mask_);
}

void CFX_AggClipRgn::IntersectMaskRect(FX_RECT rect,
                                       FX_RECT mask_rect,
                                       RetainPtr<CFX_DIBitmap> pOldMask) {
  type_ = kMaskF;
  box_ = rect;
  box_.Intersect(mask_rect);
  if (box_.IsEmpty()) {
    type_ = kRectI;
    return;
  }
  if (box_ == mask_rect) {
    mask_ = std::move(pOldMask);
    return;
  }
  mask_ = pdfium::MakeRetain<CFX_DIBitmap>();
  CHECK(mask_->Create(box_.Width(), box_.Height(), FXDIB_Format::k8bppMask));
  const int offset = box_.left - mask_rect.left;
  for (int row = box_.top; row < box_.bottom; row++) {
    pdfium::span<uint8_t> dest_scan =
        mask_->GetWritableScanline(row - box_.top);
    pdfium::span<const uint8_t> src_scan =
        pOldMask->GetScanline(row - mask_rect.top);
    fxcrt::Copy(src_scan.subspan(static_cast<size_t>(offset),
                                 static_cast<size_t>(box_.Width())),
                dest_scan);
  }
}

void CFX_AggClipRgn::IntersectMaskF(int left,
                                    int top,
                                    RetainPtr<CFX_DIBitmap> pMask) {
  FX_RECT mask_box(left, top, left + pMask->GetWidth(),
                   top + pMask->GetHeight());
  if (!mask_box.IsEmpty()) {
    // Make sure non-empty masks have the right format. If the mask is empty,
    // then the format does not matter as it will not get used.
    CHECK_EQ(pMask->GetFormat(), FXDIB_Format::k8bppMask);
  }
  if (type_ == kRectI) {
    IntersectMaskRect(box_, mask_box, std::move(pMask));
    return;
  }

  FX_RECT new_box = box_;
  new_box.Intersect(mask_box);
  if (new_box.IsEmpty()) {
    type_ = kRectI;
    mask_ = nullptr;
    box_ = new_box;
    return;
  }
  auto new_dib = pdfium::MakeRetain<CFX_DIBitmap>();
  CHECK(new_dib->Create(new_box.Width(), new_box.Height(),
                        FXDIB_Format::k8bppMask));
  for (int row = new_box.top; row < new_box.bottom; row++) {
    pdfium::span<const uint8_t> old_scan = mask_->GetScanline(row - box_.top);
    pdfium::span<const uint8_t> mask_scan = pMask->GetScanline(row - top);
    auto new_scan = new_dib->GetWritableScanline(row - new_box.top);
    for (int col = new_box.left; col < new_box.right; col++) {
      new_scan[col - new_box.left] =
          old_scan[col - box_.left] * mask_scan[col - left] / 255;
    }
  }
  box_ = new_box;
  mask_ = std::move(new_dib);
}
