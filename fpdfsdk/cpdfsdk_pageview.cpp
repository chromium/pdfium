// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/cpdfsdk_pageview.h"

#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fpdfapi/fpdf_render/include/cpdf_renderoptions.h"
#include "core/fpdfdoc/include/cpdf_annotlist.h"
#include "core/fpdfdoc/include/cpdf_interform.h"
#include "fpdfsdk/include/cpdfsdk_annot.h"
#include "fpdfsdk/include/cpdfsdk_annothandlermgr.h"
#include "fpdfsdk/include/cpdfsdk_annotiterator.h"
#include "fpdfsdk/include/cpdfsdk_environment.h"
#include "fpdfsdk/include/cpdfsdk_interform.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_page.h"
#include "xfa/fxfa/include/xfa_ffdocview.h"
#include "xfa/fxfa/include/xfa_ffpageview.h"
#include "xfa/fxfa/include/xfa_ffwidgethandler.h"
#include "xfa/fxfa/include/xfa_rendercontext.h"
#include "xfa/fxgraphics/include/cfx_graphics.h"
#endif  // PDF_ENABLE_XFA

CPDFSDK_PageView::CPDFSDK_PageView(CPDFSDK_Document* pSDKDoc,
                                   UnderlyingPageType* page)
    : m_page(page),
      m_pSDKDoc(pSDKDoc),
      m_CaptureWidget(nullptr),
#ifndef PDF_ENABLE_XFA
      m_bOwnsPage(false),
#endif  // PDF_ENABLE_XFA
      m_bEnterWidget(FALSE),
      m_bExitWidget(FALSE),
      m_bOnWidget(FALSE),
      m_bValid(FALSE),
      m_bLocked(FALSE) {
  CPDFSDK_InterForm* pInterForm = pSDKDoc->GetInterForm();
  if (pInterForm) {
    CPDF_InterForm* pPDFInterForm = pInterForm->GetInterForm();
#ifdef PDF_ENABLE_XFA
    if (page->GetPDFPage())
      pPDFInterForm->FixPageFields(page->GetPDFPage());
#else   // PDF_ENABLE_XFA
    pPDFInterForm->FixPageFields(page);
#endif  // PDF_ENABLE_XFA
  }
#ifndef PDF_ENABLE_XFA
  m_page->SetView(this);
#endif  // PDF_ENABLE_XFA
}

CPDFSDK_PageView::~CPDFSDK_PageView() {
#ifndef PDF_ENABLE_XFA
  // The call to |ReleaseAnnot| can cause the page pointed to by |m_page| to
  // be freed, which will cause issues if we try to cleanup the pageview pointer
  // in |m_page|. So, reset the pageview pointer before doing anything else.
  m_page->SetView(nullptr);
#endif  // PDF_ENABLE_XFA

  CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();
  CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
  for (CPDFSDK_Annot* pAnnot : m_fxAnnotArray)
    pAnnotHandlerMgr->ReleaseAnnot(pAnnot);

  m_fxAnnotArray.clear();
  m_pAnnotList.reset();

#ifndef PDF_ENABLE_XFA
  if (m_bOwnsPage)
    delete m_page;
#endif  // PDF_ENABLE_XFA
}

