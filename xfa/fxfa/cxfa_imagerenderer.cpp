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
    : m_ImageMatrix(image_to_device),
      m_pDevice(device),
      m_pBitmap(std::move(bitmap)) {
  // Assume this always draws into CFX_DefaultRenderDevice.
  CHECK(m_pDevice);
  CHECK(m_pDevice->GetRenderCaps() & FXRC_GET_BITS);
  CHECK(m_pBitmap);
}

CXFA_ImageRenderer::~CXFA_ImageRenderer() = default;

bool CXFA_ImageRenderer::Start() {
  FXDIB_ResampleOptions options;
  options.bInterpolateBilinear = true;
  RenderDeviceDriverIface::StartResult result = m_pDevice->StartDIBits(
      m_pBitmap, /*alpha=*/1.0f, /*argb=*/0, m_ImageMatrix, options);
  if (result.result == RenderDeviceDriverIface::Result::kFailure) {
    return false;
  }

  CHECK_EQ(result.result, RenderDeviceDriverIface::Result::kSuccess);
  m_DeviceHandle = std::move(result.agg_image_renderer);
  if (!m_DeviceHandle) {
    return false;
  }

  m_State = State::kStarted;
  return true;
}

bool CXFA_ImageRenderer::Continue() {
  CHECK_EQ(m_State, State::kStarted);
  return m_pDevice->ContinueDIBits(m_DeviceHandle.get(), nullptr);
}
