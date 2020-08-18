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
#include "fpdfsdk/fpdfxfa/cpdfxfa_widget.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_ffwidgethandler.h"
#include "xfa/fxgraphics/cxfa_graphics.h"

namespace {

constexpr uint32_t kIteratorFilter = XFA_WidgetStatus_Visible |
                                     XFA_WidgetStatus_Viewable |
                                     XFA_WidgetStatus_Focused;

}  // namespace

CPDFXFA_Page::CPDFXFA_Page(CPDF_Document* pDocument, int page_index)
    : m_pDocument(pDocument), m_iPageIndex(page_index) {
  ASSERT(m_pDocument->GetExtension());
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
  return m_pDocument.Get();
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
  auto* pContext = static_cast<CPDFXFA_Context*>(m_pDocument->GetExtension());
  CXFA_FFDocView* pXFADocView = pContext->GetXFADocView();
  return pXFADocView ? pXFADocView->GetPageView(m_iPageIndex) : nullptr;
}

bool CPDFXFA_Page::LoadPage() {
  auto* pContext = static_cast<CPDFXFA_Context*>(m_pDocument->GetExtension());
  switch (pContext->GetFormType()) {
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
      std::make_unique<CPDF_PageRenderCache>(m_pPDFPage.Get()));
  m_pPDFPage->ParseContent();
}

float CPDFXFA_Page::GetPageWidth() const {
  CXFA_FFPageView* pPageView = GetXFAPageView();
  if (!m_pPDFPage && !pPageView)
    return 0.0f;

  auto* pContext = static_cast<CPDFXFA_Context*>(m_pDocument->GetExtension());
  switch (pContext->GetFormType()) {
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

  auto* pContext = static_cast<CPDFXFA_Context*>(m_pDocument->GetExtension());
  switch (pContext->GetFormType()) {
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

  auto* pContext = static_cast<CPDFXFA_Context*>(m_pDocument->GetExtension());
  switch (pContext->GetFormType()) {
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
  CPDFXFA_Widget* pXFAWidget = ToXFAWidget(pSDKAnnot);
  if (!pXFAWidget)
    return nullptr;

  CXFA_FFPageView* xfa_page_view = GetXFAPageView();
  if (!xfa_page_view)
    return nullptr;

  ObservedPtr<CPDFSDK_Annot> pObservedAnnot(pSDKAnnot);
  CPDFSDK_PageView* pPageView = pSDKAnnot->GetPageView();
  IXFA_WidgetIterator* pWidgetIterator =
      xfa_page_view->CreateGCedTraverseWidgetIterator(kIteratorFilter);

  // Check |pSDKAnnot| again because JS may have destroyed it
  if (!pObservedAnnot)
    return nullptr;

  if (pWidgetIterator->GetCurrentWidget() != pXFAWidget->GetXFAFFWidget())
    pWidgetIterator->SetCurrentWidget(pXFAWidget->GetXFAFFWidget());

  CXFA_FFWidget* hNextFocus =
      bNext ? pWidgetIterator->MoveToNext() : pWidgetIterator->MoveToPrevious();
  if (!hNextFocus)
    return nullptr;

  return pPageView->GetAnnotByXFAWidget(hNextFocus);
}

CPDFSDK_Annot* CPDFXFA_Page::GetFirstOrLastXFAAnnot(CPDFSDK_PageView* page_view,
                                                    bool last) const {
  CXFA_FFPageView* xfa_page_view = GetXFAPageView();
  if (!xfa_page_view)
    return nullptr;

  ObservedPtr<CPDFSDK_PageView> watched_page_view(page_view);
  IXFA_WidgetIterator* it =
      xfa_page_view->CreateGCedTraverseWidgetIterator(kIteratorFilter);
  if (!watched_page_view)
    return nullptr;

  CXFA_FFWidget* pWidget = last ? it->MoveToLast() : it->MoveToFirst();
  return watched_page_view->GetAnnotByXFAWidget(pWidget);
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

  IXFA_WidgetIterator* pWidgetIterator =
      pPageView->CreateGCedFormWidgetIterator(XFA_WidgetStatus_Viewable);

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
  IXFA_WidgetIterator* pWidgetIterator = xfaView->CreateGCedFormWidgetIterator(
      XFA_WidgetStatus_Visible | XFA_WidgetStatus_Viewable);

  while (1) {
    CXFA_FFWidget* pWidget = pWidgetIterator->MoveToNext();
    if (!pWidget)
      break;

    CFX_RectF rtWidgetBox = pWidget->GetBBox(CXFA_FFWidget::kDoNotDrawFocus);
    ++rtWidgetBox.width;
    ++rtWidgetBox.height;
    if (rtWidgetBox.IntersectWith(rectClip))
      pWidget->RenderWidget(&gs, mtUser2Device, CXFA_FFWidget::kHighlight);
  }

  CPDFXFA_Widget* pXFAWidget = ToXFAWidget(pAnnot);
  if (!pXFAWidget)
    return;

  CXFA_FFDocView* docView = xfaView->GetDocView();
  if (!docView)
    return;

  docView->GetWidgetHandler()->RenderWidget(pXFAWidget->GetXFAFFWidget(), &gs,
                                            mtUser2Device, false);
}
