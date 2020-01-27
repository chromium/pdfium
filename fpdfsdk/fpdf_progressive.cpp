// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_progressive.h"

#include <utility>

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/render/cpdf_pagerendercontext.h"
#include "core/fpdfapi/render/cpdf_progressiverenderer.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/cpdfsdk_pauseadapter.h"
#include "fpdfsdk/cpdfsdk_renderpage.h"
#include "public/fpdfview.h"
#include "third_party/base/ptr_util.h"

#ifdef _SKIA_SUPPORT_PATHS_
#include "core/fxge/cfx_renderdevice.h"
#endif

// These checks are here because core/ and public/ cannot depend on each other.
static_assert(CPDF_ProgressiveRenderer::kReady == FPDF_RENDER_READY,
              "CPDF_ProgressiveRenderer::kReady value mismatch");
static_assert(CPDF_ProgressiveRenderer::kToBeContinued ==
                  FPDF_RENDER_TOBECONTINUED,
              "CPDF_ProgressiveRenderer::kToBeContinued value mismatch");
static_assert(CPDF_ProgressiveRenderer::kDone == FPDF_RENDER_DONE,
              "CPDF_ProgressiveRenderer::kDone value mismatch");
static_assert(CPDF_ProgressiveRenderer::kFailed == FPDF_RENDER_FAILED,
              "CPDF_ProgressiveRenderer::kFailed value mismatch");

namespace {

int ToFPDFStatus(CPDF_ProgressiveRenderer::Status status) {
  return static_cast<int>(status);
}

}  // namespace

FPDF_EXPORT int FPDF_CALLCONV FPDF_RenderPageBitmap_Start(FPDF_BITMAP bitmap,
                                                          FPDF_PAGE page,
                                                          int start_x,
                                                          int start_y,
                                                          int size_x,
                                                          int size_y,
                                                          int rotate,
                                                          int flags,
                                                          IFSDK_PAUSE* pause) {
  if (!bitmap || !pause || pause->version != 1)
    return FPDF_RENDER_FAILED;

  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return FPDF_RENDER_FAILED;

  auto pOwnedContext = pdfium::MakeUnique<CPDF_PageRenderContext>();
  CPDF_PageRenderContext* pContext = pOwnedContext.get();
  pPage->SetRenderContext(std::move(pOwnedContext));

  RetainPtr<CFX_DIBitmap> pBitmap(CFXDIBitmapFromFPDFBitmap(bitmap));
  auto pOwnedDevice = pdfium::MakeUnique<CFX_DefaultRenderDevice>();
  CFX_DefaultRenderDevice* pDevice = pOwnedDevice.get();
  pContext->m_pDevice = std::move(pOwnedDevice);
  pDevice->Attach(pBitmap, !!(flags & FPDF_REVERSE_BYTE_ORDER), nullptr, false);

  CPDFSDK_PauseAdapter pause_adapter(pause);
  CPDFSDK_RenderPageWithContext(pContext, pPage, start_x, start_y, size_x,
                                size_y, rotate, flags,
                                /*need_to_restore=*/false, &pause_adapter);

#ifdef _SKIA_SUPPORT_PATHS_
  pDevice->Flush(false);
  pBitmap->UnPreMultiply();
#endif

  if (!pContext->m_pRenderer)
    return FPDF_RENDER_FAILED;

  return ToFPDFStatus(pContext->m_pRenderer->GetStatus());
}

FPDF_EXPORT int FPDF_CALLCONV FPDF_RenderPage_Continue(FPDF_PAGE page,
                                                       IFSDK_PAUSE* pause) {
  if (!pause || pause->version != 1)
    return FPDF_RENDER_FAILED;

  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return FPDF_RENDER_FAILED;

  auto* pContext =
      static_cast<CPDF_PageRenderContext*>(pPage->GetRenderContext());
  if (!pContext || !pContext->m_pRenderer)
    return FPDF_RENDER_FAILED;

  CPDFSDK_PauseAdapter pause_adapter(pause);
  pContext->m_pRenderer->Continue(&pause_adapter);
#ifdef _SKIA_SUPPORT_PATHS_
  CFX_RenderDevice* pDevice = pContext->m_pDevice.get();
  pDevice->Flush(false);
  pDevice->GetBitmap()->UnPreMultiply();
#endif
  return ToFPDFStatus(pContext->m_pRenderer->GetStatus());
}

FPDF_EXPORT void FPDF_CALLCONV FPDF_RenderPage_Close(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (pPage) {
#ifdef _SKIA_SUPPORT_PATHS_
    auto* pContext =
        static_cast<CPDF_PageRenderContext*>(pPage->GetRenderContext());
    if (pContext && pContext->m_pRenderer) {
      CFX_RenderDevice* pDevice = pContext->m_pDevice.get();
      pDevice->Flush(true);
      pDevice->GetBitmap()->UnPreMultiply();
    }
#endif
    pPage->SetRenderContext(nullptr);
  }
}