void CPDFSDK_PageView::PageView_OnDraw(CFX_RenderDevice* pDevice,
                                       CFX_Matrix* pUser2Device,
#ifdef PDF_ENABLE_XFA
                                       CPDF_RenderOptions* pOptions,
                                       const FX_RECT& pClip) {
#else
                                       CPDF_RenderOptions* pOptions) {
#endif  // PDF_ENABLE_XFA
  m_curMatrix = *pUser2Device;
  CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();

#ifdef PDF_ENABLE_XFA
  CPDFXFA_Page* pPage = GetPDFXFAPage();
  if (!pPage)
    return;

  if (pPage->GetDocument()->GetDocType() == DOCTYPE_DYNAMIC_XFA) {
    CFX_Graphics gs;
    gs.Create(pDevice);
    CFX_RectF rectClip;
    rectClip.Set(static_cast<FX_FLOAT>(pClip.left),
                 static_cast<FX_FLOAT>(pClip.top),
                 static_cast<FX_FLOAT>(pClip.Width()),
                 static_cast<FX_FLOAT>(pClip.Height()));
    gs.SetClipRect(rectClip);
    std::unique_ptr<CXFA_RenderContext> pRenderContext(new CXFA_RenderContext);
    CXFA_RenderOptions renderOptions;
    renderOptions.m_bHighlight = TRUE;
    CXFA_FFPageView* xfaView = pPage->GetXFAPageView();
    pRenderContext->StartRender(xfaView, &gs, *pUser2Device, renderOptions);
    pRenderContext->DoRender();
    pRenderContext->StopRender();
    CXFA_FFDocView* docView = xfaView->GetDocView();
    if (!docView)
      return;
    CPDFSDK_Annot* annot = GetFocusAnnot();
    if (!annot)
      return;
    // Render the focus widget
    docView->GetWidgetHandler()->RenderWidget(annot->GetXFAWidget(), &gs,
                                              pUser2Device, FALSE);
    return;
  }
#endif  // PDF_ENABLE_XFA

  // for pdf/static xfa.
  CPDFSDK_AnnotIterator annotIterator(this, true);
  while (CPDFSDK_Annot* pSDKAnnot = annotIterator.Next()) {
    CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
    pAnnotHandlerMgr->Annot_OnDraw(this, pSDKAnnot, pDevice, pUser2Device,
                                   pOptions->m_bDrawAnnots);
  }
}

const CPDF_Annot* CPDFSDK_PageView::GetPDFAnnotAtPoint(FX_FLOAT pageX,
                                                       FX_FLOAT pageY) {
  for (const auto& pAnnot : m_pAnnotList->All()) {
    CFX_FloatRect annotRect = pAnnot->GetRect();
    if (annotRect.Contains(pageX, pageY))
      return pAnnot.get();
  }
  return nullptr;
}

const CPDF_Annot* CPDFSDK_PageView::GetPDFWidgetAtPoint(FX_FLOAT pageX,
                                                        FX_FLOAT pageY) {
  for (const auto& pAnnot : m_pAnnotList->All()) {
    if (pAnnot->GetSubtype() == CPDF_Annot::Subtype::WIDGET) {
      CFX_FloatRect annotRect = pAnnot->GetRect();
      if (annotRect.Contains(pageX, pageY))
        return pAnnot.get();
    }
  }
  return nullptr;
}

CPDFSDK_Annot* CPDFSDK_PageView::GetFXAnnotAtPoint(FX_FLOAT pageX,
                                                   FX_FLOAT pageY) {
  CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();
  CPDFSDK_AnnotHandlerMgr* pAnnotMgr = pEnv->GetAnnotHandlerMgr();
  CPDFSDK_AnnotIterator annotIterator(this, false);
  while (CPDFSDK_Annot* pSDKAnnot = annotIterator.Next()) {
    CFX_FloatRect rc = pAnnotMgr->Annot_OnGetViewBBox(this, pSDKAnnot);
    if (pSDKAnnot->GetAnnotSubtype() == CPDF_Annot::Subtype::POPUP)
      continue;
    if (rc.Contains(pageX, pageY))
      return pSDKAnnot;
  }

  return nullptr;
}

CPDFSDK_Annot* CPDFSDK_PageView::GetFXWidgetAtPoint(FX_FLOAT pageX,
                                                    FX_FLOAT pageY) {
  CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();
  CPDFSDK_AnnotHandlerMgr* pAnnotMgr = pEnv->GetAnnotHandlerMgr();
  CPDFSDK_AnnotIterator annotIterator(this, false);
  while (CPDFSDK_Annot* pSDKAnnot = annotIterator.Next()) {
    bool bHitTest = pSDKAnnot->GetAnnotSubtype() == CPDF_Annot::Subtype::WIDGET;
#ifdef PDF_ENABLE_XFA
    bHitTest = bHitTest ||
               pSDKAnnot->GetAnnotSubtype() == CPDF_Annot::Subtype::XFAWIDGET;
#endif  // PDF_ENABLE_XFA
    if (bHitTest) {
      pAnnotMgr->Annot_OnGetViewBBox(this, pSDKAnnot);
      CFX_FloatPoint point(pageX, pageY);
      if (pAnnotMgr->Annot_OnHitTest(this, pSDKAnnot, point))
        return pSDKAnnot;
    }
  }

  return nullptr;
}

