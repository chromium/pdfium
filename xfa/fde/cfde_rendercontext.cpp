// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/cfde_rendercontext.h"

#include <vector>

#include "third_party/base/logging.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fde/cfde_renderdevice.h"
#include "xfa/fde/cfde_txtedtpage.h"
#include "xfa/fde/cfde_txtedttextset.h"

CFDE_RenderContext::CFDE_RenderContext(CFDE_RenderDevice* pRenderDevice)
    : m_pRenderDevice(pRenderDevice) {}

CFDE_RenderContext::~CFDE_RenderContext() {}

void CFDE_RenderContext::Render(CFDE_TxtEdtPage* pCanvasSet,
                                const CFX_Matrix& tmDoc2Device) {
  if (!m_pRenderDevice || !pCanvasSet)
    return;

  CFDE_TxtEdtTextSet* pVisualSet = pCanvasSet->GetTextSet();
  if (!pVisualSet)
    return;

  CFX_RetainPtr<CFGAS_GEFont> pFont = pVisualSet->GetFont();
  if (!pFont)
    return;

  CFX_RectF rtDocClip = m_pRenderDevice->GetClipRect();
  if (rtDocClip.IsEmpty()) {
    rtDocClip.left = rtDocClip.top = 0;
    rtDocClip.width = static_cast<float>(m_pRenderDevice->GetWidth());
    rtDocClip.height = static_cast<float>(m_pRenderDevice->GetHeight());
  }
  tmDoc2Device.GetInverse().TransformRect(rtDocClip);

  std::vector<FXTEXT_CHARPOS> char_pos;

  for (size_t i = 0; i < pCanvasSet->GetTextPieceCount(); ++i) {
    const FDE_TEXTEDITPIECE& pText = pCanvasSet->GetTextPiece(i);
    if (!rtDocClip.IntersectWith(pVisualSet->GetRect(pText)))
      continue;

    int32_t iCount = pVisualSet->GetDisplayPos(pText, nullptr, false);
    if (iCount < 1)
      continue;
    if (char_pos.size() < static_cast<size_t>(iCount))
      char_pos.resize(iCount, FXTEXT_CHARPOS());

    iCount = pVisualSet->GetDisplayPos(pText, char_pos.data(), false);
    m_pRenderDevice->DrawString(pVisualSet->GetFontColor(), pFont,
                                char_pos.data(), iCount,
                                pVisualSet->GetFontSize(), &tmDoc2Device);
  }
}
