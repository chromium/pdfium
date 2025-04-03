// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_imagerenderer.h"

#include <utility>

#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxge/agg/cfx_agg_imagerenderer.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/dib/cfx_dibitmap.h"

CXFA_ImageRenderer::CXFA_ImageRenderer(CFX_RenderDevice* device,
                                       RetainPtr<CFX_DIBitmap> bitmap,
                                       const CFX_Matrix& image_to_device)
    : image_matrix_(image_to_device),
      device_(device),
      bitmap_(std::move(bitmap)) {
  // Assume this always draws into CFX_DefaultRenderDevice.
  CHECK(device_);
  CHECK(device_->GetRenderCaps() & FXRC_GET_BITS);
  CHECK(bitmap_);
}

CXFA_ImageRenderer::~CXFA_ImageRenderer() = default;

bool CXFA_ImageRenderer::Start() {
  FXDIB_ResampleOptions options;
  options.bInterpolateBilinear = true;
  RenderDeviceDriverIface::StartResult result = device_->StartDIBits(
      bitmap_, /*alpha=*/1.0f, /*argb=*/0, image_matrix_, options);
  if (result.result == RenderDeviceDriverIface::Result::kFailure) {
    return false;
  }

  CHECK_EQ(result.result, RenderDeviceDriverIface::Result::kSuccess);
  device_handle_ = std::move(result.agg_image_renderer);
  if (!device_handle_) {
    return false;
  }

  state_ = State::kStarted;
  return true;
}

bool CXFA_ImageRenderer::Continue() {
  CHECK_EQ(state_, State::kStarted);
  return device_->ContinueDIBits(device_handle_.get(), nullptr);
}
