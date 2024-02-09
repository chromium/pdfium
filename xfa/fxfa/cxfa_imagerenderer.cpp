// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_imagerenderer.h"

#include <math.h>

#include <utility>

#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/dib/cfx_dibbase.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cfx_imagerenderer.h"
#include "core/fxge/dib/cfx_imagetransformer.h"

CXFA_ImageRenderer::CXFA_ImageRenderer(CFX_RenderDevice* pDevice,
                                       const RetainPtr<CFX_DIBBase>& pDIBBase,
                                       const CFX_Matrix& pImage2Device)
    : m_ImageMatrix(pImage2Device), m_pDevice(pDevice), m_pDIBBase(pDIBBase) {}

CXFA_ImageRenderer::~CXFA_ImageRenderer() = default;

bool CXFA_ImageRenderer::Start() {
  FXDIB_ResampleOptions options;
  options.bInterpolateBilinear = true;
  if (m_pDevice->StartDIBits(m_pDIBBase, /*alpha=*/1.0f, /*argb=*/0,
                             m_ImageMatrix, options, &m_DeviceHandle)) {
    if (m_DeviceHandle) {
      m_State = State::kStarted;
      return true;
    }
    return false;
  }
  CFX_FloatRect image_rect_f = m_ImageMatrix.GetUnitRect();
  FX_RECT image_rect = image_rect_f.GetOuterRect();
  int dest_width = image_rect.Width();
  int dest_height = image_rect.Height();
  if ((fabs(m_ImageMatrix.b) >= 0.5f || m_ImageMatrix.a == 0) ||
      (fabs(m_ImageMatrix.c) >= 0.5f || m_ImageMatrix.d == 0)) {
    RetainPtr<CFX_DIBBase> pDib = m_pDIBBase;
    if (m_pDIBBase->IsAlphaFormat() &&
        !(m_pDevice->GetRenderCaps() & FXRC_ALPHA_IMAGE) &&
        !(m_pDevice->GetRenderCaps() & FXRC_GET_BITS)) {
      m_pCloneConvert = m_pDIBBase->ConvertTo(FXDIB_Format::kRgb);
      if (!m_pCloneConvert)
        return false;

      pDib = m_pCloneConvert;
    }
    FX_RECT clip_box = m_pDevice->GetClipBox();
    clip_box.Intersect(image_rect);
    m_State = State::kTransforming;
    m_pTransformer = std::make_unique<CFX_ImageTransformer>(pDib, m_ImageMatrix,
                                                            options, &clip_box);
    return true;
  }
  if (m_ImageMatrix.a < 0)
    dest_width = -dest_width;
  if (m_ImageMatrix.d > 0)
    dest_height = -dest_height;
  int dest_left = dest_width > 0 ? image_rect.left : image_rect.right;
  int dest_top = dest_height > 0 ? image_rect.top : image_rect.bottom;
  if (m_pDIBBase->IsOpaqueImage()) {
    if (m_pDevice->StretchDIBitsWithFlagsAndBlend(
            m_pDIBBase, dest_left, dest_top, dest_width, dest_height, options,
            BlendMode::kNormal)) {
      return false;
    }
  }
  if (m_pDIBBase->IsMaskFormat()) {
    if (m_pDevice->StretchBitMaskWithFlags(m_pDIBBase, dest_left, dest_top,
                                           dest_width, dest_height, 0,
                                           options)) {
      return false;
    }
  }

  FX_RECT clip_box = m_pDevice->GetClipBox();
  FX_RECT dest_rect = clip_box;
  dest_rect.Intersect(image_rect);
  FX_RECT dest_clip(
      dest_rect.left - image_rect.left, dest_rect.top - image_rect.top,
      dest_rect.right - image_rect.left, dest_rect.bottom - image_rect.top);
  RetainPtr<CFX_DIBitmap> pStretched =
      m_pDIBBase->StretchTo(dest_width, dest_height, options, &dest_clip);
  if (pStretched) {
    CompositeDIBitmap(std::move(pStretched), dest_rect.left, dest_rect.top);
  }

  return false;
}

bool CXFA_ImageRenderer::Continue() {
  if (m_State == State::kTransforming) {
    if (m_pTransformer->Continue(nullptr))
      return true;

    RetainPtr<CFX_DIBitmap> bitmap = m_pTransformer->DetachBitmap();
    if (!bitmap) {
      return false;
    }

    if (bitmap->IsMaskFormat()) {
      m_pDevice->SetBitMask(std::move(bitmap), m_pTransformer->result().left,
                            m_pTransformer->result().top, 0);
    } else {
      m_pDevice->SetDIBitsWithBlend(
          std::move(bitmap), m_pTransformer->result().left,
          m_pTransformer->result().top, BlendMode::kNormal);
    }
    return false;
  }
  if (m_State == State::kStarted)
    return m_pDevice->ContinueDIBits(m_DeviceHandle.get(), nullptr);

  return false;
}

void CXFA_ImageRenderer::CompositeDIBitmap(RetainPtr<CFX_DIBitmap> bitmap,
                                           int left,
                                           int top) {
  CHECK(bitmap);

  if (!bitmap->IsMaskFormat()) {
    if (m_pDevice->SetDIBits(std::move(bitmap), left, top)) {
      return;
    }
  } else if (m_pDevice->SetBitMask(bitmap, left, top, 0)) {
    return;
  }

  bool bGetBackGround = ((m_pDevice->GetRenderCaps() & FXRC_ALPHA_OUTPUT)) ||
                        (!(m_pDevice->GetRenderCaps() & FXRC_ALPHA_OUTPUT) &&
                         (m_pDevice->GetRenderCaps() & FXRC_GET_BITS));
  if (bGetBackGround) {
    if (bitmap->IsMaskFormat()) {
      return;
    }

    m_pDevice->SetDIBitsWithBlend(std::move(bitmap), left, top,
                                  BlendMode::kNormal);
    return;
  }
  if (!bitmap->IsAlphaFormat() ||
      (m_pDevice->GetRenderCaps() & FXRC_ALPHA_IMAGE)) {
    return;
  }

  bitmap = bitmap->ConvertTo(FXDIB_Format::kRgb);
  if (!bitmap) {
    return;
  }

  CXFA_ImageRenderer image_renderer(m_pDevice, std::move(bitmap),
                                    m_ImageMatrix);
  if (!image_renderer.Start()) {
    return;
  }

  while (image_renderer.Continue()) {
    continue;
  }
}
