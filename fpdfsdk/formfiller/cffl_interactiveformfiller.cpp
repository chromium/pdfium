// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/formfiller/cffl_interactiveformfiller.h"

#include "constants/access_permissions.h"
#include "constants/form_flags.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fxcrt/autorestorer.h"
#include "core/fxge/cfx_drawutils.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_interactiveform.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/formfiller/cffl_checkbox.h"
#include "fpdfsdk/formfiller/cffl_combobox.h"
#include "fpdfsdk/formfiller/cffl_formfiller.h"
#include "fpdfsdk/formfiller/cffl_listbox.h"
#include "fpdfsdk/formfiller/cffl_privatedata.h"
#include "fpdfsdk/formfiller/cffl_pushbutton.h"
#include "fpdfsdk/formfiller/cffl_radiobutton.h"
#include "fpdfsdk/formfiller/cffl_textfield.h"
#include "public/fpdf_fwlevent.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"
#include "third_party/base/stl_util.h"

CFFL_InteractiveFormFiller::CFFL_InteractiveFormFiller(
    CPDFSDK_FormFillEnvironment* pFormFillEnv)
    : m_pFormFillEnv(pFormFillEnv) {}

CFFL_InteractiveFormFiller::~CFFL_InteractiveFormFiller() = default;

bool CFFL_InteractiveFormFiller::Annot_HitTest(CPDFSDK_PageView* pPageView,
                                               CPDFSDK_Annot* pAnnot,
                                               const CFX_PointF& point) {
  return pAnnot->GetRect().Contains(point);
}

FX_RECT CFFL_InteractiveFormFiller::GetViewBBox(CPDFSDK_PageView* pPageView,
                                                CPDFSDK_Annot* pAnnot) {
  if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot))
    return pFormFiller->GetViewBBox(pPageView);

  DCHECK(pPageView);

  CPDF_Annot* pPDFAnnot = pAnnot->GetPDFAnnot();
  CFX_FloatRect rcWin = pPDFAnnot->GetRect();
  if (!rcWin.IsEmpty()) {
    rcWin.Inflate(1, 1);
    rcWin.Normalize();
  }
  return rcWin.GetOuterRect();
}

void CFFL_InteractiveFormFiller::OnDraw(CPDFSDK_PageView* pPageView,
                                        CPDFSDK_Annot* pAnnot,
                                        CFX_RenderDevice* pDevice,
                                        const CFX_Matrix& mtUser2Device) {
  DCHECK(pPageView);
  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot);
  if (!IsVisible(pWidget))
    return;

  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot);
  if (pFormFiller && pFormFiller->IsValid()) {
    pFormFiller->OnDraw(pPageView, pAnnot, pDevice, mtUser2Device);
    if (m_pFormFillEnv->GetFocusAnnot() != pAnnot)
      return;

    CFX_FloatRect rcFocus = pFormFiller->GetFocusBox(pPageView);
    if (rcFocus.IsEmpty())
      return;

    CFX_DrawUtils::DrawFocusRect(pDevice, mtUser2Device, rcFocus);

    return;
  }

  if (pFormFiller) {
    pFormFiller->OnDrawDeactive(pPageView, pAnnot, pDevice, mtUser2Device);
  } else {
    pWidget->DrawAppearance(pDevice, mtUser2Device, CPDF_Annot::Normal,
                            nullptr);
  }

  if (!IsReadOnly(pWidget) && IsFillingAllowed(pWidget))
    pWidget->DrawShadow(pDevice, pPageView);
}

void CFFL_InteractiveFormFiller::OnDelete(CPDFSDK_Annot* pAnnot) {
  UnRegisterFormFiller(pAnnot);
}

void CFFL_InteractiveFormFiller::OnMouseEnter(
    CPDFSDK_PageView* pPageView,
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    uint32_t nFlag) {
  DCHECK_EQ((*pAnnot)->GetPDFAnnot()->GetSubtype(),
            CPDF_Annot::Subtype::WIDGET);
  if (!m_bNotifying) {
    CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot->Get());
    if (pWidget->GetAAction(CPDF_AAction::kCursorEnter).GetDict()) {
      m_bNotifying = true;

      uint32_t nValueAge = pWidget->GetValueAge();
      pWidget->ClearAppModified();
      DCHECK(pPageView);

      CPDFSDK_FieldAction fa;
      fa.bModifier = CPWL_Wnd::IsCTRLKeyDown(nFlag);
      fa.bShift = CPWL_Wnd::IsSHIFTKeyDown(nFlag);
      pWidget->OnAAction(CPDF_AAction::kCursorEnter, &fa, pPageView);
      m_bNotifying = false;
      if (!pAnnot->HasObservable())
        return;

      if (pWidget->IsAppModified()) {
        if (CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget)) {
          pFormFiller->ResetPWLWindow(pPageView,
                                      pWidget->GetValueAge() == nValueAge);
        }
      }
    }
  }
  if (CFFL_FormFiller* pFormFiller = GetOrCreateFormFiller(pAnnot->Get()))
    pFormFiller->OnMouseEnter(pPageView);
}

