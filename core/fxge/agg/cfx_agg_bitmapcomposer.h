// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_AGG_CFX_AGG_BITMAPCOMPOSER_H_
#define CORE_FXGE_AGG_CFX_AGG_BITMAPCOMPOSER_H_

#include <stdint.h>

#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/dib/cfx_scanlinecompositor.h"
#include "core/fxge/dib/fx_dib.h"
#include "core/fxge/dib/scanlinecomposer_iface.h"

class CFX_AggClipRgn;
class CFX_DIBitmap;
struct FX_RECT;

class CFX_AggBitmapComposer final : public ScanlineComposerIface {
 public:
  CFX_AggBitmapComposer();
  ~CFX_AggBitmapComposer() override;

  void Compose(const RetainPtr<CFX_DIBitmap>& pDest,
               const CFX_AggClipRgn* pClipRgn,
               float alpha,
               uint32_t mask_color,
               const FX_RECT& dest_rect,
               bool bVertical,
               bool bFlipX,
               bool bFlipY,
               bool bRgbByteOrder,
               BlendMode blend_mode);

  // ScanlineComposerIface:
  bool SetInfo(int width,
               int height,
               FXDIB_Format src_format,
               DataVector<uint32_t> src_palette) override;
  void ComposeScanline(int line, pdfium::span<const uint8_t> scanline) override;

 private:
  void DoCompose(pdfium::span<uint8_t> dest_scan,
                 pdfium::span<const uint8_t> src_scan,
                 int dest_width,
                 pdfium::span<const uint8_t> clip_scan);
  void ComposeScanlineV(int line, pdfium::span<const uint8_t> scanline);

  RetainPtr<CFX_DIBitmap> bitmap_;
  UnownedPtr<const CFX_AggClipRgn> clip_rgn_;
  FXDIB_Format src_format_;
  int dest_left_;
  int dest_top_;
  int dest_width_;
  int dest_height_;
  float alpha_;
  uint32_t mask_color_;
  RetainPtr<CFX_DIBitmap> clip_mask_;
  CFX_ScanlineCompositor compositor_;
  bool vertical_;
  bool flip_x_;
  bool flip_y_;
  bool rgb_byte_order_ = false;
  BlendMode blend_mode_ = BlendMode::kNormal;
  DataVector<uint8_t> scanline_v_;
  DataVector<uint8_t> clip_scan_v_;
  DataVector<uint8_t> add_clip_scan_;
};

#endif  // CORE_FXGE_AGG_CFX_AGG_BITMAPCOMPOSER_H_