void CPDFSDK_PageView::KillFocusAnnotIfNeeded() {
  // if there is a focused annot on the page, we should kill the focus first.
  if (CPDFSDK_Annot* focusedAnnot = m_pSDKDoc->GetFocusAnnot()) {
    if (pdfium::ContainsValue(m_fxAnnotArray, focusedAnnot))
      KillFocusAnnot();
  }
}

CPDFSDK_Annot* CPDFSDK_PageView::AddAnnot(CPDF_Annot* pPDFAnnot) {
  CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();
  ASSERT(pEnv);
  CPDFSDK_AnnotHandlerMgr* pAnnotHandler = pEnv->GetAnnotHandlerMgr();
  CPDFSDK_Annot* pSDKAnnot = pAnnotHandler->NewAnnot(pPDFAnnot, this);
  if (!pSDKAnnot)
    return nullptr;

  m_fxAnnotArray.push_back(pSDKAnnot);
  pAnnotHandler->Annot_OnCreate(pSDKAnnot);
  return pSDKAnnot;
}

#ifdef PDF_ENABLE_XFA
CPDFSDK_Annot* CPDFSDK_PageView::AddAnnot(CXFA_FFWidget* pPDFAnnot) {
  if (!pPDFAnnot)
    return nullptr;

  CPDFSDK_Annot* pSDKAnnot = GetAnnotByXFAWidget(pPDFAnnot);
  if (pSDKAnnot)
    return pSDKAnnot;

  CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();
  CPDFSDK_AnnotHandlerMgr* pAnnotHandler = pEnv->GetAnnotHandlerMgr();
  pSDKAnnot = pAnnotHandler->NewAnnot(pPDFAnnot, this);
  if (!pSDKAnnot)
    return nullptr;

  m_fxAnnotArray.push_back(pSDKAnnot);
  return pSDKAnnot;
}
#endif  // PDF_ENABLE_XFA

CPDFSDK_Annot* CPDFSDK_PageView::AddAnnot(CPDF_Dictionary* pDict) {
  return pDict ? AddAnnot(pDict->GetStringFor("Subtype").c_str(), pDict)
               : nullptr;
}

CPDFSDK_Annot* CPDFSDK_PageView::AddAnnot(const FX_CHAR* lpSubType,
                                          CPDF_Dictionary* pDict) {
  return nullptr;
}

FX_BOOL CPDFSDK_PageView::DeleteAnnot(CPDFSDK_Annot* pAnnot) {
#ifdef PDF_ENABLE_XFA
  if (!pAnnot)
    return FALSE;
  CPDFXFA_Page* pPage = pAnnot->GetPDFXFAPage();
  if (!pPage || (pPage->GetDocument()->GetDocType() != DOCTYPE_STATIC_XFA &&
                 pPage->GetDocument()->GetDocType() != DOCTYPE_DYNAMIC_XFA))
    return FALSE;

  if (GetFocusAnnot() == pAnnot)
    KillFocusAnnot();
  CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();
  CPDFSDK_AnnotHandlerMgr* pAnnotHandler = pEnv->GetAnnotHandlerMgr();
  if (pAnnotHandler)
    pAnnotHandler->ReleaseAnnot(pAnnot);

  auto it = std::find(m_fxAnnotArray.begin(), m_fxAnnotArray.end(), pAnnot);
  if (it != m_fxAnnotArray.end())
    m_fxAnnotArray.erase(it);
  if (m_CaptureWidget == pAnnot)
    m_CaptureWidget = nullptr;

  return TRUE;
#else   // PDF_ENABLE_XFA
  return FALSE;
#endif  // PDF_ENABLE_XFA
}

CPDF_Document* CPDFSDK_PageView::GetPDFDocument() {
  if (m_page) {
#ifdef PDF_ENABLE_XFA
    return m_page->GetDocument()->GetPDFDoc();
#else   // PDF_ENABLE_XFA
    return m_page->m_pDocument;
#endif  // PDF_ENABLE_XFA
  }
  return nullptr;
}

