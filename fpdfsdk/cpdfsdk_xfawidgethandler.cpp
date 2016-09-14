// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/cpdfsdk_xfawidgethandler.h"

#include "core/fpdfdoc/include/cpdf_interform.h"
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_doc.h"
#include "fpdfsdk/include/cpdfsdk_annot.h"
#include "fpdfsdk/include/cpdfsdk_document.h"
#include "fpdfsdk/include/cpdfsdk_environment.h"
#include "fpdfsdk/include/cpdfsdk_interform.h"
#include "fpdfsdk/include/cpdfsdk_pageview.h"
#include "fpdfsdk/include/cpdfsdk_xfawidget.h"
#include "xfa/fwl/core/include/fwl_widgethit.h"
#include "xfa/fxfa/include/fxfa_basic.h"
#include "xfa/fxfa/include/xfa_ffdocview.h"
#include "xfa/fxfa/include/xfa_ffpageview.h"
#include "xfa/fxfa/include/xfa_ffwidget.h"
#include "xfa/fxfa/include/xfa_ffwidgethandler.h"
#include "xfa/fxgraphics/include/cfx_graphics.h"

CPDFSDK_XFAWidgetHandler::CPDFSDK_XFAWidgetHandler(CPDFSDK_Environment* pApp)
    : m_pApp(pApp) {}

CPDFSDK_XFAWidgetHandler::~CPDFSDK_XFAWidgetHandler() {}

FX_BOOL CPDFSDK_XFAWidgetHandler::CanAnswer(CPDFSDK_Annot* pAnnot) {
  return !!pAnnot->GetXFAWidget();
}

CPDFSDK_Annot* CPDFSDK_XFAWidgetHandler::NewAnnot(CPDF_Annot* pAnnot,
                                                  CPDFSDK_PageView* pPage) {
  return nullptr;
}

CPDFSDK_Annot* CPDFSDK_XFAWidgetHandler::NewAnnot(CXFA_FFWidget* pAnnot,
                                                  CPDFSDK_PageView* pPage) {
  CPDFSDK_Document* pSDKDoc = m_pApp->GetSDKDocument();
  CPDFSDK_InterForm* pInterForm = pSDKDoc->GetInterForm();
  CPDFSDK_XFAWidget* pWidget = new CPDFSDK_XFAWidget(pAnnot, pPage, pInterForm);
  pInterForm->AddXFAMap(pAnnot, pWidget);
  return pWidget;
}

void CPDFSDK_XFAWidgetHandler::OnDraw(CPDFSDK_PageView* pPageView,
                                      CPDFSDK_Annot* pAnnot,
                                      CFX_RenderDevice* pDevice,
                                      CFX_Matrix* pUser2Device,
                                      bool bDrawAnnots) {
  ASSERT(pPageView);
  ASSERT(pAnnot);

  CPDFSDK_Document* pSDKDoc = pPageView->GetSDKDocument();
  CXFA_FFWidgetHandler* pWidgetHandler = GetXFAWidgetHandler(pAnnot);

  CFX_Graphics gs;
  gs.Create(pDevice);

  CFX_Matrix mt;
  mt = *pUser2Device;

  FX_BOOL bIsHighlight = FALSE;
  if (pSDKDoc->GetFocusAnnot() != pAnnot)
    bIsHighlight = TRUE;

  pWidgetHandler->RenderWidget(pAnnot->GetXFAWidget(), &gs, &mt, bIsHighlight);

  // to do highlight and shadow
}

void CPDFSDK_XFAWidgetHandler::OnCreate(CPDFSDK_Annot* pAnnot) {}

void CPDFSDK_XFAWidgetHandler::OnLoad(CPDFSDK_Annot* pAnnot) {}

void CPDFSDK_XFAWidgetHandler::OnDelete(CPDFSDK_Annot* pAnnot) {}

void CPDFSDK_XFAWidgetHandler::OnRelease(CPDFSDK_Annot* pAnnot) {}

void CPDFSDK_XFAWidgetHandler::ReleaseAnnot(CPDFSDK_Annot* pAnnot) {
  CPDFSDK_XFAWidget* pWidget = reinterpret_cast<CPDFSDK_XFAWidget*>(pAnnot);
  CPDFSDK_InterForm* pInterForm = pWidget->GetInterForm();
  pInterForm->RemoveXFAMap(pWidget->GetXFAWidget());

  delete pWidget;
}

void CPDFSDK_XFAWidgetHandler::DeleteAnnot(CPDFSDK_Annot* pAnnot) {}