void CFFL_InteractiveFormFiller::OnMouseExit(CPDFSDK_PageView* pPageView,
                                             ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                             uint32_t nFlag) {
  DCHECK_EQ((*pAnnot)->GetPDFAnnot()->GetSubtype(),
            CPDF_Annot::Subtype::WIDGET);
  if (!m_bNotifying) {
    CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot->Get());
    if (pWidget->GetAAction(CPDF_AAction::kCursorExit).GetDict()) {
      m_bNotifying = true;

      uint32_t nValueAge = pWidget->GetValueAge();
      pWidget->ClearAppModified();
      DCHECK(pPageView);

      CPDFSDK_FieldAction fa;
      fa.bModifier = CPWL_Wnd::IsCTRLKeyDown(nFlag);
      fa.bShift = CPWL_Wnd::IsSHIFTKeyDown(nFlag);
      pWidget->OnAAction(CPDF_AAction::kCursorExit, &fa, pPageView);
      m_bNotifying = false;
      if (!pAnnot->HasObservable())
        return;

      if (pWidget->IsAppModified()) {
        if (CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget)) {
          pFormFiller->ResetPWLWindow(pPageView,
                                      nValueAge == pWidget->GetValueAge());
        }
      }
    }
  }
  if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot->Get()))
    pFormFiller->OnMouseExit(pPageView);
}

bool CFFL_InteractiveFormFiller::OnLButtonDown(
    CPDFSDK_PageView* pPageView,
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    uint32_t nFlags,
    const CFX_PointF& point) {
  DCHECK_EQ((*pAnnot)->GetPDFAnnot()->GetSubtype(),
            CPDF_Annot::Subtype::WIDGET);
  if (!m_bNotifying) {
    CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot->Get());
    if (Annot_HitTest(pPageView, pAnnot->Get(), point) &&
        pWidget->GetAAction(CPDF_AAction::kButtonDown).GetDict()) {
      m_bNotifying = true;

      uint32_t nValueAge = pWidget->GetValueAge();
      pWidget->ClearAppModified();
      DCHECK(pPageView);

      CPDFSDK_FieldAction fa;
      fa.bModifier = CPWL_Wnd::IsCTRLKeyDown(nFlags);
      fa.bShift = CPWL_Wnd::IsSHIFTKeyDown(nFlags);
      pWidget->OnAAction(CPDF_AAction::kButtonDown, &fa, pPageView);
      m_bNotifying = false;
      if (!pAnnot->HasObservable())
        return true;

      if (!IsValidAnnot(pPageView, pAnnot->Get()))
        return true;

      if (pWidget->IsAppModified()) {
        if (CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget)) {
          pFormFiller->ResetPWLWindow(pPageView,
                                      nValueAge == pWidget->GetValueAge());
        }
      }
    }
  }
  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot->Get());
  return pFormFiller &&
         pFormFiller->OnLButtonDown(pPageView, pAnnot->Get(), nFlags, point);
}

bool CFFL_InteractiveFormFiller::OnLButtonUp(CPDFSDK_PageView* pPageView,
                                             ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                             uint32_t nFlags,
                                             const CFX_PointF& point) {
  DCHECK_EQ((*pAnnot)->GetPDFAnnot()->GetSubtype(),
            CPDF_Annot::Subtype::WIDGET);
  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot->Get());

  bool bSetFocus;
  switch (pWidget->GetFieldType()) {
    case FormFieldType::kPushButton:
    case FormFieldType::kCheckBox:
    case FormFieldType::kRadioButton: {
      FX_RECT bbox = GetViewBBox(pPageView, pAnnot->Get());
      bSetFocus =
          bbox.Contains(static_cast<int>(point.x), static_cast<int>(point.y));
      break;
    }
    default:
      bSetFocus = true;
      break;
  }
  if (bSetFocus)
    m_pFormFillEnv->SetFocusAnnot(pAnnot);

  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot->Get());
  bool bRet = pFormFiller &&
              pFormFiller->OnLButtonUp(pPageView, pAnnot->Get(), nFlags, point);
  if (m_pFormFillEnv->GetFocusAnnot() != pAnnot->Get())
    return bRet;
  if (OnButtonUp(pAnnot, pPageView, nFlags) || !pAnnot)
    return true;
#ifdef PDF_ENABLE_XFA
  if (OnClick(pAnnot, pPageView, nFlags) || !pAnnot)
    return true;
#endif  // PDF_ENABLE_XFA
  return bRet;
}

