// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/agg/cfx_agg_imagerenderer.h"

#include <math.h>

#include <memory>
#include <utility>

#include "core/fxcrt/fx_system.h"
#include "core/fxge/agg/cfx_agg_cliprgn.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cfx_imagestretcher.h"
#include "core/fxge/dib/cfx_imagetransformer.h"

CFX_AggImageRenderer::CFX_AggImageRenderer(
    const RetainPtr<CFX_DIBitmap>& pDevice,
    const CFX_AggClipRgn* pClipRgn,
    RetainPtr<const CFX_DIBBase> source,
    float alpha,
    uint32_t mask_color,
    const CFX_Matrix& matrix,
    const FXDIB_ResampleOptions& options,
    bool bRgbByteOrder)
    : device_(pDevice),
      clip_rgn_(pClipRgn),
      matrix_(matrix),
      alpha_(alpha),
      mask_color_(mask_color),
      rgb_byte_order_(bRgbByteOrder) {
  FX_RECT image_rect = matrix_.GetUnitRect().GetOuterRect();
  clip_box_ = pClipRgn
                  ? pClipRgn->GetBox()
                  : FX_RECT(0, 0, pDevice->GetWidth(), pDevice->GetHeight());
  clip_box_.Intersect(image_rect);
  if (clip_box_.IsEmpty()) {
    return;
  }

  if ((fabs(matrix_.b) >= 0.5f || matrix_.a == 0) ||
      (fabs(matrix_.c) >= 0.5f || matrix_.d == 0)) {
    if (fabs(matrix_.a) < fabs(matrix_.b) / 20 &&
        fabs(matrix_.d) < fabs(matrix_.c) / 20 && fabs(matrix_.a) < 0.5f &&
        fabs(matrix_.d) < 0.5f) {
      int dest_width = image_rect.Width();
      int dest_height = image_rect.Height();
      FX_RECT bitmap_clip = clip_box_;
      bitmap_clip.Offset(-image_rect.left, -image_rect.top);
      bitmap_clip = bitmap_clip.SwappedClipBox(dest_width, dest_height,
                                               matrix_.c > 0, matrix_.b < 0);
      const bool flip_x = matrix_.c > 0;
      const bool flip_y = matrix_.b < 0;
      composer_.Compose(pDevice, pClipRgn, alpha, mask_color, clip_box_,
                        /*bVertical=*/true, flip_x, flip_y, rgb_byte_order_,
                        BlendMode::kNormal);
      stretcher_ = std::make_unique<CFX_ImageStretcher>(
          &composer_, std::move(source), dest_height, dest_width, bitmap_clip,
          options);
      if (stretcher_->Start()) {
        state_ = State::kStretching;
      }
      return;
    }
    state_ = State::kTransforming;
    transformer_ = std::make_unique<CFX_ImageTransformer>(
        std::move(source), matrix_, options, &clip_box_);
    return;
  }

  int dest_width = image_rect.Width();
  if (matrix_.a < 0) {
    dest_width = -dest_width;
  }

  int dest_height = image_rect.Height();
  if (matrix_.d > 0) {
    dest_height = -dest_height;
  }

  if (dest_width == 0 || dest_height == 0) {
    return;
  }

  FX_RECT bitmap_clip = clip_box_;
  bitmap_clip.Offset(-image_rect.left, -image_rect.top);
  composer_.Compose(pDevice, pClipRgn, alpha, mask_color, clip_box_,
                    /*bVertical=*/false, /*bFlipX=*/false, /*bFlipY=*/false,
                    rgb_byte_order_, BlendMode::kNormal);
  state_ = State::kStretching;
  stretcher_ = std::make_unique<CFX_ImageStretcher>(
      &composer_, std::move(source), dest_width, dest_height, bitmap_clip,
      options);
  stretcher_->Start();
}

CFX_AggImageRenderer::~CFX_AggImageRenderer() = default;

bool CFX_AggImageRenderer::Continue(PauseIndicatorIface* pPause) {
  if (state_ == State::kStretching) {
    return stretcher_->Continue(pPause);
  }
  if (state_ != State::kTransforming) {
    return false;
  }
  if (transformer_->Continue(pPause)) {
    return true;
  }

  RetainPtr<CFX_DIBitmap> pBitmap = transformer_->DetachBitmap();
  if (!pBitmap || pBitmap->GetBuffer().empty()) {
    return false;
  }

  if (pBitmap->IsMaskFormat()) {
    if (alpha_ != 1.0f) {
      mask_color_ = FXARGB_MUL_ALPHA(mask_color_, FXSYS_roundf(alpha_ * 255));
    }
    device_->CompositeMask(transformer_->result().left,
                           transformer_->result().top, pBitmap->GetWidth(),
                           pBitmap->GetHeight(), pBitmap, mask_color_, 0, 0,
                           BlendMode::kNormal, clip_rgn_, rgb_byte_order_);
  } else {
    pBitmap->MultiplyAlpha(alpha_);
    device_->CompositeBitmap(transformer_->result().left,
                             transformer_->result().top, pBitmap->GetWidth(),
                             pBitmap->GetHeight(), pBitmap, 0, 0,
                             BlendMode::kNormal, clip_rgn_, rgb_byte_order_);
  }
  return false;
}