CFX_FloatRect CPDFSDK_XFAWidgetHandler::GetViewBBox(CPDFSDK_PageView* pPageView,
                                                    CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot);

  CFX_RectF rcBBox;
  XFA_Element eType = pAnnot->GetXFAWidget()->GetDataAcc()->GetUIType();
  if (eType == XFA_Element::Signature)
    pAnnot->GetXFAWidget()->GetBBox(rcBBox, XFA_WidgetStatus_Visible, TRUE);
  else
    pAnnot->GetXFAWidget()->GetBBox(rcBBox, XFA_WidgetStatus_None);

  CFX_FloatRect rcWidget(rcBBox.left, rcBBox.top, rcBBox.left + rcBBox.width,
                         rcBBox.top + rcBBox.height);
  rcWidget.left -= 1.0f;
  rcWidget.right += 1.0f;
  rcWidget.bottom -= 1.0f;
  rcWidget.top += 1.0f;

  return rcWidget;
}

FX_BOOL CPDFSDK_XFAWidgetHandler::HitTest(CPDFSDK_PageView* pPageView,
                                          CPDFSDK_Annot* pAnnot,
                                          const CFX_FloatPoint& point) {
  if (!pPageView || !pAnnot)
    return FALSE;

  CPDFSDK_Document* pSDKDoc = pPageView->GetSDKDocument();
  if (!pSDKDoc)
    return FALSE;

  CPDFXFA_Document* pDoc = pSDKDoc->GetXFADocument();
  if (!pDoc)
    return FALSE;

  CXFA_FFDocView* pDocView = pDoc->GetXFADocView();
  if (!pDocView)
    return FALSE;

  CXFA_FFWidgetHandler* pWidgetHandler = pDocView->GetWidgetHandler();
  if (!pWidgetHandler)
    return FALSE;

  FWL_WidgetHit dwHitTest =
      pWidgetHandler->OnHitTest(pAnnot->GetXFAWidget(), point.x, point.y);
  return dwHitTest != FWL_WidgetHit::Unknown;
}

void CPDFSDK_XFAWidgetHandler::OnMouseEnter(CPDFSDK_PageView* pPageView,
                                            CPDFSDK_Annot* pAnnot,
                                            uint32_t nFlag) {
  if (!pPageView || !pAnnot)
    return;
  CXFA_FFWidgetHandler* pWidgetHandler = GetXFAWidgetHandler(pAnnot);
  pWidgetHandler->OnMouseEnter(pAnnot->GetXFAWidget());
}

void CPDFSDK_XFAWidgetHandler::OnMouseExit(CPDFSDK_PageView* pPageView,
                                           CPDFSDK_Annot* pAnnot,
                                           uint32_t nFlag) {
  if (!pPageView || !pAnnot)
    return;

  CXFA_FFWidgetHandler* pWidgetHandler = GetXFAWidgetHandler(pAnnot);
  pWidgetHandler->OnMouseExit(pAnnot->GetXFAWidget());
}

FX_BOOL CPDFSDK_XFAWidgetHandler::OnLButtonDown(CPDFSDK_PageView* pPageView,
                                                CPDFSDK_Annot* pAnnot,
                                                uint32_t nFlags,
                                                const CFX_FloatPoint& point) {
  if (!pPageView || !pAnnot)
    return FALSE;

  CXFA_FFWidgetHandler* pWidgetHandler = GetXFAWidgetHandler(pAnnot);
  return pWidgetHandler->OnLButtonDown(pAnnot->GetXFAWidget(),
                                       GetFWLFlags(nFlags), point.x, point.y);
}

FX_BOOL CPDFSDK_XFAWidgetHandler::OnLButtonUp(CPDFSDK_PageView* pPageView,
                                              CPDFSDK_Annot* pAnnot,
                                              uint32_t nFlags,
                                              const CFX_FloatPoint& point) {
  if (!pPageView || !pAnnot)
    return FALSE;

  CXFA_FFWidgetHandler* pWidgetHandler = GetXFAWidgetHandler(pAnnot);
  return pWidgetHandler->OnLButtonUp(pAnnot->GetXFAWidget(),
                                     GetFWLFlags(nFlags), point.x, point.y);
}