bool CFFL_InteractiveFormFiller::OnButtonUp(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                            CPDFSDK_PageView* pPageView,
                                            uint32_t nFlag) {
  if (m_bNotifying)
    return false;

  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot->Get());
  if (!pWidget->GetAAction(CPDF_AAction::kButtonUp).GetDict())
    return false;

  m_bNotifying = true;

  uint32_t nAge = pWidget->GetAppearanceAge();
  uint32_t nValueAge = pWidget->GetValueAge();
  DCHECK(pPageView);

  CPDFSDK_FieldAction fa;
  fa.bModifier = CPWL_Wnd::IsCTRLKeyDown(nFlag);
  fa.bShift = CPWL_Wnd::IsSHIFTKeyDown(nFlag);
  pWidget->OnAAction(CPDF_AAction::kButtonUp, &fa, pPageView);
  m_bNotifying = false;
  if (!pAnnot->HasObservable() || !IsValidAnnot(pPageView, pWidget))
    return true;
  if (nAge == pWidget->GetAppearanceAge())
    return false;

  CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget);
  if (pFormFiller)
    pFormFiller->ResetPWLWindow(pPageView, nValueAge == pWidget->GetValueAge());
  return true;
}

bool CFFL_InteractiveFormFiller::SetIndexSelected(
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    int index,
    bool selected) {
  DCHECK_EQ((*pAnnot)->GetPDFAnnot()->GetSubtype(),
            CPDF_Annot::Subtype::WIDGET);

  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot->Get());
  return pFormFiller && pFormFiller->SetIndexSelected(index, selected);
}

bool CFFL_InteractiveFormFiller::IsIndexSelected(
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    int index) {
  DCHECK_EQ((*pAnnot)->GetPDFAnnot()->GetSubtype(),
            CPDF_Annot::Subtype::WIDGET);

  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot->Get());
  return pFormFiller && pFormFiller->IsIndexSelected(index);
}

bool CFFL_InteractiveFormFiller::OnLButtonDblClk(
    CPDFSDK_PageView* pPageView,
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    uint32_t nFlags,
    const CFX_PointF& point) {
  DCHECK_EQ((*pAnnot)->GetPDFAnnot()->GetSubtype(),
            CPDF_Annot::Subtype::WIDGET);
  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot->Get());
  return pFormFiller && pFormFiller->OnLButtonDblClk(pPageView, nFlags, point);
}

bool CFFL_InteractiveFormFiller::OnMouseMove(CPDFSDK_PageView* pPageView,
                                             ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                             uint32_t nFlags,
                                             const CFX_PointF& point) {
  DCHECK_EQ((*pAnnot)->GetPDFAnnot()->GetSubtype(),
            CPDF_Annot::Subtype::WIDGET);
  CFFL_FormFiller* pFormFiller = GetOrCreateFormFiller(pAnnot->Get());
  return pFormFiller && pFormFiller->OnMouseMove(pPageView, nFlags, point);
}

bool CFFL_InteractiveFormFiller::OnMouseWheel(
    CPDFSDK_PageView* pPageView,
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    uint32_t nFlags,
    const CFX_PointF& point,
    const CFX_Vector& delta) {
  DCHECK_EQ((*pAnnot)->GetPDFAnnot()->GetSubtype(),
            CPDF_Annot::Subtype::WIDGET);
  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot->Get());
  return pFormFiller &&
         pFormFiller->OnMouseWheel(pPageView, nFlags, point, delta);
}

bool CFFL_InteractiveFormFiller::OnRButtonDown(
    CPDFSDK_PageView* pPageView,
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    uint32_t nFlags,
    const CFX_PointF& point) {
  DCHECK_EQ((*pAnnot)->GetPDFAnnot()->GetSubtype(),
            CPDF_Annot::Subtype::WIDGET);
  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot->Get());
  return pFormFiller && pFormFiller->OnRButtonDown(pPageView, nFlags, point);
}

bool CFFL_InteractiveFormFiller::OnRButtonUp(CPDFSDK_PageView* pPageView,
                                             ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                             uint32_t nFlags,
                                             const CFX_PointF& point) {
  DCHECK_EQ((*pAnnot)->GetPDFAnnot()->GetSubtype(),
            CPDF_Annot::Subtype::WIDGET);
  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot->Get());
  return pFormFiller && pFormFiller->OnRButtonUp(pPageView, nFlags, point);
}

bool CFFL_InteractiveFormFiller::OnKeyDown(CPDFSDK_Annot* pAnnot,
                                           uint32_t nKeyCode,
                                           uint32_t nFlags) {
  DCHECK_EQ(pAnnot->GetPDFAnnot()->GetSubtype(), CPDF_Annot::Subtype::WIDGET);

  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot);
  return pFormFiller && pFormFiller->OnKeyDown(nKeyCode, nFlags);
}

