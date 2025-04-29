// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/agg/cfx_agg_bitmapcomposer.h"

#include <stddef.h>

#include <algorithm>

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_2d_size.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/agg/cfx_agg_cliprgn.h"
#include "core/fxge/dib/cfx_dibitmap.h"

CFX_AggBitmapComposer::CFX_AggBitmapComposer() = default;

CFX_AggBitmapComposer::~CFX_AggBitmapComposer() = default;

void CFX_AggBitmapComposer::Compose(const RetainPtr<CFX_DIBitmap>& pDest,
                                    const CFX_AggClipRgn* pClipRgn,
                                    float alpha,
                                    uint32_t mask_color,
                                    const FX_RECT& dest_rect,
                                    bool bVertical,
                                    bool bFlipX,
                                    bool bFlipY,
                                    bool bRgbByteOrder,
                                    BlendMode blend_mode) {
  bitmap_ = pDest;
  clip_rgn_ = pClipRgn;
  dest_left_ = dest_rect.left;
  dest_top_ = dest_rect.top;
  dest_width_ = dest_rect.Width();
  dest_height_ = dest_rect.Height();
  alpha_ = alpha;
  mask_color_ = mask_color;
  clip_mask_ = nullptr;
  if (pClipRgn && pClipRgn->GetType() != CFX_AggClipRgn::kRectI) {
    clip_mask_ = pClipRgn->GetMask();
  }
  vertical_ = bVertical;
  flip_x_ = bFlipX;
  flip_y_ = bFlipY;
  rgb_byte_order_ = bRgbByteOrder;
  blend_mode_ = blend_mode;
}

bool CFX_AggBitmapComposer::SetInfo(int width,
                                    int height,
                                    FXDIB_Format src_format,
                                    DataVector<uint32_t> src_palette) {
  DCHECK_NE(src_format, FXDIB_Format::k1bppMask);
  DCHECK_NE(src_format, FXDIB_Format::k1bppRgb);
  src_format_ = src_format;
  if (!compositor_.Init(bitmap_->GetFormat(), src_format, src_palette,
                        mask_color_, blend_mode_, rgb_byte_order_)) {
    return false;
  }
  if (vertical_) {
    scanline_v_.resize(bitmap_->GetBPP() / 8 * width + 4);
    clip_scan_v_.resize(bitmap_->GetHeight());
  }
  if (alpha_ != 1.0f) {
    add_clip_scan_.resize(vertical_ ? bitmap_->GetHeight()
                                    : bitmap_->GetWidth());
  }
  return true;
}

void CFX_AggBitmapComposer::DoCompose(pdfium::span<uint8_t> dest_scan,
                                      pdfium::span<const uint8_t> src_scan,
                                      int dest_width,
                                      pdfium::span<const uint8_t> clip_scan) {
  if (alpha_ != 1.0f) {
    if (!clip_scan.empty()) {
      for (int i = 0; i < dest_width; ++i) {
        add_clip_scan_[i] = clip_scan[i] * alpha_;
      }
    } else {
      std::ranges::fill(
          pdfium::span(add_clip_scan_).first(static_cast<size_t>(dest_width)),
          FXSYS_roundf(alpha_ * 255));
    }
    clip_scan = add_clip_scan_;
  }
  if (src_format_ == FXDIB_Format::k8bppMask) {
    compositor_.CompositeByteMaskLine(dest_scan, src_scan, dest_width,
                                      clip_scan);
  } else if (src_format_ == FXDIB_Format::k8bppRgb) {
    compositor_.CompositePalBitmapLine(dest_scan, src_scan, 0, dest_width,
                                       clip_scan);
  } else {
    compositor_.CompositeRgbBitmapLine(dest_scan, src_scan, dest_width,
                                       clip_scan);
  }
}

void CFX_AggBitmapComposer::ComposeScanline(
    int line,
    pdfium::span<const uint8_t> scanline) {
  if (vertical_) {
    ComposeScanlineV(line, scanline);
    return;
  }
  pdfium::span<const uint8_t> clip_scan;
  if (clip_mask_) {
    clip_scan =
        clip_mask_
            ->GetWritableScanline(dest_top_ + line - clip_rgn_->GetBox().top)
            .subspan(
                static_cast<size_t>(dest_left_ - clip_rgn_->GetBox().left));
  }
  pdfium::span<uint8_t> dest_scan =
      bitmap_->GetWritableScanline(line + dest_top_);
  if (!dest_scan.empty()) {
    FX_SAFE_UINT32 offset = dest_left_;
    offset *= bitmap_->GetBPP();
    offset /= 8;
    if (!offset.IsValid()) {
      return;
    }

    dest_scan = dest_scan.subspan(offset.ValueOrDie());
  }
  DoCompose(dest_scan, scanline, dest_width_, clip_scan);
}

void CFX_AggBitmapComposer::ComposeScanlineV(
    int line,
    pdfium::span<const uint8_t> scanline) {
  const int bytes_per_pixel = bitmap_->GetBPP() / 8;
  int dest_pitch = bitmap_->GetPitch();
  int dest_x = dest_left_ + (flip_x_ ? (dest_width_ - line - 1) : line);
  pdfium::span<uint8_t> dest_span = bitmap_->GetWritableBuffer();
  if (!dest_span.empty()) {
    const size_t dest_x_offset = Fx2DSizeOrDie(dest_x, bytes_per_pixel);
    const size_t dest_y_offset = Fx2DSizeOrDie(dest_top_, dest_pitch);
    dest_span = dest_span.subspan(dest_y_offset).subspan(dest_x_offset);
    if (flip_y_) {
      const size_t dest_flip_offset =
          Fx2DSizeOrDie(dest_pitch, dest_height_ - 1);
      dest_span = dest_span.subspan(dest_flip_offset);
    }
  }
  uint8_t* dest_buf = dest_span.data();
  const int y_step = flip_y_ ? -dest_pitch : dest_pitch;
  uint8_t* src_scan = scanline_v_.data();
  uint8_t* dest_scan = dest_buf;
  UNSAFE_TODO({
    for (int i = 0; i < dest_height_; ++i) {
      for (int j = 0; j < bytes_per_pixel; ++j) {
        *src_scan++ = dest_scan[j];
      }
      dest_scan += y_step;
    }
    pdfium::span<uint8_t> clip_scan;
    if (clip_mask_) {
      clip_scan = clip_scan_v_;
      int clip_pitch = clip_mask_->GetPitch();
      const uint8_t* src_clip =
          clip_mask_->GetScanline(dest_top_ - clip_rgn_->GetBox().top)
              .subspan(static_cast<size_t>(dest_x - clip_rgn_->GetBox().left))
              .data();
      if (flip_y_) {
        src_clip += Fx2DSizeOrDie(clip_pitch, dest_height_ - 1);
        clip_pitch = -clip_pitch;
      }
      for (int i = 0; i < dest_height_; ++i) {
        clip_scan[i] = *src_clip;
        src_clip += clip_pitch;
      }
    }
    DoCompose(scanline_v_, scanline, dest_height_, clip_scan);
    src_scan = scanline_v_.data();
    dest_scan = dest_buf;
    for (int i = 0; i < dest_height_; ++i) {
      for (int j = 0; j < bytes_per_pixel; ++j) {
        dest_scan[j] = *src_scan++;
      }
      dest_scan += y_step;
    }
  });
}
