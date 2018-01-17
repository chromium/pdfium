// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_imagerenderer.h"

#include "core/fpdfapi/page/cpdf_pageobjectholder.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/dib/cfx_dibsource.h"
#include "core/fxge/dib/cfx_imagerenderer.h"
#include "core/fxge/dib/cfx_imagetransformer.h"
#include "third_party/base/ptr_util.h"

CXFA_ImageRenderer::CXFA_ImageRenderer(
    CFX_RenderDevice* pDevice,
    const RetainPtr<CFX_DIBSource>& pDIBSource,
    const CFX_Matrix* pImage2Device,
    uint32_t flags)
    : m_pDevice(pDevice),
      m_ImageMatrix(*pImage2Device),
      m_pDIBSource(pDIBSource),
      m_Flags(flags) {}

CXFA_ImageRenderer::~CXFA_ImageRenderer() {}

bool CXFA_ImageRenderer::Start() {
  if (m_pDevice->StartDIBitsWithBlend(m_pDIBSource, 255, 0, &m_ImageMatrix,
                                      m_Flags, &m_DeviceHandle,
                                      FXDIB_BLEND_NORMAL)) {
    if (m_DeviceHandle) {
      m_Status = 3;
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
    RetainPtr<CFX_DIBSource> pDib = m_pDIBSource;
    if (m_pDIBSource->HasAlpha() &&
        !(m_pDevice->GetRenderCaps() & FXRC_ALPHA_IMAGE) &&
        !(m_pDevice->GetRenderCaps() & FXRC_GET_BITS)) {
      m_pCloneConvert = m_pDIBSource->CloneConvert(FXDIB_Rgb);
      if (!m_pCloneConvert)
        return false;

      pDib = m_pCloneConvert;
    }
    FX_RECT clip_box = m_pDevice->GetClipBox();
    clip_box.Intersect(image_rect);
    m_Status = 2;
    m_pTransformer = pdfium::MakeUnique<CFX_ImageTransformer>(
        pDib, &m_ImageMatrix, m_Flags, &clip_box);
    return true;
  }
  if (m_ImageMatrix.a < 0)
    dest_width = -dest_width;
  if (m_ImageMatrix.d > 0)
    dest_height = -dest_height;
  int dest_left, dest_top;
  dest_left = dest_width > 0 ? image_rect.left : image_rect.right;
  dest_top = dest_height > 0 ? image_rect.top : image_rect.bottom;
  if (m_pDIBSource->IsOpaqueImage()) {
    if (m_pDevice->StretchDIBitsWithFlagsAndBlend(
            m_pDIBSource, dest_left, dest_top, dest_width, dest_height, m_Flags,
            FXDIB_BLEND_NORMAL)) {
      return false;
    }
  }
  if (m_pDIBSource->IsAlphaMask()) {
    if (m_pDevice->StretchBitMaskWithFlags(m_pDIBSource, dest_left, dest_top,
                                           dest_width, dest_height, 0,
                                           m_Flags)) {
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
      m_pDIBSource->StretchTo(dest_width, dest_height, m_Flags, &dest_clip);
  if (pStretched) {
    CompositeDIBitmap(pStretched, dest_rect.left, dest_rect.top, false);
  }
  return false;
}

bool CXFA_ImageRenderer::Continue() {
  if (m_Status == 2) {
    if (m_pTransformer->Continue(nullptr))
      return true;

    RetainPtr<CFX_DIBitmap> pBitmap = m_pTransformer->DetachBitmap();
    if (!pBitmap)
      return false;

    if (pBitmap->IsAlphaMask()) {
      m_pDevice->SetBitMask(pBitmap, m_pTransformer->result().left,
                            m_pTransformer->result().top, 0);
    } else {
      m_pDevice->SetDIBitsWithBlend(pBitmap, m_pTransformer->result().left,
                                    m_pTransformer->result().top,
                                    FXDIB_BLEND_NORMAL);
    }
    return false;
  }
  if (m_Status == 3)
    return m_pDevice->ContinueDIBits(m_DeviceHandle.get(), nullptr);

  return false;
}

void CXFA_ImageRenderer::CompositeDIBitmap(
    const RetainPtr<CFX_DIBitmap>& pDIBitmap,
    int left,
    int top,
    int iTransparency) {
  if (!pDIBitmap)
    return;

  bool bIsolated = !!(iTransparency & PDFTRANS_ISOLATED);
  bool bGroup = !!(iTransparency & PDFTRANS_GROUP);
  if (!pDIBitmap->IsAlphaMask()) {
    if (m_pDevice->SetDIBits(pDIBitmap, left, top))
      return;
  } else {
    uint32_t fill_argb = 0;
    if (m_pDevice->SetBitMask(pDIBitmap, left, top, fill_argb))
      return;
  }

  bool bGetBackGround = ((m_pDevice->GetRenderCaps() & FXRC_ALPHA_OUTPUT)) ||
                        (!(m_pDevice->GetRenderCaps() & FXRC_ALPHA_OUTPUT) &&
                         (m_pDevice->GetRenderCaps() & FXRC_GET_BITS));
  if (bGetBackGround) {
    if (bIsolated || !bGroup) {
      if (pDIBitmap->IsAlphaMask())
        return;

      m_pDevice->SetDIBitsWithBlend(pDIBitmap, left, top, FXDIB_BLEND_NORMAL);
    } else {
      FX_RECT rect(left, top, left + pDIBitmap->GetWidth(),
                   top + pDIBitmap->GetHeight());
      rect.Intersect(m_pDevice->GetClipBox());
      RetainPtr<CFX_DIBitmap> pClone;
      if (m_pDevice->GetBackDrop() && m_pDevice->GetBitmap()) {
        pClone = m_pDevice->GetBackDrop()->Clone(&rect);
        RetainPtr<CFX_DIBitmap> pForeBitmap = m_pDevice->GetBitmap();
        pClone->CompositeBitmap(0, 0, pClone->GetWidth(), pClone->GetHeight(),
                                pForeBitmap, rect.left, rect.top);
        left = left >= 0 ? 0 : left;
        top = top >= 0 ? 0 : top;
        if (!pDIBitmap->IsAlphaMask())
          pClone->CompositeBitmap(0, 0, pClone->GetWidth(), pClone->GetHeight(),
                                  pDIBitmap, left, top, FXDIB_BLEND_NORMAL);
        else
          pClone->CompositeMask(0, 0, pClone->GetWidth(), pClone->GetHeight(),
                                pDIBitmap, 0, left, top, FXDIB_BLEND_NORMAL);
      } else {
        pClone = pDIBitmap;
      }
      if (m_pDevice->GetBackDrop()) {
        m_pDevice->SetDIBits(pClone, rect.left, rect.top);
      } else {
        if (pDIBitmap->IsAlphaMask())
          return;
        m_pDevice->SetDIBitsWithBlend(pDIBitmap, rect.left, rect.top,
                                      FXDIB_BLEND_NORMAL);
      }
    }
    return;
  }
  if (!pDIBitmap->HasAlpha() ||
      (m_pDevice->GetRenderCaps() & FXRC_ALPHA_IMAGE)) {
    return;
  }
  RetainPtr<CFX_DIBitmap> pCloneConvert = pDIBitmap->CloneConvert(FXDIB_Rgb);
  if (!pCloneConvert)
    return;

  CXFA_ImageRenderer imageRender(m_pDevice, pCloneConvert, &m_ImageMatrix,
                                 m_Flags);
  if (!imageRender.Start()) {
    return;
  }
  while (imageRender.Continue())
    continue;
}