CPDF_Page* CPDFSDK_PageView::GetPDFPage() const {
#ifdef PDF_ENABLE_XFA
  return m_page ? m_page->GetPDFPage() : nullptr;
#else   // PDF_ENABLE_XFA
  return m_page;
#endif  // PDF_ENABLE_XFA
}

size_t CPDFSDK_PageView::CountAnnots() const {
  return m_fxAnnotArray.size();
}

CPDFSDK_Annot* CPDFSDK_PageView::GetAnnot(size_t nIndex) {
  return nIndex < m_fxAnnotArray.size() ? m_fxAnnotArray[nIndex] : nullptr;
}

CPDFSDK_Annot* CPDFSDK_PageView::GetAnnotByDict(CPDF_Dictionary* pDict) {
  for (CPDFSDK_Annot* pAnnot : m_fxAnnotArray) {
    if (pAnnot->GetPDFAnnot()->GetAnnotDict() == pDict)
      return pAnnot;
  }
  return nullptr;
}

#ifdef PDF_ENABLE_XFA
CPDFSDK_Annot* CPDFSDK_PageView::GetAnnotByXFAWidget(CXFA_FFWidget* hWidget) {
  if (!hWidget)
    return nullptr;

  for (CPDFSDK_Annot* pAnnot : m_fxAnnotArray) {
    if (pAnnot->GetXFAWidget() == hWidget)
      return pAnnot;
  }
  return nullptr;
}
#endif  // PDF_ENABLE_XFA

FX_BOOL CPDFSDK_PageView::OnLButtonDown(const CFX_FloatPoint& point,
                                        uint32_t nFlag) {
  CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();
  ASSERT(pEnv);
  CPDFSDK_Annot* pFXAnnot = GetFXWidgetAtPoint(point.x, point.y);
  if (!pFXAnnot) {
    KillFocusAnnot(nFlag);
    return FALSE;
  }

  CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
  FX_BOOL bRet =
      pAnnotHandlerMgr->Annot_OnLButtonDown(this, pFXAnnot, nFlag, point);
  if (bRet)
    SetFocusAnnot(pFXAnnot);
  return bRet;
}

#ifdef PDF_ENABLE_XFA
FX_BOOL CPDFSDK_PageView::OnRButtonDown(const CFX_FloatPoint& point,
                                        uint32_t nFlag) {
  CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();
  ASSERT(pEnv);
  CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
  ASSERT(pAnnotHandlerMgr);

  CPDFSDK_Annot* pFXAnnot = GetFXWidgetAtPoint(point.x, point.y);

  if (!pFXAnnot)
    return FALSE;

  if (pAnnotHandlerMgr->Annot_OnRButtonDown(this, pFXAnnot, nFlag, point))
    SetFocusAnnot(pFXAnnot);

  return TRUE;
}

FX_BOOL CPDFSDK_PageView::OnRButtonUp(const CFX_FloatPoint& point,
                                      uint32_t nFlag) {
  CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();
  ASSERT(pEnv);
  CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();

  CPDFSDK_Annot* pFXAnnot = GetFXWidgetAtPoint(point.x, point.y);

  if (!pFXAnnot)
    return FALSE;

  if (pAnnotHandlerMgr->Annot_OnRButtonUp(this, pFXAnnot, nFlag, point))
    SetFocusAnnot(pFXAnnot);

  return TRUE;
}
#endif  // PDF_ENABLE_XFA

FX_BOOL CPDFSDK_PageView::OnLButtonUp(const CFX_FloatPoint& point,
                                      uint32_t nFlag) {
  CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();
  ASSERT(pEnv);
  CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
  CPDFSDK_Annot* pFXAnnot = GetFXWidgetAtPoint(point.x, point.y);
  CPDFSDK_Annot* pFocusAnnot = GetFocusAnnot();
  FX_BOOL bRet = FALSE;
  if (pFocusAnnot && pFocusAnnot != pFXAnnot) {
    // Last focus Annot gets a chance to handle the event.
    bRet = pAnnotHandlerMgr->Annot_OnLButtonUp(this, pFocusAnnot, nFlag, point);
  }
  if (pFXAnnot && !bRet)
    bRet = pAnnotHandlerMgr->Annot_OnLButtonUp(this, pFXAnnot, nFlag, point);
  return bRet;
}