bool CFFL_InteractiveFormFiller::OnChar(CPDFSDK_Annot* pAnnot,
                                        uint32_t nChar,
                                        uint32_t nFlags) {
  DCHECK_EQ(pAnnot->GetPDFAnnot()->GetSubtype(), CPDF_Annot::Subtype::WIDGET);
  if (nChar == FWL_VKEY_Tab)
    return true;

  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot);
  return pFormFiller && pFormFiller->OnChar(pAnnot, nChar, nFlags);
}

bool CFFL_InteractiveFormFiller::OnSetFocus(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                            uint32_t nFlag) {
  if (!pAnnot->HasObservable())
    return false;

  DCHECK_EQ((*pAnnot)->GetPDFAnnot()->GetSubtype(),
            CPDF_Annot::Subtype::WIDGET);
  if (!m_bNotifying) {
    CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot->Get());
    if (pWidget->GetAAction(CPDF_AAction::kGetFocus).GetDict()) {
      m_bNotifying = true;

      uint32_t nValueAge = pWidget->GetValueAge();
      pWidget->ClearAppModified();

      CFFL_FormFiller* pFormFiller = GetOrCreateFormFiller(pWidget);
      if (!pFormFiller)
        return false;

      CPDFSDK_PageView* pPageView = (*pAnnot)->GetPageView();
      DCHECK(pPageView);

      CPDFSDK_FieldAction fa;
      fa.bModifier = CPWL_Wnd::IsCTRLKeyDown(nFlag);
      fa.bShift = CPWL_Wnd::IsSHIFTKeyDown(nFlag);
      pFormFiller->GetActionData(pPageView, CPDF_AAction::kGetFocus, fa);
      pWidget->OnAAction(CPDF_AAction::kGetFocus, &fa, pPageView);
      m_bNotifying = false;
      if (!pAnnot->HasObservable())
        return false;

      if (pWidget->IsAppModified()) {
        if (CFFL_FormFiller* pFiller = GetFormFiller(pWidget)) {
          pFiller->ResetPWLWindow(pPageView,
                                  nValueAge == pWidget->GetValueAge());
        }
      }
    }
  }

  if (CFFL_FormFiller* pFormFiller = GetOrCreateFormFiller(pAnnot->Get()))
    pFormFiller->SetFocusForAnnot(pAnnot->Get(), nFlag);

  return true;
}

bool CFFL_InteractiveFormFiller::OnKillFocus(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                             uint32_t nFlag) {
  if (!pAnnot->HasObservable())
    return false;

  DCHECK_EQ((*pAnnot)->GetPDFAnnot()->GetSubtype(),
            CPDF_Annot::Subtype::WIDGET);
  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot->Get());
  if (!pFormFiller)
    return true;

  pFormFiller->KillFocusForAnnot(nFlag);
  if (!pAnnot->HasObservable())
    return false;

  if (m_bNotifying)
    return true;

  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot->Get());
  if (!pWidget->GetAAction(CPDF_AAction::kLoseFocus).GetDict())
    return true;

  m_bNotifying = true;
  pWidget->ClearAppModified();

  CPDFSDK_PageView* pPageView = pWidget->GetPageView();
  DCHECK(pPageView);

  CPDFSDK_FieldAction fa;
  fa.bModifier = CPWL_Wnd::IsCTRLKeyDown(nFlag);
  fa.bShift = CPWL_Wnd::IsSHIFTKeyDown(nFlag);
  pFormFiller->GetActionData(pPageView, CPDF_AAction::kLoseFocus, fa);
  pWidget->OnAAction(CPDF_AAction::kLoseFocus, &fa, pPageView);
  m_bNotifying = false;
  return pAnnot->HasObservable();
}

bool CFFL_InteractiveFormFiller::IsVisible(CPDFSDK_Widget* pWidget) {
  return pWidget->IsVisible();
}

bool CFFL_InteractiveFormFiller::IsReadOnly(CPDFSDK_Widget* pWidget) {
  int nFieldFlags = pWidget->GetFieldFlags();
  return !!(nFieldFlags & pdfium::form_flags::kReadOnly);
}

bool CFFL_InteractiveFormFiller::IsFillingAllowed(
    CPDFSDK_Widget* pWidget) const {
  if (pWidget->GetFieldType() == FormFieldType::kPushButton)
    return false;

  return m_pFormFillEnv->HasPermissions(
      pdfium::access_permissions::kFillForm |
      pdfium::access_permissions::kModifyAnnotation |
      pdfium::access_permissions::kModifyContent);
}

CFFL_FormFiller* CFFL_InteractiveFormFiller::GetFormFiller(
    CPDFSDK_Annot* pAnnot) {
  auto it = m_Map.find(pAnnot);
  return it != m_Map.end() ? it->second.get() : nullptr;
}