FX_BOOL CPDFSDK_XFAWidgetHandler::OnLButtonDblClk(CPDFSDK_PageView* pPageView,
                                                  CPDFSDK_Annot* pAnnot,
                                                  uint32_t nFlags,
                                                  const CFX_FloatPoint& point) {
  if (!pPageView || !pAnnot)
    return FALSE;

  CXFA_FFWidgetHandler* pWidgetHandler = GetXFAWidgetHandler(pAnnot);
  return pWidgetHandler->OnLButtonDblClk(pAnnot->GetXFAWidget(),
                                         GetFWLFlags(nFlags), point.x, point.y);
}

FX_BOOL CPDFSDK_XFAWidgetHandler::OnMouseMove(CPDFSDK_PageView* pPageView,
                                              CPDFSDK_Annot* pAnnot,
                                              uint32_t nFlags,
                                              const CFX_FloatPoint& point) {
  if (!pPageView || !pAnnot)
    return FALSE;

  CXFA_FFWidgetHandler* pWidgetHandler = GetXFAWidgetHandler(pAnnot);
  return pWidgetHandler->OnMouseMove(pAnnot->GetXFAWidget(),
                                     GetFWLFlags(nFlags), point.x, point.y);
}

FX_BOOL CPDFSDK_XFAWidgetHandler::OnMouseWheel(CPDFSDK_PageView* pPageView,
                                               CPDFSDK_Annot* pAnnot,
                                               uint32_t nFlags,
                                               short zDelta,
                                               const CFX_FloatPoint& point) {
  if (!pPageView || !pAnnot)
    return FALSE;

  CXFA_FFWidgetHandler* pWidgetHandler = GetXFAWidgetHandler(pAnnot);
  return pWidgetHandler->OnMouseWheel(
      pAnnot->GetXFAWidget(), GetFWLFlags(nFlags), zDelta, point.x, point.y);
}

FX_BOOL CPDFSDK_XFAWidgetHandler::OnRButtonDown(CPDFSDK_PageView* pPageView,
                                                CPDFSDK_Annot* pAnnot,
                                                uint32_t nFlags,
                                                const CFX_FloatPoint& point) {
  if (!pPageView || !pAnnot)
    return FALSE;

  CXFA_FFWidgetHandler* pWidgetHandler = GetXFAWidgetHandler(pAnnot);
  return pWidgetHandler->OnRButtonDown(pAnnot->GetXFAWidget(),
                                       GetFWLFlags(nFlags), point.x, point.y);
}

FX_BOOL CPDFSDK_XFAWidgetHandler::OnRButtonUp(CPDFSDK_PageView* pPageView,
                                              CPDFSDK_Annot* pAnnot,
                                              uint32_t nFlags,
                                              const CFX_FloatPoint& point) {
  if (!pPageView || !pAnnot)
    return FALSE;

  CXFA_FFWidgetHandler* pWidgetHandler = GetXFAWidgetHandler(pAnnot);
  return pWidgetHandler->OnRButtonUp(pAnnot->GetXFAWidget(),
                                     GetFWLFlags(nFlags), point.x, point.y);
}

FX_BOOL CPDFSDK_XFAWidgetHandler::OnRButtonDblClk(CPDFSDK_PageView* pPageView,
                                                  CPDFSDK_Annot* pAnnot,
                                                  uint32_t nFlags,
                                                  const CFX_FloatPoint& point) {
  if (!pPageView || !pAnnot)
    return FALSE;

  CXFA_FFWidgetHandler* pWidgetHandler = GetXFAWidgetHandler(pAnnot);
  return pWidgetHandler->OnRButtonDblClk(pAnnot->GetXFAWidget(),
                                         GetFWLFlags(nFlags), point.x, point.y);
}

FX_BOOL CPDFSDK_XFAWidgetHandler::OnChar(CPDFSDK_Annot* pAnnot,
                                         uint32_t nChar,
                                         uint32_t nFlags) {
  if (!pAnnot)
    return FALSE;

  CXFA_FFWidgetHandler* pWidgetHandler = GetXFAWidgetHandler(pAnnot);
  return pWidgetHandler->OnChar(pAnnot->GetXFAWidget(), nChar,
                                GetFWLFlags(nFlags));
}

FX_BOOL CPDFSDK_XFAWidgetHandler::OnKeyDown(CPDFSDK_Annot* pAnnot,
                                            int nKeyCode,
                                            int nFlag) {
  if (!pAnnot)
    return FALSE;

  CXFA_FFWidgetHandler* pWidgetHandler = GetXFAWidgetHandler(pAnnot);
  return pWidgetHandler->OnKeyDown(pAnnot->GetXFAWidget(), nKeyCode,
                                   GetFWLFlags(nFlag));
}