FX_BOOL CPDFSDK_PageView::OnMouseMove(const CFX_FloatPoint& point, int nFlag) {
  CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();
  CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
  if (CPDFSDK_Annot* pFXAnnot = GetFXAnnotAtPoint(point.x, point.y)) {
    if (m_CaptureWidget && m_CaptureWidget != pFXAnnot) {
      m_bExitWidget = TRUE;
      m_bEnterWidget = FALSE;
      pAnnotHandlerMgr->Annot_OnMouseExit(this, m_CaptureWidget, nFlag);
    }
    m_CaptureWidget = pFXAnnot;
    m_bOnWidget = TRUE;
    if (!m_bEnterWidget) {
      m_bEnterWidget = TRUE;
      m_bExitWidget = FALSE;
      pAnnotHandlerMgr->Annot_OnMouseEnter(this, pFXAnnot, nFlag);
    }
    pAnnotHandlerMgr->Annot_OnMouseMove(this, pFXAnnot, nFlag, point);
    return TRUE;
  }
  if (m_bOnWidget) {
    m_bOnWidget = FALSE;
    m_bExitWidget = TRUE;
    m_bEnterWidget = FALSE;
    if (m_CaptureWidget) {
      pAnnotHandlerMgr->Annot_OnMouseExit(this, m_CaptureWidget, nFlag);
      m_CaptureWidget = nullptr;
    }
  }
  return FALSE;
}

FX_BOOL CPDFSDK_PageView::OnMouseWheel(double deltaX,
                                       double deltaY,
                                       const CFX_FloatPoint& point,
                                       int nFlag) {
  if (CPDFSDK_Annot* pAnnot = GetFXWidgetAtPoint(point.x, point.y)) {
    CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();
    CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
    return pAnnotHandlerMgr->Annot_OnMouseWheel(this, pAnnot, nFlag,
                                                (int)deltaY, point);
  }
  return FALSE;
}

FX_BOOL CPDFSDK_PageView::OnChar(int nChar, uint32_t nFlag) {
  if (CPDFSDK_Annot* pAnnot = GetFocusAnnot()) {
    CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();
    CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
    return pAnnotHandlerMgr->Annot_OnChar(pAnnot, nChar, nFlag);
  }

  return FALSE;
}

FX_BOOL CPDFSDK_PageView::OnKeyDown(int nKeyCode, int nFlag) {
  if (CPDFSDK_Annot* pAnnot = GetFocusAnnot()) {
    CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();
    CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
    return pAnnotHandlerMgr->Annot_OnKeyDown(pAnnot, nKeyCode, nFlag);
  }
  return FALSE;
}

FX_BOOL CPDFSDK_PageView::OnKeyUp(int nKeyCode, int nFlag) {
  return FALSE;
}

void CPDFSDK_PageView::LoadFXAnnots() {
  CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();
  CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();

  SetLock(TRUE);

#ifdef PDF_ENABLE_XFA
  CFX_RetainPtr<CPDFXFA_Page> protector(m_page);
  if (m_pSDKDoc->GetXFADocument()->GetDocType() == DOCTYPE_DYNAMIC_XFA) {
    CXFA_FFPageView* pageView = m_page->GetXFAPageView();
    std::unique_ptr<IXFA_WidgetIterator> pWidgetHander(
        pageView->CreateWidgetIterator(
            XFA_TRAVERSEWAY_Form,
            XFA_WidgetStatus_Visible | XFA_WidgetStatus_Viewable));
    if (!pWidgetHander) {
      SetLock(FALSE);
      return;
    }

    while (CXFA_FFWidget* pXFAAnnot = pWidgetHander->MoveToNext()) {
      CPDFSDK_Annot* pAnnot = pAnnotHandlerMgr->NewAnnot(pXFAAnnot, this);
      if (!pAnnot)
        continue;
      m_fxAnnotArray.push_back(pAnnot);
      pAnnotHandlerMgr->Annot_OnLoad(pAnnot);
    }

    SetLock(FALSE);
    return;
  }
#endif  // PDF_ENABLE_XFA

  CPDF_Page* pPage = GetPDFPage();
  ASSERT(pPage);
  FX_BOOL bUpdateAP = CPDF_InterForm::IsUpdateAPEnabled();
  // Disable the default AP construction.
  CPDF_InterForm::SetUpdateAP(FALSE);
  m_pAnnotList.reset(new CPDF_AnnotList(pPage));
  CPDF_InterForm::SetUpdateAP(bUpdateAP);

  const size_t nCount = m_pAnnotList->Count();
  for (size_t i = 0; i < nCount; ++i) {
    CPDF_Annot* pPDFAnnot = m_pAnnotList->GetAt(i);
    CheckUnSupportAnnot(GetPDFDocument(), pPDFAnnot);
    CPDFSDK_Annot* pAnnot = pAnnotHandlerMgr->NewAnnot(pPDFAnnot, this);
    if (!pAnnot)
      continue;
    m_fxAnnotArray.push_back(pAnnot);
    pAnnotHandlerMgr->Annot_OnLoad(pAnnot);
  }

  SetLock(FALSE);
}