CFFL_FormFiller* CFFL_InteractiveFormFiller::GetOrCreateFormFiller(
    CPDFSDK_Annot* pAnnot) {
  CFFL_FormFiller* result = GetFormFiller(pAnnot);
  if (result)
    return result;

  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot);
  std::unique_ptr<CFFL_FormFiller> pFormFiller;
  switch (pWidget->GetFieldType()) {
    case FormFieldType::kPushButton:
      pFormFiller =
          std::make_unique<CFFL_PushButton>(m_pFormFillEnv.Get(), pWidget);
      break;
    case FormFieldType::kCheckBox:
      pFormFiller =
          std::make_unique<CFFL_CheckBox>(m_pFormFillEnv.Get(), pWidget);
      break;
    case FormFieldType::kRadioButton:
      pFormFiller =
          std::make_unique<CFFL_RadioButton>(m_pFormFillEnv.Get(), pWidget);
      break;
    case FormFieldType::kTextField:
      pFormFiller =
          std::make_unique<CFFL_TextField>(m_pFormFillEnv.Get(), pWidget);
      break;
    case FormFieldType::kListBox:
      pFormFiller =
          std::make_unique<CFFL_ListBox>(m_pFormFillEnv.Get(), pWidget);
      break;
    case FormFieldType::kComboBox:
      pFormFiller =
          std::make_unique<CFFL_ComboBox>(m_pFormFillEnv.Get(), pWidget);
      break;
    case FormFieldType::kUnknown:
    default:
      return nullptr;
  }

  result = pFormFiller.get();
  m_Map[pAnnot] = std::move(pFormFiller);
  return result;
}

WideString CFFL_InteractiveFormFiller::GetText(CPDFSDK_Annot* pAnnot) {
  DCHECK_EQ(pAnnot->GetPDFAnnot()->GetSubtype(), CPDF_Annot::Subtype::WIDGET);
  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot);
  return pFormFiller ? pFormFiller->GetText() : WideString();
}

WideString CFFL_InteractiveFormFiller::GetSelectedText(CPDFSDK_Annot* pAnnot) {
  DCHECK_EQ(pAnnot->GetPDFAnnot()->GetSubtype(), CPDF_Annot::Subtype::WIDGET);
  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot);
  return pFormFiller ? pFormFiller->GetSelectedText() : WideString();
}

void CFFL_InteractiveFormFiller::ReplaceSelection(CPDFSDK_Annot* pAnnot,
                                                  const WideString& text) {
  DCHECK_EQ(pAnnot->GetPDFAnnot()->GetSubtype(), CPDF_Annot::Subtype::WIDGET);
  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot);
  if (!pFormFiller)
    return;

  pFormFiller->ReplaceSelection(text);
}

bool CFFL_InteractiveFormFiller::SelectAllText(CPDFSDK_Annot* pAnnot) {
  DCHECK_EQ(pAnnot->GetPDFAnnot()->GetSubtype(), CPDF_Annot::Subtype::WIDGET);
  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot);
  return pAnnot && pFormFiller->SelectAllText();
}

bool CFFL_InteractiveFormFiller::CanUndo(CPDFSDK_Annot* pAnnot) {
  DCHECK_EQ(pAnnot->GetPDFAnnot()->GetSubtype(), CPDF_Annot::Subtype::WIDGET);
  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot);
  return pFormFiller && pFormFiller->CanUndo();
}

bool CFFL_InteractiveFormFiller::CanRedo(CPDFSDK_Annot* pAnnot) {
  DCHECK_EQ(pAnnot->GetPDFAnnot()->GetSubtype(), CPDF_Annot::Subtype::WIDGET);
  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot);
  return pFormFiller && pFormFiller->CanRedo();
}

bool CFFL_InteractiveFormFiller::Undo(CPDFSDK_Annot* pAnnot) {
  DCHECK_EQ(pAnnot->GetPDFAnnot()->GetSubtype(), CPDF_Annot::Subtype::WIDGET);
  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot);
  return pFormFiller && pFormFiller->Undo();
}

bool CFFL_InteractiveFormFiller::Redo(CPDFSDK_Annot* pAnnot) {
  DCHECK_EQ(pAnnot->GetPDFAnnot()->GetSubtype(), CPDF_Annot::Subtype::WIDGET);
  CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot);
  return pFormFiller && pFormFiller->Redo();
}

void CFFL_InteractiveFormFiller::UnRegisterFormFiller(CPDFSDK_Annot* pAnnot) {
  auto it = m_Map.find(pAnnot);
  if (it == m_Map.end())
    return;

  m_Map.erase(it);
}

