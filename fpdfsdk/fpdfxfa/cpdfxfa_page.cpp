// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/fpdfxfa/cpdfxfa_page.h"

#include <memory>

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/render/cpdf_pagerendercache.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/fpdfxfa/cpdfxfa_context.h"
#include "fpdfsdk/fpdfxfa/cxfa_fwladaptertimermgr.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_ffwidgethandler.h"
#include "xfa/fxfa/cxfa_rendercontext.h"
#include "xfa/fxgraphics/cxfa_graphics.h"

CPDFXFA_Page::CPDFXFA_Page(CPDFXFA_Context* pContext, int page_index)
    : m_pContext(pContext), m_iPageIndex(page_index) {
  ASSERT(m_pContext);
  ASSERT(m_iPageIndex >= 0);
}

CPDFXFA_Page::~CPDFXFA_Page() = default;

CPDF_Page* CPDFXFA_Page::AsPDFPage() {
  return m_pPDFPage.Get();
}

CPDFXFA_Page* CPDFXFA_Page::AsXFAPage() {
  return this;
}

CPDF_Document* CPDFXFA_Page::GetDocument() const {
  return m_pContext->GetPDFDoc();
}

bool CPDFXFA_Page::LoadPDFPage() {
  CPDF_Document* pPDFDoc = GetDocument();
  CPDF_Dictionary* pDict = pPDFDoc->GetPageDictionary(m_iPageIndex);
  if (!pDict)
    return false;

  if (!m_pPDFPage || m_pPDFPage->GetDict() != pDict)
    LoadPDFPageFromDict(pDict);

  return true;
}

CXFA_FFPageView* CPDFXFA_Page::GetXFAPageView() const {
  CXFA_FFDocView* pXFADocView = m_pContext->GetXFADocView();
  return pXFADocView ? pXFADocView->GetPageView(m_iPageIndex) : nullptr;
}

bool CPDFXFA_Page::LoadPage() {
  switch (m_pContext->GetFormType()) {
    case FormType::kNone:
    case FormType::kAcroForm:
    case FormType::kXFAForeground:
      return LoadPDFPage();
    case FormType::kXFAFull:
      return !!GetXFAPageView();
  }
  NOTREACHED();
  return false;
}

void CPDFXFA_Page::LoadPDFPageFromDict(CPDF_Dictionary* pPageDict) {
  ASSERT(pPageDict);
  m_pPDFPage = pdfium::MakeRetain<CPDF_Page>(GetDocument(), pPageDict);
  m_pPDFPage->SetRenderCache(
      pdfium::MakeUnique<CPDF_PageRenderCache>(m_pPDFPage.Get()));
  m_pPDFPage->ParseContent();
}

CPDF_Document::Extension* CPDFXFA_Page::GetDocumentExtension() const {
  return m_pContext.Get();
}

float CPDFXFA_Page::GetPageWidth() const {
  CXFA_FFPageView* pPageView = GetXFAPageView();
  if (!m_pPDFPage && !pPageView)
    return 0.0f;

  switch (m_pContext->GetFormType()) {
    case FormType::kNone:
    case FormType::kAcroForm:
    case FormType::kXFAForeground:
      if (m_pPDFPage)
        return m_pPDFPage->GetPageWidth();
      FALLTHROUGH;
    case FormType::kXFAFull:
      if (pPageView)
        return pPageView->GetPageViewRect().width;
      break;
  }

  return 0.0f;
}

float CPDFXFA_Page::GetPageHeight() const {
  CXFA_FFPageView* pPageView = GetXFAPageView();
  if (!m_pPDFPage && !pPageView)
    return 0.0f;

  switch (m_pContext->GetFormType()) {
    case FormType::kNone:
    case FormType::kAcroForm:
    case FormType::kXFAForeground:
      if (m_pPDFPage)
        return m_pPDFPage->GetPageHeight();
      FALLTHROUGH;
    case FormType::kXFAFull:
      if (pPageView)
        return pPageView->GetPageViewRect().height;
      break;
  }

  return 0.0f;
}

Optional<CFX_PointF> CPDFXFA_Page::DeviceToPage(
    const FX_RECT& rect,
    int rotate,
    const CFX_PointF& device_point) const {
  CXFA_FFPageView* pPageView = GetXFAPageView();
  if (!m_pPDFPage && !pPageView)
    return {};

  CFX_PointF pos =
      GetDisplayMatrix(rect, rotate).GetInverse().Transform(device_point);
  return pos;
}

