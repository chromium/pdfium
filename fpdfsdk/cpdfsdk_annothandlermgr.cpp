// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_annothandlermgr.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfdoc/cpdf_annot.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/cpdfsdk_annotiterator.h"
#include "fpdfsdk/cpdfsdk_baannot.h"
#include "fpdfsdk/cpdfsdk_baannothandler.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/cpdfsdk_widgethandler.h"
#include "fpdfsdk/pwl/cpwl_wnd.h"
#include "public/fpdf_fwlevent.h"
#include "third_party/base/check.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/cpdfxfa_page.h"
#include "fpdfsdk/fpdfxfa/cpdfxfa_widget.h"
#include "fpdfsdk/fpdfxfa/cpdfxfa_widgethandler.h"

namespace {

CPDFXFA_Page* XFAPageIfNotBackedByPDFPage(CPDFSDK_PageView* pPageView) {
  auto* pPage = static_cast<CPDFXFA_Page*>(pPageView->GetXFAPage());
  return pPage && !pPage->AsPDFPage() ? pPage : nullptr;
}

}  // namespace
#endif  // PDF_ENABLE_XFA

CPDFSDK_AnnotHandlerMgr::CPDFSDK_AnnotHandlerMgr(
    std::unique_ptr<CPDFSDK_BAAnnotHandler> pBAAnnotHandler,
    std::unique_ptr<CPDFSDK_WidgetHandler> pWidgetHandler,
    std::unique_ptr<IPDFSDK_AnnotHandler> pXFAWidgetHandler)
    : m_pBAAnnotHandler(std::move(pBAAnnotHandler)),
      m_pWidgetHandler(std::move(pWidgetHandler)),
      m_pXFAWidgetHandler(std::move(pXFAWidgetHandler)) {
  DCHECK(m_pBAAnnotHandler);
  DCHECK(m_pWidgetHandler);
}

CPDFSDK_AnnotHandlerMgr::~CPDFSDK_AnnotHandlerMgr() = default;

void CPDFSDK_AnnotHandlerMgr::SetFormFillEnv(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  m_pBAAnnotHandler->SetFormFillEnvironment(pFormFillEnv);
  m_pWidgetHandler->SetFormFillEnvironment(pFormFillEnv);
  if (m_pXFAWidgetHandler)
    m_pXFAWidgetHandler->SetFormFillEnvironment(pFormFillEnv);
}

std::unique_ptr<CPDFSDK_Annot> CPDFSDK_AnnotHandlerMgr::NewAnnot(
    CPDF_Annot* pAnnot,
    CPDFSDK_PageView* pPageView) {
  DCHECK(pPageView);
  return GetAnnotHandlerOfType(pAnnot->GetSubtype())
      ->NewAnnot(pAnnot, pPageView);
}

#ifdef PDF_ENABLE_XFA
std::unique_ptr<CPDFSDK_Annot> CPDFSDK_AnnotHandlerMgr::NewXFAAnnot(
    CXFA_FFWidget* pAnnot,
    CPDFSDK_PageView* pPageView) {
  DCHECK(pAnnot);
  DCHECK(pPageView);
  return static_cast<CPDFXFA_WidgetHandler*>(m_pXFAWidgetHandler.get())
      ->NewAnnotForXFA(pAnnot, pPageView);
}
#endif  // PDF_ENABLE_XFA

void CPDFSDK_AnnotHandlerMgr::ReleaseAnnot(
    std::unique_ptr<CPDFSDK_Annot> pAnnot) {
  IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot.get());
  pAnnotHandler->ReleaseAnnot(std::move(pAnnot));
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnLoad(CPDFSDK_Annot* pAnnot) {
  DCHECK(pAnnot);
  GetAnnotHandler(pAnnot)->OnLoad(pAnnot);
}

WideString CPDFSDK_AnnotHandlerMgr::Annot_GetText(CPDFSDK_Annot* pAnnot) {
  return GetAnnotHandler(pAnnot)->GetText(pAnnot);
}

WideString CPDFSDK_AnnotHandlerMgr::Annot_GetSelectedText(
    CPDFSDK_Annot* pAnnot) {
  return GetAnnotHandler(pAnnot)->GetSelectedText(pAnnot);
}