void CFFL_InteractiveFormFiller::QueryWherePopup(
    const IPWL_SystemHandler::PerWindowData* pAttached,
    float fPopupMin,
    float fPopupMax,
    bool* bBottom,
    float* fPopupRet) {
  auto* pData = static_cast<const CFFL_PrivateData*>(pAttached);
  CPDFSDK_Widget* pWidget = pData->GetWidget();
  CPDF_Page* pPage = pWidget->GetPDFPage();

  CFX_FloatRect rcPageView(0, pPage->GetPageHeight(), pPage->GetPageWidth(), 0);
  rcPageView.Normalize();

  CFX_FloatRect rcAnnot = pWidget->GetRect();
  float fTop = 0.0f;
  float fBottom = 0.0f;
  switch (pWidget->GetRotate() / 90) {
    default:
    case 0:
      fTop = rcPageView.top - rcAnnot.top;
      fBottom = rcAnnot.bottom - rcPageView.bottom;
      break;
    case 1:
      fTop = rcAnnot.left - rcPageView.left;
      fBottom = rcPageView.right - rcAnnot.right;
      break;
    case 2:
      fTop = rcAnnot.bottom - rcPageView.bottom;
      fBottom = rcPageView.top - rcAnnot.top;
      break;
    case 3:
      fTop = rcPageView.right - rcAnnot.right;
      fBottom = rcAnnot.left - rcPageView.left;
      break;
  }

  constexpr float kMaxListBoxHeight = 140;
  const float fMaxListBoxHeight =
      pdfium::clamp(kMaxListBoxHeight, fPopupMin, fPopupMax);

  if (fBottom > fMaxListBoxHeight) {
    *fPopupRet = fMaxListBoxHeight;
    *bBottom = true;
    return;
  }

  if (fTop > fMaxListBoxHeight) {
    *fPopupRet = fMaxListBoxHeight;
    *bBottom = false;
    return;
  }

  if (fTop > fBottom) {
    *fPopupRet = fTop;
    *bBottom = false;
  } else {
    *fPopupRet = fBottom;
    *bBottom = true;
  }
}

bool CFFL_InteractiveFormFiller::OnKeyStrokeCommit(
    ObservedPtr<CPDFSDK_Annot>* pAnnot,
    CPDFSDK_PageView* pPageView,
    uint32_t nFlag) {
  if (m_bNotifying)
    return true;

  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot->Get());
  if (!pWidget->GetAAction(CPDF_AAction::kKeyStroke).GetDict())
    return true;

  DCHECK(pPageView);
  m_bNotifying = true;
  pWidget->ClearAppModified();

  CPDFSDK_FieldAction fa;
  fa.bModifier = CPWL_Wnd::IsCTRLKeyDown(nFlag);
  fa.bShift = CPWL_Wnd::IsSHIFTKeyDown(nFlag);
  fa.bWillCommit = true;
  fa.bKeyDown = true;
  fa.bRC = true;

  CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget);
  pFormFiller->GetActionData(pPageView, CPDF_AAction::kKeyStroke, fa);
  pFormFiller->SaveState(pPageView);
  pWidget->OnAAction(CPDF_AAction::kKeyStroke, &fa, pPageView);
  if (!pAnnot->HasObservable())
    return true;

  m_bNotifying = false;
  return fa.bRC;
}

bool CFFL_InteractiveFormFiller::OnValidate(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                            CPDFSDK_PageView* pPageView,
                                            uint32_t nFlag) {
  if (m_bNotifying)
    return true;

  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot->Get());
  if (!pWidget->GetAAction(CPDF_AAction::kValidate).GetDict())
    return true;

  DCHECK(pPageView);
  m_bNotifying = true;
  pWidget->ClearAppModified();

  CPDFSDK_FieldAction fa;
  fa.bModifier = CPWL_Wnd::IsCTRLKeyDown(nFlag);
  fa.bShift = CPWL_Wnd::IsSHIFTKeyDown(nFlag);
  fa.bKeyDown = true;
  fa.bRC = true;

  CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget);
  pFormFiller->GetActionData(pPageView, CPDF_AAction::kValidate, fa);
  pFormFiller->SaveState(pPageView);
  pWidget->OnAAction(CPDF_AAction::kValidate, &fa, pPageView);
  if (!pAnnot->HasObservable())
    return true;

  m_bNotifying = false;
  return fa.bRC;
}

void CFFL_InteractiveFormFiller::OnCalculate(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                             CPDFSDK_PageView* pPageView,
                                             uint32_t nFlag) {
  if (m_bNotifying)
    return;

  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot->Get());
  if (pWidget) {
    CPDFSDK_InteractiveForm* pForm =
        pPageView->GetFormFillEnv()->GetInteractiveForm();
    pForm->OnCalculate(pWidget->GetFormField());
  }
  m_bNotifying = false;
}

void CFFL_InteractiveFormFiller::OnFormat(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                          CPDFSDK_PageView* pPageView,
                                          uint32_t nFlag) {
  if (m_bNotifying)
    return;

  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot->Get());
  DCHECK(pWidget);
  CPDFSDK_InteractiveForm* pForm =
      pPageView->GetFormFillEnv()->GetInteractiveForm();

  Optional<WideString> sValue = pForm->OnFormat(pWidget->GetFormField());
  if (!pAnnot->HasObservable())
    return;

  if (sValue.has_value()) {
    pForm->ResetFieldAppearance(pWidget->GetFormField(), sValue);
    pForm->UpdateField(pWidget->GetFormField());
  }

  m_bNotifying = false;
}

