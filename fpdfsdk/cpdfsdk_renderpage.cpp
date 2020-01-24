// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_renderpage.h"

#include <utility>

#include "core/fpdfapi/render/cpdf_pagerendercache.h"
#include "core/fpdfapi/render/cpdf_pagerendercontext.h"
#include "core/fpdfapi/render/cpdf_progressiverenderer.h"
#include "core/fpdfapi/render/cpdf_renderoptions.h"
#include "core/fpdfdoc/cpdf_annotlist.h"
#include "core/fxge/cfx_renderdevice.h"
#include "fpdfsdk/cpdfsdk_pauseadapter.h"
#include "public/fpdfview.h"
#include "third_party/base/ptr_util.h"

namespace {

void RenderPageImpl(CPDF_PageRenderContext* pContext,
                    CPDF_Page* pPage,
                    const CFX_Matrix& matrix,
                    const FX_RECT& clipping_rect,
                    int flags,
                    bool need_to_restore,
                    CPDFSDK_PauseAdapter* pause) {
  if (!pContext->m_pOptions)
    pContext->m_pOptions = pdfium::MakeUnique<CPDF_RenderOptions>();

  auto& options = pContext->m_pOptions->GetOptions();
  options.bClearType = !!(flags & FPDF_LCD_TEXT);
  options.bNoNativeText = !!(flags & FPDF_NO_NATIVETEXT);
  options.bLimitedImageCache = !!(flags & FPDF_RENDER_LIMITEDIMAGECACHE);
  options.bForceHalftone = !!(flags & FPDF_RENDER_FORCEHALFTONE);
  options.bNoTextSmooth = !!(flags & FPDF_RENDER_NO_SMOOTHTEXT);
  options.bNoImageSmooth = !!(flags & FPDF_RENDER_NO_SMOOTHIMAGE);
  options.bNoPathSmooth = !!(flags & FPDF_RENDER_NO_SMOOTHPATH);

  // Grayscale output
  if (flags & FPDF_GRAYSCALE)
    pContext->m_pOptions->SetColorMode(CPDF_RenderOptions::kGray);

  const CPDF_OCContext::UsageType usage =
      (flags & FPDF_PRINTING) ? CPDF_OCContext::Print : CPDF_OCContext::View;
  pContext->m_pOptions->SetOCContext(
      pdfium::MakeRetain<CPDF_OCContext>(pPage->GetDocument(), usage));

  pContext->m_pDevice->SaveState();
  pContext->m_pDevice->SetBaseClip(clipping_rect);
  pContext->m_pDevice->SetClip_Rect(clipping_rect);
  pContext->m_pContext = pdfium::MakeUnique<CPDF_RenderContext>(
      pPage->GetDocument(), pPage->m_pPageResources.Get(),
      static_cast<CPDF_PageRenderCache*>(pPage->GetRenderCache()));

  pContext->m_pContext->AppendLayer(pPage, &matrix);

  if (flags & FPDF_ANNOT) {
    auto pOwnedList = pdfium::MakeUnique<CPDF_AnnotList>(pPage);
    CPDF_AnnotList* pList = pOwnedList.get();
    pContext->m_pAnnots = std::move(pOwnedList);
    bool bPrinting =
        pContext->m_pDevice->GetDeviceType() != DeviceType::kDisplay;
    pList->DisplayAnnots(pPage, pContext->m_pContext.get(), bPrinting, &matrix,
                         false, nullptr);
  }

  pContext->m_pRenderer = pdfium::MakeUnique<CPDF_ProgressiveRenderer>(
      pContext->m_pContext.get(), pContext->m_pDevice.get(),
      pContext->m_pOptions.get());
  pContext->m_pRenderer->Start(pause);
  if (need_to_restore)
    pContext->m_pDevice->RestoreState(false);
}

}  // namespace

void CPDFSDK_RenderPage(CPDF_PageRenderContext* pContext,
                        CPDF_Page* pPage,
                        const CFX_Matrix& matrix,
                        const FX_RECT& clipping_rect,
                        int flags) {
  RenderPageImpl(pContext, pPage, matrix, clipping_rect, flags,
                 /*need_to_restore=*/true, /*pause=*/nullptr);
}

void CPDFSDK_RenderPageWithContext(CPDF_PageRenderContext* pContext,
                                   CPDF_Page* pPage,
                                   int start_x,
                                   int start_y,
                                   int size_x,
                                   int size_y,
                                   int rotate,
                                   int flags,
                                   bool need_to_restore,
                                   CPDFSDK_PauseAdapter* pause) {
  const FX_RECT rect(start_x, start_y, start_x + size_x, start_y + size_y);
  RenderPageImpl(pContext, pPage, pPage->GetDisplayMatrix(rect, rotate), rect,
                 flags, need_to_restore, pause);
}
