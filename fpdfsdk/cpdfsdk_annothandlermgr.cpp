// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/cpdfsdk_annothandlermgr.h"

#include "core/fpdfdoc/include/cpdf_annot.h"
#include "fpdfsdk/include/cba_annotiterator.h"
#include "fpdfsdk/include/cpdfsdk_annot.h"
#include "fpdfsdk/include/cpdfsdk_baannot.h"
#include "fpdfsdk/include/cpdfsdk_baannothandler.h"
#include "fpdfsdk/include/cpdfsdk_datetime.h"
#include "fpdfsdk/include/cpdfsdk_document.h"
#include "fpdfsdk/include/cpdfsdk_environment.h"
#include "fpdfsdk/include/cpdfsdk_pageview.h"
#include "fpdfsdk/include/cpdfsdk_widgethandler.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_page.h"
#include "fpdfsdk/include/cpdfsdk_xfawidgethandler.h"
#include "xfa/fxfa/include/xfa_ffpageview.h"
#include "xfa/fxfa/include/xfa_ffwidget.h"
#endif  // PDF_ENABLE_XFA

CPDFSDK_AnnotHandlerMgr::CPDFSDK_AnnotHandlerMgr(CPDFSDK_Environment* pApp)
    : m_pBAAnnotHandler(new CPDFSDK_BAAnnotHandler()),
      m_pWidgetHandler(new CPDFSDK_WidgetHandler(pApp)),
#ifdef PDF_ENABLE_XFA
      m_pXFAWidgetHandler(new CPDFSDK_XFAWidgetHandler(pApp)),
#endif  // PDF_ENABLE_XFA
      m_pApp(pApp) {
  m_pWidgetHandler->SetFormFiller(m_pApp->GetIFormFiller());
}

CPDFSDK_AnnotHandlerMgr::~CPDFSDK_AnnotHandlerMgr() {}

CPDFSDK_Annot* CPDFSDK_AnnotHandlerMgr::NewAnnot(CPDF_Annot* pAnnot,
                                                 CPDFSDK_PageView* pPageView) {
  ASSERT(pPageView);
  return GetAnnotHandler(pAnnot->GetSubtype())->NewAnnot(pAnnot, pPageView);
}

#ifdef PDF_ENABLE_XFA
CPDFSDK_Annot* CPDFSDK_AnnotHandlerMgr::NewAnnot(CXFA_FFWidget* pAnnot,
                                                 CPDFSDK_PageView* pPageView) {
  ASSERT(pAnnot);
  ASSERT(pPageView);

  return GetAnnotHandler(CPDF_Annot::Subtype::XFAWIDGET)
      ->NewAnnot(pAnnot, pPageView);
}
#endif  // PDF_ENABLE_XFA

void CPDFSDK_AnnotHandlerMgr::ReleaseAnnot(CPDFSDK_Annot* pAnnot) {
  IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot);
  pAnnotHandler->OnRelease(pAnnot);
  pAnnotHandler->ReleaseAnnot(pAnnot);
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnCreate(CPDFSDK_Annot* pAnnot) {
  CPDF_Annot* pPDFAnnot = pAnnot->GetPDFAnnot();

  CPDFSDK_DateTime curTime;
  pPDFAnnot->GetAnnotDict()->SetStringFor("M", curTime.ToPDFDateTimeString());
  pPDFAnnot->GetAnnotDict()->SetNumberFor("F", 0);

  GetAnnotHandler(pAnnot)->OnCreate(pAnnot);
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnLoad(CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot);
  GetAnnotHandler(pAnnot)->OnLoad(pAnnot);
}

IPDFSDK_AnnotHandler* CPDFSDK_AnnotHandlerMgr::GetAnnotHandler(
    CPDFSDK_Annot* pAnnot) const {
  return GetAnnotHandler(pAnnot->GetAnnotSubtype());
}