#ifdef PDF_ENABLE_XFA
bool CFFL_InteractiveFormFiller::OnClick(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                         CPDFSDK_PageView* pPageView,
                                         uint32_t nFlag) {
  if (m_bNotifying)
    return false;

  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot->Get());
  if (!pWidget->HasXFAAAction(PDFSDK_XFA_Click))
    return false;

  m_bNotifying = true;
  uint32_t nAge = pWidget->GetAppearanceAge();
  uint32_t nValueAge = pWidget->GetValueAge();

  CPDFSDK_FieldAction fa;
  fa.bModifier = CPWL_Wnd::IsCTRLKeyDown(nFlag);
  fa.bShift = CPWL_Wnd::IsSHIFTKeyDown(nFlag);

  pWidget->OnXFAAAction(PDFSDK_XFA_Click, &fa, pPageView);
  m_bNotifying = false;
  if (!pAnnot->HasObservable() || !IsValidAnnot(pPageView, pWidget))
    return true;
  if (nAge == pWidget->GetAppearanceAge())
    return false;

  if (CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget))
    pFormFiller->ResetPWLWindow(pPageView, nValueAge == pWidget->GetValueAge());
  return false;
}

bool CFFL_InteractiveFormFiller::OnFull(ObservedPtr<CPDFSDK_Widget>* pAnnot,
                                        CPDFSDK_PageView* pPageView,
                                        uint32_t nFlag) {
  if (m_bNotifying)
    return false;

  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot->Get());
  if (!pWidget->HasXFAAAction(PDFSDK_XFA_Full))
    return false;

  m_bNotifying = true;
  uint32_t nAge = pWidget->GetAppearanceAge();
  uint32_t nValueAge = pWidget->GetValueAge();

  CPDFSDK_FieldAction fa;
  fa.bModifier = CPWL_Wnd::IsCTRLKeyDown(nFlag);
  fa.bShift = CPWL_Wnd::IsSHIFTKeyDown(nFlag);

  pWidget->OnXFAAAction(PDFSDK_XFA_Full, &fa, pPageView);
  m_bNotifying = false;
  if (!pAnnot->HasObservable() || !IsValidAnnot(pPageView, pWidget))
    return true;
  if (nAge == pWidget->GetAppearanceAge())
    return false;

  if (CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget))
    pFormFiller->ResetPWLWindow(pPageView, nValueAge == pWidget->GetValueAge());

  return true;
}

bool CFFL_InteractiveFormFiller::OnPreOpen(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                           CPDFSDK_PageView* pPageView,
                                           uint32_t nFlag) {
  if (m_bNotifying)
    return false;

  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot->Get());
  if (!pWidget->HasXFAAAction(PDFSDK_XFA_PreOpen))
    return false;

  m_bNotifying = true;
  uint32_t nAge = pWidget->GetAppearanceAge();
  uint32_t nValueAge = pWidget->GetValueAge();

  CPDFSDK_FieldAction fa;
  fa.bModifier = CPWL_Wnd::IsCTRLKeyDown(nFlag);
  fa.bShift = CPWL_Wnd::IsSHIFTKeyDown(nFlag);

  pWidget->OnXFAAAction(PDFSDK_XFA_PreOpen, &fa, pPageView);
  m_bNotifying = false;
  if (!pAnnot->HasObservable() || !IsValidAnnot(pPageView, pWidget))
    return true;
  if (nAge == pWidget->GetAppearanceAge())
    return false;

  if (CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget))
    pFormFiller->ResetPWLWindow(pPageView, nValueAge == pWidget->GetValueAge());

  return true;
}

bool CFFL_InteractiveFormFiller::OnPostOpen(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                            CPDFSDK_PageView* pPageView,
                                            uint32_t nFlag) {
  if (m_bNotifying)
    return false;

  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot->Get());
  if (!pWidget->HasXFAAAction(PDFSDK_XFA_PostOpen))
    return false;

  m_bNotifying = true;
  uint32_t nAge = pWidget->GetAppearanceAge();
  uint32_t nValueAge = pWidget->GetValueAge();

  CPDFSDK_FieldAction fa;
  fa.bModifier = CPWL_Wnd::IsCTRLKeyDown(nFlag);
  fa.bShift = CPWL_Wnd::IsSHIFTKeyDown(nFlag);

  pWidget->OnXFAAAction(PDFSDK_XFA_PostOpen, &fa, pPageView);
  m_bNotifying = false;
  if (!pAnnot->HasObservable() || !IsValidAnnot(pPageView, pWidget))
    return true;
  if (nAge == pWidget->GetAppearanceAge())
    return false;

  if (CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget))
    pFormFiller->ResetPWLWindow(pPageView, nValueAge == pWidget->GetValueAge());

  return true;
}
#endif  // PDF_ENABLE_XFA

