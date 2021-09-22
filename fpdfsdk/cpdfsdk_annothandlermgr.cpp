// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_annothandlermgr.h"

#include <utility>

#include "core/fpdfdoc/cpdf_annot.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/ipdfsdk_annothandler.h"
#include "third_party/base/check.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/cpdfxfa_widget.h"
#endif  // PDF_ENABLE_XFA

CPDFSDK_AnnotHandlerMgr::CPDFSDK_AnnotHandlerMgr(
    std::unique_ptr<IPDFSDK_AnnotHandler> pBAAnnotHandler,
    std::unique_ptr<IPDFSDK_AnnotHandler> pWidgetHandler,
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
std::unique_ptr<CPDFSDK_Annot> CPDFSDK_AnnotHandlerMgr::NewAnnotForXFA(
    CXFA_FFWidget* pFFWidget,
    CPDFSDK_PageView* pPageView) {
  DCHECK(pFFWidget);
  DCHECK(pPageView);
  return m_pXFAWidgetHandler->NewAnnotForXFA(pFFWidget, pPageView);
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

void CPDFSDK_AnnotHandlerMgr::Annot_OnDraw(CPDFSDK_Annot* pAnnot,
                                           CFX_RenderDevice* pDevice,
                                           const CFX_Matrix& mtUser2Device,
                                           bool bDrawAnnots) {
  DCHECK(pAnnot);
  GetAnnotHandler(pAnnot)->OnDraw(pAnnot, pDevice, mtUser2Device, bDrawAnnots);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnLButtonDown(
    ObservedPtr<CPDFSDK_Annot>& pAnnot,
    Mask<FWL_EVENTFLAG> nFlags,
    const CFX_PointF& point) {
  DCHECK(pAnnot);
  return GetAnnotHandler(pAnnot.Get())->OnLButtonDown(pAnnot, nFlags, point);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnLButtonUp(
    ObservedPtr<CPDFSDK_Annot>& pAnnot,
    Mask<FWL_EVENTFLAG> nFlags,
    const CFX_PointF& point) {
  DCHECK(pAnnot);
  return GetAnnotHandler(pAnnot.Get())->OnLButtonUp(pAnnot, nFlags, point);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnLButtonDblClk(
    ObservedPtr<CPDFSDK_Annot>& pAnnot,
    Mask<FWL_EVENTFLAG> nFlags,
    const CFX_PointF& point) {
  DCHECK(pAnnot);
  return GetAnnotHandler(pAnnot.Get())->OnLButtonDblClk(pAnnot, nFlags, point);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnMouseMove(
    ObservedPtr<CPDFSDK_Annot>& pAnnot,
    Mask<FWL_EVENTFLAG> nFlags,
    const CFX_PointF& point) {
  DCHECK(pAnnot);
  return GetAnnotHandler(pAnnot.Get())->OnMouseMove(pAnnot, nFlags, point);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnMouseWheel(
    ObservedPtr<CPDFSDK_Annot>& pAnnot,
    Mask<FWL_EVENTFLAG> nFlags,
    const CFX_PointF& point,
    const CFX_Vector& delta) {
  DCHECK(pAnnot);
  return GetAnnotHandler(pAnnot.Get())
      ->OnMouseWheel(pAnnot, nFlags, point, delta);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnRButtonDown(
    ObservedPtr<CPDFSDK_Annot>& pAnnot,
    Mask<FWL_EVENTFLAG> nFlags,
    const CFX_PointF& point) {
  DCHECK(pAnnot);
  return GetAnnotHandler(pAnnot.Get())->OnRButtonDown(pAnnot, nFlags, point);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnRButtonUp(
    ObservedPtr<CPDFSDK_Annot>& pAnnot,
    Mask<FWL_EVENTFLAG> nFlags,
    const CFX_PointF& point) {
  DCHECK(pAnnot);
  return GetAnnotHandler(pAnnot.Get())->OnRButtonUp(pAnnot, nFlags, point);
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnMouseEnter(
    ObservedPtr<CPDFSDK_Annot>& pAnnot,
    Mask<FWL_EVENTFLAG> nFlag) {
  DCHECK(pAnnot);
  GetAnnotHandler(pAnnot.Get())->OnMouseEnter(pAnnot, nFlag);
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnMouseExit(
    ObservedPtr<CPDFSDK_Annot>& pAnnot,
    Mask<FWL_EVENTFLAG> nFlag) {
  DCHECK(pAnnot);
  GetAnnotHandler(pAnnot.Get())->OnMouseExit(pAnnot, nFlag);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnChar(CPDFSDK_Annot* pAnnot,
                                           uint32_t nChar,
                                           Mask<FWL_EVENTFLAG> nFlags) {
  return GetAnnotHandler(pAnnot)->OnChar(pAnnot, nChar, nFlags);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnKeyDown(CPDFSDK_Annot* pAnnot,
                                              FWL_VKEYCODE nKeyCode,
                                              Mask<FWL_EVENTFLAG> nFlag) {
  return GetAnnotHandler(pAnnot)->OnKeyDown(pAnnot, nKeyCode, nFlag);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnSetFocus(
    ObservedPtr<CPDFSDK_Annot>& pAnnot,
    Mask<FWL_EVENTFLAG> nFlag) {
  DCHECK(pAnnot);
  return GetAnnotHandler(pAnnot.Get())->OnSetFocus(pAnnot, nFlag);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnKillFocus(
    ObservedPtr<CPDFSDK_Annot>& pAnnot,
    Mask<FWL_EVENTFLAG> nFlag) {
  DCHECK(pAnnot);
  return GetAnnotHandler(pAnnot.Get())->OnKillFocus(pAnnot, nFlag);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_SetIndexSelected(
    ObservedPtr<CPDFSDK_Annot>& pAnnot,
    int index,
    bool selected) {
  DCHECK(pAnnot);
  return GetAnnotHandler(pAnnot.Get())
      ->SetIndexSelected(pAnnot, index, selected);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_IsIndexSelected(
    ObservedPtr<CPDFSDK_Annot>& pAnnot,
    int index) {
  DCHECK(pAnnot);
  return GetAnnotHandler(pAnnot.Get())->IsIndexSelected(pAnnot, index);
}

#ifdef PDF_ENABLE_XFA
bool CPDFSDK_AnnotHandlerMgr::Annot_OnChangeFocus(
    ObservedPtr<CPDFSDK_Annot>& pSetAnnot) {
  CPDFXFA_Widget* pSetXFAWidget = ToXFAWidget(pSetAnnot.Get());
  const bool bXFA = pSetXFAWidget && pSetXFAWidget->GetXFAFFWidget();
  return !bXFA || m_pXFAWidgetHandler->OnXFAChangedFocus(pSetAnnot);
}
#endif  // PDF_ENABLE_XFA

CFX_FloatRect CPDFSDK_AnnotHandlerMgr::Annot_OnGetViewBBox(
    CPDFSDK_Annot* pAnnot) {
  DCHECK(pAnnot);
  return GetAnnotHandler(pAnnot)->GetViewBBox(pAnnot);
}

bool CPDFSDK_AnnotHandlerMgr::Annot_OnHitTest(CPDFSDK_Annot* pAnnot,
                                              const CFX_PointF& point) {
  DCHECK(pAnnot);
  IPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot);
  if (pAnnotHandler->CanAnswer(pAnnot))
    return pAnnotHandler->HitTest(pAnnot, point);

  return false;
}