void CPDFSDK_AnnotHandlerMgr::Annot_ReplaceSelection(CPDFSDK_Annot* pAnnot,
                                                     const WideString& text) {
  GetAnnotHandler(pAnnot)->ReplaceSelection(pAnnot, text);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_SelectAllText(CPDFSDK_Annot* pAnnot) {
  return GetAnnotHandler(pAnnot)->SelectAllText(pAnnot);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_CanUndo(CPDFSDK_Annot* pAnnot) {
  return GetAnnotHandler(pAnnot)->CanUndo(pAnnot);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_CanRedo(CPDFSDK_Annot* pAnnot) {
  return GetAnnotHandler(pAnnot)->CanRedo(pAnnot);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_Undo(CPDFSDK_Annot* pAnnot) {
  return GetAnnotHandler(pAnnot)->Undo(pAnnot);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_Redo(CPDFSDK_Annot* pAnnot) {
  return GetAnnotHandler(pAnnot)->Redo(pAnnot);
}

IPDFSDK_AnnotHandler* CPDFSDK_AnnotHandlerMgr::GetAnnotHandler(
    CPDFSDK_Annot* pAnnot) const {
  return GetAnnotHandlerOfType(pAnnot->GetAnnotSubtype());
}

IPDFSDK_AnnotHandler* CPDFSDK_AnnotHandlerMgr::GetAnnotHandlerOfType(
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
                                           const CFX_Matrix& mtUser2Device,
                                           bool bDrawAnnots) {
  DCHECK(pAnnot);
  GetAnnotHandler(pAnnot)->OnDraw(pPageView, pAnnot, pDevice, mtUser2Device,
                                  bDrawAnnots);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnLButtonDown(
    CPDFSDK_PageView* pPageView,
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    uint32_t nFlags,
    const CFX_PointF& point) {
  DCHECK(pAnnot->HasObservable());
  return GetAnnotHandler(pAnnot->Get())
      ->OnLButtonDown(pPageView, pAnnot, nFlags, point);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnLButtonUp(
    CPDFSDK_PageView* pPageView,
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    uint32_t nFlags,
    const CFX_PointF& point) {
  DCHECK(pAnnot->HasObservable());
  return GetAnnotHandler(pAnnot->Get())
      ->OnLButtonUp(pPageView, pAnnot, nFlags, point);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnLButtonDblClk(
    CPDFSDK_PageView* pPageView,
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    uint32_t nFlags,
    const CFX_PointF& point) {
  DCHECK(pAnnot->HasObservable());
  return GetAnnotHandler(pAnnot->Get())
      ->OnLButtonDblClk(pPageView, pAnnot, nFlags, point);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnMouseMove(
    CPDFSDK_PageView* pPageView,
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    uint32_t nFlags,
    const CFX_PointF& point) {
  DCHECK(pAnnot->HasObservable());
  return GetAnnotHandler(pAnnot->Get())
      ->OnMouseMove(pPageView, pAnnot, nFlags, point);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnMouseWheel(
    CPDFSDK_PageView* pPageView,
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    uint32_t nFlags,
    const CFX_PointF& point,
    const CFX_Vector& delta) {
  DCHECK(pAnnot->HasObservable());
  auto* handler = GetAnnotHandler(pAnnot->Get());
  return handler->OnMouseWheel(pPageView, pAnnot, nFlags, point, delta);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnRButtonDown(
    CPDFSDK_PageView* pPageView,
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    uint32_t nFlags,
    const CFX_PointF& point) {
  DCHECK(pAnnot->HasObservable());
  return GetAnnotHandler(pAnnot->Get())
      ->OnRButtonDown(pPageView, pAnnot, nFlags, point);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnRButtonUp(
    CPDFSDK_PageView* pPageView,
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    uint32_t nFlags,
    const CFX_PointF& point) {
  DCHECK(pAnnot->HasObservable());
  return GetAnnotHandler(pAnnot->Get())
      ->OnRButtonUp(pPageView, pAnnot, nFlags, point);
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnMouseEnter(
    CPDFSDK_PageView* pPageView,
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    uint32_t nFlag) {
  DCHECK(pAnnot->HasObservable());
  GetAnnotHandler(pAnnot->Get())->OnMouseEnter(pPageView, pAnnot, nFlag);
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnMouseExit(
    CPDFSDK_PageView* pPageView,
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    uint32_t nFlag) {
  DCHECK(pAnnot->HasObservable());
  GetAnnotHandler(pAnnot->Get())->OnMouseExit(pPageView, pAnnot, nFlag);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnChar(CPDFSDK_Annot* pAnnot,
                                           uint32_t nChar,
                                           uint32_t nFlags) {
  return GetAnnotHandler(pAnnot)->OnChar(pAnnot, nChar, nFlags);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnKeyDown(CPDFSDK_PageView* pPageView,
                                              CPDFSDK_Annot* pAnnot,
                                              FWL_VKEYCODE nKeyCode,
                                              int nFlag) {
  if (!pAnnot) {
    // If pressed key is not tab then no action is needed.
    if (nKeyCode != FWL_VKEY_Tab)
      return false;

    // If ctrl key or alt key is pressed, then no action is needed.
    if (CPWL_Wnd::IsCTRLKeyDown(nFlag) || CPWL_Wnd::IsALTKeyDown(nFlag))
      return false;

    ObservedPtr<CPDFSDK_Annot> end_annot(
        CPWL_Wnd::IsSHIFTKeyDown(nFlag) ? GetLastFocusableAnnot(pPageView)
                                        : GetFirstFocusableAnnot(pPageView));
    return end_annot && pPageView->GetFormFillEnv()->SetFocusAnnot(&end_annot);
  }

  if (CPWL_Wnd::IsCTRLKeyDown(nFlag) || CPWL_Wnd::IsALTKeyDown(nFlag)) {
    return GetAnnotHandler(pAnnot)->OnKeyDown(pAnnot, nKeyCode, nFlag);
  }
  ObservedPtr<CPDFSDK_Annot> pObservedAnnot(pAnnot);
  CPDFSDK_Annot* pFocusAnnot = pPageView->GetFocusAnnot();
  if (pFocusAnnot && (nKeyCode == FWL_VKEY_Tab)) {
    ObservedPtr<CPDFSDK_Annot> pNext(CPWL_Wnd::IsSHIFTKeyDown(nFlag)
                                         ? GetPrevAnnot(pFocusAnnot)
                                         : GetNextAnnot(pFocusAnnot));
    if (!pNext)
      return false;
    if (pNext.Get() != pFocusAnnot) {
      pPageView->GetFormFillEnv()->SetFocusAnnot(&pNext);
      return true;
    }
  }

  // Check |pAnnot| again because JS may have destroyed it in |GetNextAnnot|
  if (!pObservedAnnot)
    return false;

  return GetAnnotHandler(pAnnot)->OnKeyDown(pAnnot, nKeyCode, nFlag);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnSetFocus(
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    uint32_t nFlag) {
  DCHECK(pAnnot->HasObservable());
  return GetAnnotHandler(pAnnot->Get())->OnSetFocus(pAnnot, nFlag);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnKillFocus(
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    uint32_t nFlag) {
  DCHECK(pAnnot->HasObservable());
  return GetAnnotHandler(pAnnot->Get())->OnKillFocus(pAnnot, nFlag);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_SetIndexSelected(
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    int index,
    bool selected) {
  return GetAnnotHandler(pAnnot->Get())
      ->SetIndexSelected(pAnnot, index, selected);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_IsIndexSelected(
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    int index) {
  return GetAnnotHandler(pAnnot->Get())->IsIndexSelected(pAnnot, index);
}

#ifdef PDF_ENABLE_XFA
bool CPDFSDK_AnnotHandlerMgr::Annot_OnChangeFocus(
    ObservedPtr<CPDFSDK_Annot>* pSetAnnot,
    ObservedPtr<CPDFSDK_Annot>* pKillAnnot) {
  CPDFXFA_Widget* pSetXFAWidget = ToXFAWidget(pSetAnnot->Get());
  CPDFXFA_Widget* pKillXFAWidget = ToXFAWidget(pKillAnnot->Get());
  bool bXFA = (pSetXFAWidget && pSetXFAWidget->GetXFAFFWidget()) ||
              (pKillXFAWidget && pKillXFAWidget->GetXFAFFWidget());

  return !bXFA || static_cast<CPDFXFA_WidgetHandler*>(m_pXFAWidgetHandler.get())
                      ->OnXFAChangedFocus(pKillAnnot, pSetAnnot);
}
#endif  // PDF_ENABLE_XFA

CFX_FloatRect CPDFSDK_AnnotHandlerMgr::Annot_OnGetViewBBox(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot) {
  DCHECK(pAnnot);
  return GetAnnotHandler(pAnnot)->GetViewBBox(pPageView, pAnnot);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnHitTest(CPDFSDK_PageView* pPageView,
                                              CPDFSDK_Annot* pAnnot,
                                              const CFX_PointF& point) {
  DCHECK(pAnnot);
  IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot);
  if (pAnnotHandler->CanAnswer(pAnnot))
    return pAnnotHandler->HitTest(pPageView, pAnnot, point);

  return false;
}

CPDFSDK_Annot* CPDFSDK_AnnotHandlerMgr::GetNextAnnot(
    CPDFSDK_Annot* pSDKAnnot) const {
  CPDFSDK_PageView* pPageView = pSDKAnnot->GetPageView();
#ifdef PDF_ENABLE_XFA
  CPDFXFA_Page* pXFAPage = XFAPageIfNotBackedByPDFPage(pPageView);
  if (pXFAPage)
    return pXFAPage->GetNextXFAAnnot(pSDKAnnot);
#endif  // PDF_ENABLE_XFA
  CPDFSDK_AnnotIterator ai(
      pPageView, pPageView->GetFormFillEnv()->GetFocusableAnnotSubtypes());
  return ai.GetNextAnnot(pSDKAnnot);
}

CPDFSDK_Annot* CPDFSDK_AnnotHandlerMgr::GetPrevAnnot(
    CPDFSDK_Annot* pSDKAnnot) const {
  CPDFSDK_PageView* pPageView = pSDKAnnot->GetPageView();
#ifdef PDF_ENABLE_XFA
  CPDFXFA_Page* pXFAPage = XFAPageIfNotBackedByPDFPage(pPageView);
  if (pXFAPage)
    return pXFAPage->GetPrevXFAAnnot(pSDKAnnot);
#endif  // PDF_ENABLE_XFA
  CPDFSDK_AnnotIterator ai(
      pPageView, pPageView->GetFormFillEnv()->GetFocusableAnnotSubtypes());
  return ai.GetPrevAnnot(pSDKAnnot);
}

CPDFSDK_Annot* CPDFSDK_AnnotHandlerMgr::GetFirstFocusableAnnot(
    CPDFSDK_PageView* page_view) const {
#ifdef PDF_ENABLE_XFA
  CPDFXFA_Page* pXFAPage = XFAPageIfNotBackedByPDFPage(page_view);
  if (pXFAPage)
    return pXFAPage->GetFirstXFAAnnot(page_view);
#endif  // PDF_ENABLE_XFA
  CPDFSDK_AnnotIterator ai(
      page_view, page_view->GetFormFillEnv()->GetFocusableAnnotSubtypes());
  return ai.GetFirstAnnot();
}

CPDFSDK_Annot* CPDFSDK_AnnotHandlerMgr::GetLastFocusableAnnot(
    CPDFSDK_PageView* page_view) const {
#ifdef PDF_ENABLE_XFA
  CPDFXFA_Page* pXFAPage = XFAPageIfNotBackedByPDFPage(page_view);
  if (pXFAPage)
    return pXFAPage->GetLastXFAAnnot(page_view);
#endif  // PDF_ENABLE_XFA
  CPDFSDK_AnnotIterator ai(
      page_view, page_view->GetFormFillEnv()->GetFocusableAnnotSubtypes());
  return ai.GetLastAnnot();
}
