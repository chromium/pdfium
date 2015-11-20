// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_progressive.h"

#include "fpdfsdk/include/fsdk_define.h"
#include "fpdfsdk/include/fsdk_rendercontext.h"
#include "public/fpdfview.h"

// These checks are here because core/ and public/ cannot depend on each other.
static_assert(CPDF_ProgressiveRenderer::Ready == FPDF_RENDER_READER,
              "CPDF_ProgressiveRenderer::Ready value mismatch");
static_assert(CPDF_ProgressiveRenderer::ToBeContinued ==
                  FPDF_RENDER_TOBECOUNTINUED,
              "CPDF_ProgressiveRenderer::ToBeContinued value mismatch");
static_assert(CPDF_ProgressiveRenderer::Done == FPDF_RENDER_DONE,
              "CPDF_ProgressiveRenderer::Done value mismatch");
static_assert(CPDF_ProgressiveRenderer::Failed == FPDF_RENDER_FAILED,
              "CPDF_ProgressiveRenderer::Failed value mismatch");

DLLEXPORT int STDCALL FPDF_RenderPageBitmap_Start(FPDF_BITMAP bitmap,
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

  CRenderContext* pContext = new CRenderContext;
  pPage->SetPrivateData((void*)1, pContext, DropContext);
#ifdef _SKIA_SUPPORT_
  pContext->m_pDevice = new CFX_SkiaDevice;
  if (flags & FPDF_REVERSE_BYTE_ORDER)
    ((CFX_SkiaDevice*)pContext->m_pDevice)
        ->Attach((CFX_DIBitmap*)bitmap, 0, TRUE);
  else
    ((CFX_SkiaDevice*)pContext->m_pDevice)->Attach((CFX_DIBitmap*)bitmap);
#else
  pContext->m_pDevice = new CFX_FxgeDevice;
  if (flags & FPDF_REVERSE_BYTE_ORDER)
    ((CFX_FxgeDevice*)pContext->m_pDevice)
        ->Attach((CFX_DIBitmap*)bitmap, 0, TRUE);
  else
    ((CFX_FxgeDevice*)pContext->m_pDevice)->Attach((CFX_DIBitmap*)bitmap);
#endif
  IFSDK_PAUSE_Adapter IPauseAdapter(pause);

  FPDF_RenderPage_Retail(pContext, page, start_x, start_y, size_x, size_y,
                         rotate, flags, FALSE, &IPauseAdapter);

  if (pContext->m_pRenderer) {
    return CPDF_ProgressiveRenderer::ToFPDFStatus(
        pContext->m_pRenderer->GetStatus());
  }
  return FPDF_RENDER_FAILED;
}

DLLEXPORT int STDCALL FPDF_RenderPage_Continue(FPDF_PAGE page,
                                               IFSDK_PAUSE* pause) {
  if (!pause || pause->version != 1)
    return FPDF_RENDER_FAILED;

  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return FPDF_RENDER_FAILED;

  CRenderContext* pContext = (CRenderContext*)pPage->GetPrivateData((void*)1);
  if (pContext && pContext->m_pRenderer) {
    IFSDK_PAUSE_Adapter IPauseAdapter(pause);
    pContext->m_pRenderer->Continue(&IPauseAdapter);
    return CPDF_ProgressiveRenderer::ToFPDFStatus(
        pContext->m_pRenderer->GetStatus());
  }
  return FPDF_RENDER_FAILED;
}

DLLEXPORT void STDCALL FPDF_RenderPage_Close(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return;

  CRenderContext* pContext = (CRenderContext*)pPage->GetPrivateData((void*)1);
  if (!pContext)
    return;

  pContext->m_pDevice->RestoreState();
  delete pContext;
  pPage->RemovePrivateData((void*)1);
}
