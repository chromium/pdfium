// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_AGG_CFX_AGG_IMAGERENDERER_H_
#define CORE_FXGE_AGG_CFX_AGG_IMAGERENDERER_H_

#include <memory>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/agg/cfx_agg_bitmapcomposer.h"

class CFX_DIBBase;
class CFX_DIBitmap;
class CFX_ImageTransformer;
class CFX_ImageStretcher;
class PauseIndicatorIface;

class CFX_AggImageRenderer {
 public:
  CFX_AggImageRenderer(const RetainPtr<CFX_DIBitmap>& pDevice,
                       const CFX_AggClipRgn* pClipRgn,
                       RetainPtr<const CFX_DIBBase> source,
                       float alpha,
                       uint32_t mask_color,
                       const CFX_Matrix& matrix,
                       const FXDIB_ResampleOptions& options,
                       bool bRgbByteOrder);
  ~CFX_AggImageRenderer();

  bool Continue(PauseIndicatorIface* pPause);

 private:
  enum class State : uint8_t { kInitial = 0, kStretching, kTransforming };

  RetainPtr<CFX_DIBitmap> const device_;
  UnownedPtr<const CFX_AggClipRgn> const clip_rgn_;
  const CFX_Matrix matrix_;
  std::unique_ptr<CFX_ImageTransformer> transformer_;
  std::unique_ptr<CFX_ImageStretcher> stretcher_;
  CFX_AggBitmapComposer composer_;
  FX_RECT clip_box_;
  const float alpha_;
  uint32_t mask_color_;
  State state_ = State::kInitial;
  const bool rgb_byte_order_;
};

#endif  // CORE_FXGE_AGG_CFX_AGG_IMAGERENDERER_H_
