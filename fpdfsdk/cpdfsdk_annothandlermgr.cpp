// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/cpdfsdk_annothandlermgr.h"

#include "core/fpdfdoc/include/cpdf_annot.h"
#include "fpdfsdk/include/cba_annotiterator.h"
#include "fpdfsdk/include/cpdfsdk_annot.h"
#include "fpdfsdk/include/cpdfsdk_baannot.h"
#include "fpdfsdk/include/cpdfsdk_bfannothandler.h"
#include "fpdfsdk/include/cpdfsdk_datetime.h"
#include "fpdfsdk/include/fsdk_mgr.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/include/cpdfsdk_xfaannothandler.h"
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_page.h"
#include "xfa/fxfa/include/xfa_ffpageview.h"
#include "xfa/fxfa/include/xfa_ffwidget.h"
#endif  // PDF_ENABLE_XFA

CPDFSDK_AnnotHandlerMgr::CPDFSDK_AnnotHandlerMgr(CPDFDoc_Environment* pApp) {
  m_pApp = pApp;

  CPDFSDK_BFAnnotHandler* pHandler = new CPDFSDK_BFAnnotHandler(m_pApp);
  pHandler->SetFormFiller(m_pApp->GetIFormFiller());
  RegisterAnnotHandler(pHandler);
#ifdef PDF_ENABLE_XFA
  CPDFSDK_XFAAnnotHandler* pXFAAnnotHandler =
      new CPDFSDK_XFAAnnotHandler(m_pApp);
  RegisterAnnotHandler(pXFAAnnotHandler);
#endif  // PDF_ENABLE_XFA
}

CPDFSDK_AnnotHandlerMgr::~CPDFSDK_AnnotHandlerMgr() {}

void CPDFSDK_AnnotHandlerMgr::RegisterAnnotHandler(
    IPDFSDK_AnnotHandler* pAnnotHandler) {
  ASSERT(!GetAnnotHandler(pAnnotHandler->GetType()));

  m_mapType2Handler[pAnnotHandler->GetType()].reset(pAnnotHandler);
}

void CPDFSDK_AnnotHandlerMgr::UnRegisterAnnotHandler(
    IPDFSDK_AnnotHandler* pAnnotHandler) {
  m_mapType2Handler.erase(pAnnotHandler->GetType());
}

CPDFSDK_Annot* CPDFSDK_AnnotHandlerMgr::NewAnnot(CPDF_Annot* pAnnot,
                                                 CPDFSDK_PageView* pPageView) {
  ASSERT(pPageView);

  if (IPDFSDK_AnnotHandler* pAnnotHandler =
          GetAnnotHandler(pAnnot->GetSubtype())) {
    return pAnnotHandler->NewAnnot(pAnnot, pPageView);
  }

  return new CPDFSDK_BAAnnot(pAnnot, pPageView);
}

#ifdef PDF_ENABLE_XFA
CPDFSDK_Annot* CPDFSDK_AnnotHandlerMgr::NewAnnot(CXFA_FFWidget* pAnnot,
                                                 CPDFSDK_PageView* pPageView) {
  ASSERT(pAnnot);
  ASSERT(pPageView);

  if (IPDFSDK_AnnotHandler* pAnnotHandler =
          GetAnnotHandler(FSDK_XFAWIDGET_TYPENAME)) {
    return pAnnotHandler->NewAnnot(pAnnot, pPageView);
  }

  return nullptr;
}
#endif  // PDF_ENABLE_XFA

void CPDFSDK_AnnotHandlerMgr::ReleaseAnnot(CPDFSDK_Annot* pAnnot) {
  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    pAnnotHandler->OnRelease(pAnnot);
    pAnnotHandler->ReleaseAnnot(pAnnot);
  } else {
    delete pAnnot;
  }
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnCreate(CPDFSDK_Annot* pAnnot) {
  CPDF_Annot* pPDFAnnot = pAnnot->GetPDFAnnot();

  CPDFSDK_DateTime curTime;
  pPDFAnnot->GetAnnotDict()->SetAtString("M", curTime.ToPDFDateTimeString());
  pPDFAnnot->GetAnnotDict()->SetAtNumber("F", 0);

  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot))
    pAnnotHandler->OnCreate(pAnnot);
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnLoad(CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot);

  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot))
    pAnnotHandler->OnLoad(pAnnot);
}

IPDFSDK_AnnotHandler* CPDFSDK_AnnotHandlerMgr::GetAnnotHandler(
    CPDFSDK_Annot* pAnnot) const {
  CPDF_Annot* pPDFAnnot = pAnnot->GetPDFAnnot();
  if (pPDFAnnot)
    return GetAnnotHandler(pPDFAnnot->GetSubtype());
#ifdef PDF_ENABLE_XFA
  if (pAnnot->GetXFAWidget())
    return GetAnnotHandler(FSDK_XFAWIDGET_TYPENAME);
#endif  // PDF_ENABLE_XFA
  return nullptr;
}