IPDFSDK_AnnotHandler* CPDFSDK_AnnotHandlerMgr::GetAnnotHandler(
    CPDF_Annot::Subtype nAnnotSubtype) const {
  if (nAnnotSubtype == CPDF_Annot::Subtype::WIDGET)
    return m_pWidgetHandler.get();

#ifdef PDF_ENABLE_XFA
  if (nAnnotSubtype == CPDF_Annot::Subtype::XFAWIDGET)
    return m_pXFAWidgetHandler.get();
#endif  // PDF_ENABLE_XFA

  return m_pBAAnnotHandler.get();
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnDraw(CPDFSDK_PageView* pPageView,
                                           CPDFSDK_Annot* pAnnot,
                                           CFX_RenderDevice* pDevice,
                                           CFX_Matrix* pUser2Device,
                                           bool bDrawAnnots) {
  ASSERT(pAnnot);
  GetAnnotHandler(pAnnot)->OnDraw(pPageView, pAnnot, pDevice, pUser2Device,
                                  bDrawAnnots);
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnLButtonDown(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot,
    uint32_t nFlags,
    const CFX_FloatPoint& point) {
  ASSERT(pAnnot);
  return GetAnnotHandler(pAnnot)->OnLButtonDown(pPageView, pAnnot, nFlags,
                                                point);
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnLButtonUp(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot,
    uint32_t nFlags,
    const CFX_FloatPoint& point) {
  ASSERT(pAnnot);
  return GetAnnotHandler(pAnnot)->OnLButtonUp(pPageView, pAnnot, nFlags, point);
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnLButtonDblClk(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot,
    uint32_t nFlags,
    const CFX_FloatPoint& point) {
  ASSERT(pAnnot);
  return GetAnnotHandler(pAnnot)->OnLButtonDblClk(pPageView, pAnnot, nFlags,
                                                  point);
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnMouseMove(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot,
    uint32_t nFlags,
    const CFX_FloatPoint& point) {
  ASSERT(pAnnot);
  return GetAnnotHandler(pAnnot)->OnMouseMove(pPageView, pAnnot, nFlags, point);
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnMouseWheel(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot,
    uint32_t nFlags,
    short zDelta,
    const CFX_FloatPoint& point) {
  ASSERT(pAnnot);
  return GetAnnotHandler(pAnnot)->OnMouseWheel(pPageView, pAnnot, nFlags,
                                               zDelta, point);
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnRButtonDown(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot,
    uint32_t nFlags,
    const CFX_FloatPoint& point) {
  ASSERT(pAnnot);
  return GetAnnotHandler(pAnnot)->OnRButtonDown(pPageView, pAnnot, nFlags,
                                                point);
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnRButtonUp(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot,
    uint32_t nFlags,
    const CFX_FloatPoint& point) {
  ASSERT(pAnnot);
  return GetAnnotHandler(pAnnot)->OnRButtonUp(pPageView, pAnnot, nFlags, point);
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnMouseEnter(CPDFSDK_PageView* pPageView,
                                                 CPDFSDK_Annot* pAnnot,
                                                 uint32_t nFlag) {
  ASSERT(pAnnot);
  GetAnnotHandler(pAnnot)->OnMouseEnter(pPageView, pAnnot, nFlag);
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnMouseExit(CPDFSDK_PageView* pPageView,
                                                CPDFSDK_Annot* pAnnot,
                                                uint32_t nFlag) {
  ASSERT(pAnnot);
  GetAnnotHandler(pAnnot)->OnMouseExit(pPageView, pAnnot, nFlag);
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnChar(CPDFSDK_Annot* pAnnot,
                                              uint32_t nChar,
                                              uint32_t nFlags) {
  return GetAnnotHandler(pAnnot)->OnChar(pAnnot, nChar, nFlags);
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnKeyDown(CPDFSDK_Annot* pAnnot,
                                                 int nKeyCode,
                                                 int nFlag) {
  if (m_pApp->IsCTRLKeyDown(nFlag) || m_pApp->IsALTKeyDown(nFlag))
    return GetAnnotHandler(pAnnot)->OnKeyDown(pAnnot, nKeyCode, nFlag);

  CPDFSDK_PageView* pPage = pAnnot->GetPageView();
  CPDFSDK_Annot* pFocusAnnot = pPage->GetFocusAnnot();
  if (pFocusAnnot && (nKeyCode == FWL_VKEY_Tab)) {
    CPDFSDK_Annot* pNext =
        GetNextAnnot(pFocusAnnot, !m_pApp->IsSHIFTKeyDown(nFlag));

    if (pNext && pNext != pFocusAnnot) {
      CPDFSDK_Document* pDocument = pPage->GetSDKDocument();
      pDocument->SetFocusAnnot(pNext);
      return TRUE;
    }
  }

  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnKeyUp(CPDFSDK_Annot* pAnnot,
                                               int nKeyCode,
                                               int nFlag) {
  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnSetFocus(CPDFSDK_Annot* pAnnot,
                                                  uint32_t nFlag) {
  ASSERT(pAnnot);

  if (!GetAnnotHandler(pAnnot)->OnSetFocus(pAnnot, nFlag))
    return FALSE;

  CPDFSDK_PageView* pPage = pAnnot->GetPageView();
  pPage->GetSDKDocument();
  return TRUE;
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnKillFocus(CPDFSDK_Annot* pAnnot,
                                                   uint32_t nFlag) {
  ASSERT(pAnnot);
  return GetAnnotHandler(pAnnot)->OnKillFocus(pAnnot, nFlag);
}

#ifdef PDF_ENABLE_XFA
FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnChangeFocus(
    CPDFSDK_Annot* pSetAnnot,
    CPDFSDK_Annot* pKillAnnot) {
  FX_BOOL bXFA = (pSetAnnot && pSetAnnot->GetXFAWidget()) ||
                 (pKillAnnot && pKillAnnot->GetXFAWidget());

  if (bXFA) {
    if (IPDFSDK_AnnotHandler* pXFAAnnotHandler =
            GetAnnotHandler(CPDF_Annot::Subtype::XFAWIDGET))
      return pXFAAnnotHandler->OnXFAChangedFocus(pKillAnnot, pSetAnnot);
  }

  return TRUE;
}
#endif  // PDF_ENABLE_XFA

CFX_FloatRect CPDFSDK_AnnotHandlerMgr::Annot_OnGetViewBBox(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot);
  return GetAnnotHandler(pAnnot)->GetViewBBox(pPageView, pAnnot);
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnHitTest(CPDFSDK_PageView* pPageView,
                                                 CPDFSDK_Annot* pAnnot,
                                                 const CFX_FloatPoint& point) {
  ASSERT(pAnnot);
  IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot);
  if (pAnnotHandler->CanAnswer(pAnnot))
    return pAnnotHandler->HitTest(pPageView, pAnnot, point);

  return FALSE;
}

CPDFSDK_Annot* CPDFSDK_AnnotHandlerMgr::GetNextAnnot(CPDFSDK_Annot* pSDKAnnot,
                                                     FX_BOOL bNext) {
#ifdef PDF_ENABLE_XFA
  CPDFSDK_PageView* pPageView = pSDKAnnot->GetPageView();
  CPDFXFA_Page* pPage = pPageView->GetPDFXFAPage();
  if (!pPage)
    return nullptr;
  if (pPage->GetPDFPage()) {  // for pdf annots.
    CBA_AnnotIterator ai(pSDKAnnot->GetPageView(),
                         pSDKAnnot->GetAnnotSubtype());
    CPDFSDK_Annot* pNext =
        bNext ? ai.GetNextAnnot(pSDKAnnot) : ai.GetPrevAnnot(pSDKAnnot);
    return pNext;
  }
  // for xfa annots
  std::unique_ptr<IXFA_WidgetIterator> pWidgetIterator(
      pPage->GetXFAPageView()->CreateWidgetIterator(
          XFA_TRAVERSEWAY_Tranvalse, XFA_WidgetStatus_Visible |
                                         XFA_WidgetStatus_Viewable |
                                         XFA_WidgetStatus_Focused));
  if (!pWidgetIterator)
    return nullptr;
  if (pWidgetIterator->GetCurrentWidget() != pSDKAnnot->GetXFAWidget())
    pWidgetIterator->SetCurrentWidget(pSDKAnnot->GetXFAWidget());
  CXFA_FFWidget* hNextFocus =
      bNext ? pWidgetIterator->MoveToNext() : pWidgetIterator->MoveToPrevious();
  if (!hNextFocus && pSDKAnnot)
    hNextFocus = pWidgetIterator->MoveToFirst();

  return pPageView->GetAnnotByXFAWidget(hNextFocus);
#else   // PDF_ENABLE_XFA
  CBA_AnnotIterator ai(pSDKAnnot->GetPageView(), CPDF_Annot::Subtype::WIDGET);
  return bNext ? ai.GetNextAnnot(pSDKAnnot) : ai.GetPrevAnnot(pSDKAnnot);
#endif  // PDF_ENABLE_XFA
}