bool CFFL_InteractiveFormFiller::IsValidAnnot(CPDFSDK_PageView* pPageView,
                                              CPDFSDK_Annot* pAnnot) {
  return pPageView && pPageView->IsValidAnnot(pAnnot->GetPDFAnnot());
}

std::pair<bool, bool> CFFL_InteractiveFormFiller::OnBeforeKeyStroke(
    const IPWL_SystemHandler::PerWindowData* pAttached,
    WideString& strChange,
    const WideString& strChangeEx,
    int nSelStart,
    int nSelEnd,
    bool bKeyDown,
    uint32_t nFlag) {
  // Copy out of private data since the window owning it may not survive.
  auto* pPrivateData = static_cast<const CFFL_PrivateData*>(pAttached);
  CPDFSDK_PageView* pPageView = pPrivateData->GetPageView();
  ObservedPtr<CPDFSDK_Widget> pWidget(pPrivateData->GetWidget());
  DCHECK(pWidget);

  CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget.Get());

#ifdef PDF_ENABLE_XFA
  if (pFormFiller->IsFieldFull(pPageView)) {
    if (OnFull(&pWidget, pPageView, nFlag) || !pWidget)
      return {true, true};
  }
#endif  // PDF_ENABLE_XFA

  if (m_bNotifying ||
      !pWidget->GetAAction(CPDF_AAction::kKeyStroke).GetDict()) {
    return {true, false};
  }

  AutoRestorer<bool> restorer(&m_bNotifying);
  m_bNotifying = true;

  uint32_t nAge = pWidget->GetAppearanceAge();
  uint32_t nValueAge = pWidget->GetValueAge();
  CPDFSDK_FormFillEnvironment* pFormFillEnv = pPageView->GetFormFillEnv();

  CPDFSDK_FieldAction fa;
  fa.bModifier = CPWL_Wnd::IsCTRLKeyDown(nFlag);
  fa.bShift = CPWL_Wnd::IsSHIFTKeyDown(nFlag);
  fa.sChange = strChange;
  fa.sChangeEx = strChangeEx;
  fa.bKeyDown = bKeyDown;
  fa.bWillCommit = false;
  fa.bRC = true;
  fa.nSelStart = nSelStart;
  fa.nSelEnd = nSelEnd;
  pFormFiller->GetActionData(pPageView, CPDF_AAction::kKeyStroke, fa);
  pFormFiller->SaveState(pPageView);

  bool action_status =
      pWidget->OnAAction(CPDF_AAction::kKeyStroke, &fa, pPageView);

  if (!pWidget || !IsValidAnnot(pPageView, pWidget.Get())) {
    return {true, true};
  }
  if (!action_status)
    return {true, false};

  bool bExit = false;
  if (nAge != pWidget->GetAppearanceAge()) {
    CPWL_Wnd* pWnd = pFormFiller->ResetPWLWindow(
        pPageView, nValueAge == pWidget->GetValueAge());
    if (!pWnd)
      return {true, true};

    pPrivateData =
        static_cast<const CFFL_PrivateData*>(pWnd->GetAttachedData());
    pWidget.Reset(pPrivateData->GetWidget());
    pPageView = pPrivateData->GetPageView();
    bExit = true;
  }
  if (fa.bRC) {
    pFormFiller->SetActionData(pPageView, CPDF_AAction::kKeyStroke, fa);
  } else {
    pFormFiller->RestoreState(pPageView);
  }
  if (pFormFillEnv->GetFocusAnnot() == pWidget)
    return {false, bExit};

  pFormFiller->CommitData(pPageView, nFlag);
  return {false, true};
}

bool CFFL_InteractiveFormFiller::OnPopupPreOpen(
    const IPWL_SystemHandler::PerWindowData* pAttached,
    uint32_t nFlag) {
#ifdef PDF_ENABLE_XFA
  auto* pData = static_cast<const CFFL_PrivateData*>(pAttached);
  DCHECK(pData->GetWidget());

  ObservedPtr<CPDFSDK_Annot> pObserved(pData->GetWidget());
  return OnPreOpen(&pObserved, pData->GetPageView(), nFlag) || !pObserved;
#else
  return false;
#endif
}

bool CFFL_InteractiveFormFiller::OnPopupPostOpen(
    const IPWL_SystemHandler::PerWindowData* pAttached,
    uint32_t nFlag) {
#ifdef PDF_ENABLE_XFA
  auto* pData = static_cast<const CFFL_PrivateData*>(pAttached);
  DCHECK(pData->GetWidget());

  ObservedPtr<CPDFSDK_Annot> pObserved(pData->GetWidget());
  return OnPostOpen(&pObserved, pData->GetPageView(), nFlag) || !pObserved;
#else
  return false;
#endif
}