Optional<CFX_PointF> CPDFXFA_Page::PageToDevice(
    const FX_RECT& rect,
    int rotate,
    const CFX_PointF& page_point) const {
  CXFA_FFPageView* pPageView = GetXFAPageView();
  if (!m_pPDFPage && !pPageView)
    return {};

  CFX_Matrix page2device = GetDisplayMatrix(rect, rotate);
  return page2device.Transform(page_point);
}

CFX_Matrix CPDFXFA_Page::GetDisplayMatrix(const FX_RECT& rect,
                                          int iRotate) const {
  CXFA_FFPageView* pPageView = GetXFAPageView();
  if (!m_pPDFPage && !pPageView)
    return CFX_Matrix();

  switch (m_pContext->GetFormType()) {
    case FormType::kNone:
    case FormType::kAcroForm:
    case FormType::kXFAForeground:
      if (m_pPDFPage)
        return m_pPDFPage->GetDisplayMatrix(rect, iRotate);
      FALLTHROUGH;
    case FormType::kXFAFull:
      if (pPageView)
        return pPageView->GetDisplayMatrix(rect, iRotate);
      break;
  }

  return CFX_Matrix();
}

CPDFSDK_Annot* CPDFXFA_Page::GetNextXFAAnnot(CPDFSDK_Annot* pSDKAnnot,
                                             bool bNext) {
  ObservedPtr<CPDFSDK_Annot> pObservedAnnot(pSDKAnnot);
  CPDFSDK_PageView* pPageView = pSDKAnnot->GetPageView();
  std::unique_ptr<IXFA_WidgetIterator> pWidgetIterator(
      GetXFAPageView()->CreateWidgetIterator(XFA_TRAVERSEWAY_Tranvalse,
                                             XFA_WidgetStatus_Visible |
                                                 XFA_WidgetStatus_Viewable |
                                                 XFA_WidgetStatus_Focused));

  // Check |pSDKAnnot| again because JS may have destroyed it
  if (!pObservedAnnot || !pWidgetIterator)
    return nullptr;

  if (pWidgetIterator->GetCurrentWidget() != pSDKAnnot->GetXFAWidget())
    pWidgetIterator->SetCurrentWidget(pSDKAnnot->GetXFAWidget());
  CXFA_FFWidget* hNextFocus =
      bNext ? pWidgetIterator->MoveToNext() : pWidgetIterator->MoveToPrevious();
  if (!hNextFocus && pSDKAnnot)
    hNextFocus = pWidgetIterator->MoveToFirst();

  return pPageView->GetAnnotByXFAWidget(hNextFocus);
}

int CPDFXFA_Page::HasFormFieldAtPoint(const CFX_PointF& point) const {
  CXFA_FFPageView* pPageView = GetXFAPageView();
  if (!pPageView)
    return -1;

  CXFA_FFDocView* pDocView = pPageView->GetDocView();
  if (!pDocView)
    return -1;

  CXFA_FFWidgetHandler* pWidgetHandler = pDocView->GetWidgetHandler();
  if (!pWidgetHandler)
    return -1;

  std::unique_ptr<IXFA_WidgetIterator> pWidgetIterator(
      pPageView->CreateWidgetIterator(XFA_TRAVERSEWAY_Form,
                                      XFA_WidgetStatus_Viewable));
  if (!pWidgetIterator)
    return -1;

  CXFA_FFWidget* pXFAAnnot;
  while ((pXFAAnnot = pWidgetIterator->MoveToNext()) != nullptr) {
    if (pXFAAnnot->GetFormFieldType() == FormFieldType::kXFA)
      continue;

    CFX_FloatRect rcWidget = pXFAAnnot->GetWidgetRect().ToFloatRect();
    rcWidget.Inflate(1.0f, 1.0f);
    if (rcWidget.Contains(point))
      return static_cast<int>(pXFAAnnot->GetFormFieldType());
  }

  return -1;
}

void CPDFXFA_Page::DrawFocusAnnot(CFX_RenderDevice* pDevice,
                                  CPDFSDK_Annot* pAnnot,
                                  const CFX_Matrix& mtUser2Device,
                                  const FX_RECT& rtClip) {
  CFX_RectF rectClip(rtClip);
  CXFA_Graphics gs(pDevice);
  gs.SetClipRect(rectClip);

  CXFA_FFPageView* xfaView = GetXFAPageView();
  CXFA_RenderContext renderContext(xfaView, rectClip, mtUser2Device);
  renderContext.DoRender(&gs);

  if (!pAnnot)
    return;

  CXFA_FFDocView* docView = xfaView->GetDocView();
  if (!docView)
    return;

  docView->GetWidgetHandler()->RenderWidget(pAnnot->GetXFAWidget(), &gs,
                                            mtUser2Device, false);
}