FX_BOOL CPDFSDK_XFAWidgetHandler::OnKeyUp(CPDFSDK_Annot* pAnnot,
                                          int nKeyCode,
                                          int nFlag) {
  if (!pAnnot)
    return FALSE;

  CXFA_FFWidgetHandler* pWidgetHandler = GetXFAWidgetHandler(pAnnot);
  return pWidgetHandler->OnKeyUp(pAnnot->GetXFAWidget(), nKeyCode,
                                 GetFWLFlags(nFlag));
}

void CPDFSDK_XFAWidgetHandler::OnDeSelected(CPDFSDK_Annot* pAnnot) {}

void CPDFSDK_XFAWidgetHandler::OnSelected(CPDFSDK_Annot* pAnnot) {}

FX_BOOL CPDFSDK_XFAWidgetHandler::OnSetFocus(CPDFSDK_Annot* pAnnot,
                                             uint32_t nFlag) {
  return TRUE;
}

FX_BOOL CPDFSDK_XFAWidgetHandler::OnKillFocus(CPDFSDK_Annot* pAnnot,
                                              uint32_t nFlag) {
  return TRUE;
}

FX_BOOL CPDFSDK_XFAWidgetHandler::OnXFAChangedFocus(CPDFSDK_Annot* pOldAnnot,
                                                    CPDFSDK_Annot* pNewAnnot) {
  CXFA_FFWidgetHandler* pWidgetHandler = nullptr;

  if (pOldAnnot)
    pWidgetHandler = GetXFAWidgetHandler(pOldAnnot);
  else if (pNewAnnot)
    pWidgetHandler = GetXFAWidgetHandler(pNewAnnot);

  if (pWidgetHandler) {
    FX_BOOL bRet = TRUE;
    CXFA_FFWidget* hWidget = pNewAnnot ? pNewAnnot->GetXFAWidget() : nullptr;
    if (hWidget) {
      CXFA_FFPageView* pXFAPageView = hWidget->GetPageView();
      if (pXFAPageView) {
        bRet = pXFAPageView->GetDocView()->SetFocus(hWidget);
        if (pXFAPageView->GetDocView()->GetFocusWidget() == hWidget)
          bRet = TRUE;
      }
    }
    return bRet;
  }

  return TRUE;
}

CXFA_FFWidgetHandler* CPDFSDK_XFAWidgetHandler::GetXFAWidgetHandler(
    CPDFSDK_Annot* pAnnot) {
  if (!pAnnot)
    return nullptr;

  CPDFSDK_PageView* pPageView = pAnnot->GetPageView();
  if (!pPageView)
    return nullptr;

  CPDFSDK_Document* pSDKDoc = pPageView->GetSDKDocument();
  if (!pSDKDoc)
    return nullptr;

  CPDFXFA_Document* pDoc = pSDKDoc->GetXFADocument();
  if (!pDoc)
    return nullptr;

  CXFA_FFDocView* pDocView = pDoc->GetXFADocView();
  if (!pDocView)
    return nullptr;

  return pDocView->GetWidgetHandler();
}

const uint32_t FWL_KEYFLAG_Ctrl = (1 << 0);
const uint32_t FWL_KEYFLAG_Alt = (1 << 1);
const uint32_t FWL_KEYFLAG_Shift = (1 << 2);
const uint32_t FWL_KEYFLAG_LButton = (1 << 3);
const uint32_t FWL_KEYFLAG_RButton = (1 << 4);
const uint32_t FWL_KEYFLAG_MButton = (1 << 5);

uint32_t CPDFSDK_XFAWidgetHandler::GetFWLFlags(uint32_t dwFlag) {
  uint32_t dwFWLFlag = 0;

  if (dwFlag & FWL_EVENTFLAG_ControlKey)
    dwFWLFlag |= FWL_KEYFLAG_Ctrl;
  if (dwFlag & FWL_EVENTFLAG_LeftButtonDown)
    dwFWLFlag |= FWL_KEYFLAG_LButton;
  if (dwFlag & FWL_EVENTFLAG_MiddleButtonDown)
    dwFWLFlag |= FWL_KEYFLAG_MButton;
  if (dwFlag & FWL_EVENTFLAG_RightButtonDown)
    dwFWLFlag |= FWL_KEYFLAG_RButton;
  if (dwFlag & FWL_EVENTFLAG_ShiftKey)
    dwFWLFlag |= FWL_KEYFLAG_Shift;
  if (dwFlag & FWL_EVENTFLAG_AltKey)
    dwFWLFlag |= FWL_KEYFLAG_Alt;

  return dwFWLFlag;
}