void CPDFSDK_PageView::ClearFXAnnots() {
  SetLock(TRUE);
  if (m_pSDKDoc && GetFocusAnnot())
    m_pSDKDoc->SetFocusAnnot(nullptr);
  m_CaptureWidget = nullptr;
  for (CPDFSDK_Annot* pAnnot : m_fxAnnotArray)
    m_pSDKDoc->GetEnv()->GetAnnotHandlerMgr()->ReleaseAnnot(pAnnot);
  m_fxAnnotArray.clear();
  m_pAnnotList.reset();
  SetLock(FALSE);
}

void CPDFSDK_PageView::UpdateRects(const std::vector<CFX_FloatRect>& rects) {
  CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();
  for (const auto& rc : rects)
    pEnv->Invalidate(m_page, rc.left, rc.top, rc.right, rc.bottom);
}

void CPDFSDK_PageView::UpdateView(CPDFSDK_Annot* pAnnot) {
  CFX_FloatRect rcWindow = pAnnot->GetRect();
  CPDFSDK_Environment* pEnv = m_pSDKDoc->GetEnv();
  pEnv->Invalidate(m_page, rcWindow.left, rcWindow.top, rcWindow.right,
                   rcWindow.bottom);
}

int CPDFSDK_PageView::GetPageIndex() const {
  if (!m_page)
    return -1;

#ifdef PDF_ENABLE_XFA
  int nDocType = m_page->GetDocument()->GetDocType();
  switch (nDocType) {
    case DOCTYPE_DYNAMIC_XFA: {
      CXFA_FFPageView* pPageView = m_page->GetXFAPageView();
      return pPageView ? pPageView->GetPageIndex() : -1;
    }
    case DOCTYPE_STATIC_XFA:
    case DOCTYPE_PDF:
      return GetPageIndexForStaticPDF();
    default:
      return -1;
  }
#else   // PDF_ENABLE_XFA
  return GetPageIndexForStaticPDF();
#endif  // PDF_ENABLE_XFA
}

bool CPDFSDK_PageView::IsValidAnnot(const CPDF_Annot* p) const {
  if (!p)
    return false;

  const auto& annots = m_pAnnotList->All();
  auto it = std::find_if(annots.begin(), annots.end(),
                         [p](const std::unique_ptr<CPDF_Annot>& annot) {
                           return annot.get() == p;
                         });
  return it != annots.end();
}

CPDFSDK_Annot* CPDFSDK_PageView::GetFocusAnnot() {
  CPDFSDK_Annot* pFocusAnnot = m_pSDKDoc->GetFocusAnnot();
  if (!pFocusAnnot)
    return nullptr;

  for (CPDFSDK_Annot* pAnnot : m_fxAnnotArray) {
    if (pAnnot == pFocusAnnot)
      return pAnnot;
  }
  return nullptr;
}

int CPDFSDK_PageView::GetPageIndexForStaticPDF() const {
  CPDF_Dictionary* pDict = GetPDFPage()->m_pFormDict;
  CPDF_Document* pDoc = m_pSDKDoc->GetPDFDocument();
  return (pDoc && pDict) ? pDoc->GetPageIndex(pDict->GetObjNum()) : -1;
}