IPDFSDK_AnnotHandler* CPDFSDK_AnnotHandlerMgr::GetAnnotHandler(
    const CFX_ByteString& sType) const {
  auto it = m_mapType2Handler.find(sType);
  return it != m_mapType2Handler.end() ? it->second.get() : nullptr;
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnDraw(CPDFSDK_PageView* pPageView,
                                           CPDFSDK_Annot* pAnnot,
                                           CFX_RenderDevice* pDevice,
                                           CFX_Matrix* pUser2Device,
                                           uint32_t dwFlags) {
  ASSERT(pAnnot);

  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    pAnnotHandler->OnDraw(pPageView, pAnnot, pDevice, pUser2Device, dwFlags);
  } else {
#ifdef PDF_ENABLE_XFA
    if (pAnnot->IsXFAField())
      return;
#endif  // PDF_ENABLE_XFA
    static_cast<CPDFSDK_BAAnnot*>(pAnnot)->DrawAppearance(
        pDevice, pUser2Device, CPDF_Annot::Normal, nullptr);
  }
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnLButtonDown(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot,
    uint32_t nFlags,
    const CFX_FloatPoint& point) {
  ASSERT(pAnnot);

  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot))
    return pAnnotHandler->OnLButtonDown(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnLButtonUp(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot,
    uint32_t nFlags,
    const CFX_FloatPoint& point) {
  ASSERT(pAnnot);

  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot))
    return pAnnotHandler->OnLButtonUp(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnLButtonDblClk(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot,
    uint32_t nFlags,
    const CFX_FloatPoint& point) {
  ASSERT(pAnnot);

  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot))
    return pAnnotHandler->OnLButtonDblClk(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnMouseMove(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot,
    uint32_t nFlags,
    const CFX_FloatPoint& point) {
  ASSERT(pAnnot);

  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot))
    return pAnnotHandler->OnMouseMove(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnMouseWheel(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot,
    uint32_t nFlags,
    short zDelta,
    const CFX_FloatPoint& point) {
  ASSERT(pAnnot);

  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    return pAnnotHandler->OnMouseWheel(pPageView, pAnnot, nFlags, zDelta,
                                       point);
  }
  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnRButtonDown(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot,
    uint32_t nFlags,
    const CFX_FloatPoint& point) {
  ASSERT(pAnnot);

  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot))
    return pAnnotHandler->OnRButtonDown(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnRButtonUp(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot,
    uint32_t nFlags,
    const CFX_FloatPoint& point) {
  ASSERT(pAnnot);

  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot))
    return pAnnotHandler->OnRButtonUp(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnMouseEnter(CPDFSDK_PageView* pPageView,
                                                 CPDFSDK_Annot* pAnnot,
                                                 uint32_t nFlag) {
  ASSERT(pAnnot);

  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot))
    pAnnotHandler->OnMouseEnter(pPageView, pAnnot, nFlag);
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnMouseExit(CPDFSDK_PageView* pPageView,
                                                CPDFSDK_Annot* pAnnot,
                                                uint32_t nFlag) {
  ASSERT(pAnnot);

  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot))
    pAnnotHandler->OnMouseExit(pPageView, pAnnot, nFlag);
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnChar(CPDFSDK_Annot* pAnnot,
                                              uint32_t nChar,
                                              uint32_t nFlags) {
  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot))
    return pAnnotHandler->OnChar(pAnnot, nChar, nFlags);

  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnKeyDown(CPDFSDK_Annot* pAnnot,
                                                 int nKeyCode,
                                                 int nFlag) {
  if (!m_pApp->FFI_IsCTRLKeyDown(nFlag) && !m_pApp->FFI_IsALTKeyDown(nFlag)) {
    CPDFSDK_PageView* pPage = pAnnot->GetPageView();
    CPDFSDK_Annot* pFocusAnnot = pPage->GetFocusAnnot();
    if (pFocusAnnot && (nKeyCode == FWL_VKEY_Tab)) {
      CPDFSDK_Annot* pNext =
          GetNextAnnot(pFocusAnnot, !m_pApp->FFI_IsSHIFTKeyDown(nFlag));

      if (pNext && pNext != pFocusAnnot) {
        CPDFSDK_Document* pDocument = pPage->GetSDKDocument();
        pDocument->SetFocusAnnot(pNext);
        return TRUE;
      }
    }
  }

  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot))
    return pAnnotHandler->OnKeyDown(pAnnot, nKeyCode, nFlag);

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

  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    if (pAnnotHandler->OnSetFocus(pAnnot, nFlag)) {
      CPDFSDK_PageView* pPage = pAnnot->GetPageView();
      pPage->GetSDKDocument();
      return TRUE;
    }
  }
  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnKillFocus(CPDFSDK_Annot* pAnnot,
                                                   uint32_t nFlag) {
  ASSERT(pAnnot);

  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot))
    return pAnnotHandler->OnKillFocus(pAnnot, nFlag);

  return FALSE;
}

#ifdef PDF_ENABLE_XFA
FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnChangeFocus(
    CPDFSDK_Annot* pSetAnnot,
    CPDFSDK_Annot* pKillAnnot) {
  FX_BOOL bXFA = (pSetAnnot && pSetAnnot->GetXFAWidget()) ||
                 (pKillAnnot && pKillAnnot->GetXFAWidget());

  if (bXFA) {
    if (IPDFSDK_AnnotHandler* pXFAAnnotHandler =
            GetAnnotHandler(FSDK_XFAWIDGET_TYPENAME))
      return pXFAAnnotHandler->OnXFAChangedFocus(pKillAnnot, pSetAnnot);
  }

  return TRUE;
}
#endif  // PDF_ENABLE_XFA

CFX_FloatRect CPDFSDK_AnnotHandlerMgr::Annot_OnGetViewBBox(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot);
  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot))
    return pAnnotHandler->GetViewBBox(pPageView, pAnnot);

  return pAnnot->GetRect();
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnHitTest(CPDFSDK_PageView* pPageView,
                                                 CPDFSDK_Annot* pAnnot,
                                                 const CFX_FloatPoint& point) {
  ASSERT(pAnnot);
  if (IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    if (pAnnotHandler->CanAnswer(pAnnot))
      return pAnnotHandler->HitTest(pPageView, pAnnot, point);
  }
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
  CBA_AnnotIterator ai(pSDKAnnot->GetPageView(), "Widget");
  return bNext ? ai.GetNextAnnot(pSDKAnnot) : ai.GetPrevAnnot(pSDKAnnot);
#endif  // PDF_ENABLE_XFA
}
