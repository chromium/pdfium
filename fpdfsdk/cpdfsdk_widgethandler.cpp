// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_widgethandler.h"

#include <memory>

#include "constants/access_permissions.h"
#include "constants/form_flags.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfdoc/cpdf_interactiveform.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_interactiveform.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/formfiller/cffl_formfield.h"
#include "fpdfsdk/formfiller/cffl_interactiveformfiller.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"
#include "third_party/base/containers/contains.h"
#include "third_party/base/notreached.h"

CPDFSDK_WidgetHandler::CPDFSDK_WidgetHandler() = default;

CPDFSDK_WidgetHandler::~CPDFSDK_WidgetHandler() = default;

bool CPDFSDK_WidgetHandler::CanAnswer(CPDFSDK_Annot* pAnnot) {
  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot);
  if (pWidget->IsSignatureWidget())
    return false;

  if (!pWidget->IsVisible())
    return false;

  int nFieldFlags = pWidget->GetFieldFlags();
  if (nFieldFlags & pdfium::form_flags::kReadOnly)
    return false;

  if (pWidget->GetFieldType() == FormFieldType::kPushButton)
    return true;

  CPDF_Page* pPage = pWidget->GetPDFPage();
  uint32_t dwPermissions = pPage->GetDocument()->GetUserPermissions();
  return (dwPermissions & pdfium::access_permissions::kFillForm) ||
         (dwPermissions & pdfium::access_permissions::kModifyAnnotation);
}

std::unique_ptr<CPDFSDK_Annot> CPDFSDK_WidgetHandler::NewAnnot(
    CPDF_Annot* pAnnot,
    CPDFSDK_PageView* pPageView) {
  CHECK(pPageView);
  CPDFSDK_InteractiveForm* pForm =
      GetFormFillEnvironment()->GetInteractiveForm();
  CPDF_InteractiveForm* pPDFForm = pForm->GetInteractiveForm();
  CPDF_FormControl* pCtrl = pPDFForm->GetControlByDict(pAnnot->GetAnnotDict());
  if (!pCtrl)
    return nullptr;

  auto pWidget = std::make_unique<CPDFSDK_Widget>(pAnnot, pPageView, pForm);
  pForm->AddMap(pCtrl, pWidget.get());
  if (pPDFForm->NeedConstructAP())
    pWidget->ResetAppearance(pdfium::nullopt, CPDFSDK_Widget::kValueUnchanged);
  return pWidget;
}

void CPDFSDK_WidgetHandler::ReleaseAnnot(
    std::unique_ptr<CPDFSDK_Annot> pAnnot) {
  DCHECK(pAnnot);
  GetFormFillEnvironment()->GetInteractiveFormFiller()->OnDelete(pAnnot.get());

  std::unique_ptr<CPDFSDK_Widget> pWidget(ToCPDFSDKWidget(pAnnot.release()));
  CPDFSDK_InteractiveForm* pForm = pWidget->GetInteractiveForm();
  CPDF_FormControl* pControl = pWidget->GetFormControl();
  pForm->RemoveMap(pControl);
}

void CPDFSDK_WidgetHandler::OnDraw(CPDFSDK_Annot* pAnnot,
                                   CFX_RenderDevice* pDevice,
                                   const CFX_Matrix& mtUser2Device,
                                   bool bDrawAnnots) {
  if (pAnnot->IsSignatureWidget()) {
    pAnnot->AsBAAnnot()->DrawAppearance(
        pDevice, mtUser2Device, CPDF_Annot::AppearanceMode::kNormal, nullptr);
  } else {
    GetFormFillEnvironment()->GetInteractiveFormFiller()->OnDraw(
        pAnnot->GetPageView(), pAnnot, pDevice, mtUser2Device);
  }
}

void CPDFSDK_WidgetHandler::OnMouseEnter(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                         Mask<FWL_EVENTFLAG> nFlag) {
  if (!(*pAnnot)->IsSignatureWidget()) {
    GetFormFillEnvironment()->GetInteractiveFormFiller()->OnMouseEnter(
        (*pAnnot)->GetPageView(), pAnnot, nFlag);
  }
}

void CPDFSDK_WidgetHandler::OnMouseExit(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                        Mask<FWL_EVENTFLAG> nFlag) {
  if (!(*pAnnot)->IsSignatureWidget()) {
    GetFormFillEnvironment()->GetInteractiveFormFiller()->OnMouseExit(
        (*pAnnot)->GetPageView(), pAnnot, nFlag);
  }
}

bool CPDFSDK_WidgetHandler::OnLButtonDown(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                          Mask<FWL_EVENTFLAG> nFlags,
                                          const CFX_PointF& point) {
  return !(*pAnnot)->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->OnLButtonDown(
             (*pAnnot)->GetPageView(), pAnnot, nFlags, point);
}

bool CPDFSDK_WidgetHandler::OnLButtonUp(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                        Mask<FWL_EVENTFLAG> nFlags,
                                        const CFX_PointF& point) {
  return !(*pAnnot)->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->OnLButtonUp(
             (*pAnnot)->GetPageView(), pAnnot, nFlags, point);
}

bool CPDFSDK_WidgetHandler::OnLButtonDblClk(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                            Mask<FWL_EVENTFLAG> nFlags,
                                            const CFX_PointF& point) {
  return !(*pAnnot)->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->OnLButtonDblClk(
             (*pAnnot)->GetPageView(), pAnnot, nFlags, point);
}

bool CPDFSDK_WidgetHandler::OnMouseMove(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                        Mask<FWL_EVENTFLAG> nFlags,
                                        const CFX_PointF& point) {
  return !(*pAnnot)->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->OnMouseMove(
             (*pAnnot)->GetPageView(), pAnnot, nFlags, point);
}

bool CPDFSDK_WidgetHandler::OnMouseWheel(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                         Mask<FWL_EVENTFLAG> nFlags,
                                         const CFX_PointF& point,
                                         const CFX_Vector& delta) {
  return !(*pAnnot)->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->OnMouseWheel(
             (*pAnnot)->GetPageView(), pAnnot, nFlags, point, delta);
}

bool CPDFSDK_WidgetHandler::OnRButtonDown(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                          Mask<FWL_EVENTFLAG> nFlags,
                                          const CFX_PointF& point) {
  return !(*pAnnot)->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->OnRButtonDown(
             (*pAnnot)->GetPageView(), pAnnot, nFlags, point);
}

bool CPDFSDK_WidgetHandler::OnRButtonUp(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                        Mask<FWL_EVENTFLAG> nFlags,
                                        const CFX_PointF& point) {
  return !(*pAnnot)->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->OnRButtonUp(
             (*pAnnot)->GetPageView(), pAnnot, nFlags, point);
}

bool CPDFSDK_WidgetHandler::OnRButtonDblClk(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                            Mask<FWL_EVENTFLAG> nFlags,
                                            const CFX_PointF& point) {
  return false;
}

bool CPDFSDK_WidgetHandler::OnChar(CPDFSDK_Annot* pAnnot,
                                   uint32_t nChar,
                                   Mask<FWL_EVENTFLAG> nFlags) {
  return !pAnnot->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->OnChar(
             pAnnot, nChar, nFlags);
}

bool CPDFSDK_WidgetHandler::OnKeyDown(CPDFSDK_Annot* pAnnot,
                                      FWL_VKEYCODE nKeyCode,
                                      Mask<FWL_EVENTFLAG> nFlag) {
  return !pAnnot->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->OnKeyDown(
             pAnnot, nKeyCode, nFlag);
}

bool CPDFSDK_WidgetHandler::OnKeyUp(CPDFSDK_Annot* pAnnot,
                                    FWL_VKEYCODE nKeyCode,
                                    Mask<FWL_EVENTFLAG> nFlag) {
  return false;
}

void CPDFSDK_WidgetHandler::OnLoad(CPDFSDK_Annot* pAnnot) {
  if (pAnnot->IsSignatureWidget())
    return;

  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot);
  if (!pWidget->IsAppearanceValid())
    pWidget->ResetAppearance(pdfium::nullopt, CPDFSDK_Widget::kValueUnchanged);

  FormFieldType fieldType = pWidget->GetFieldType();
  if (fieldType == FormFieldType::kTextField ||
      fieldType == FormFieldType::kComboBox) {
    ObservedPtr<CPDFSDK_Annot> pObserved(pWidget);
    Optional<WideString> sValue = pWidget->OnFormat();
    if (!pObserved)
      return;

    if (sValue.has_value() && fieldType == FormFieldType::kComboBox)
      pWidget->ResetAppearance(sValue, CPDFSDK_Widget::kValueUnchanged);
  }

#ifdef PDF_ENABLE_XFA
  auto* pContext = GetFormFillEnvironment()->GetDocExtension();
  if (pContext && pContext->ContainsExtensionForegroundForm()) {
    if (!pWidget->IsAppearanceValid() && !pWidget->GetValue().IsEmpty())
      pWidget->ResetXFAAppearance(CPDFSDK_Widget::kValueUnchanged);
  }
#endif  // PDF_ENABLE_XFA
}

bool CPDFSDK_WidgetHandler::OnSetFocus(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                       Mask<FWL_EVENTFLAG> nFlag) {
  if (!IsFocusableAnnot((*pAnnot)->GetPDFAnnot()->GetSubtype()))
    return false;

  return (*pAnnot)->IsSignatureWidget() ||
         GetFormFillEnvironment()->GetInteractiveFormFiller()->OnSetFocus(
             pAnnot, nFlag);
}

bool CPDFSDK_WidgetHandler::OnKillFocus(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                        Mask<FWL_EVENTFLAG> nFlag) {
  if (!IsFocusableAnnot((*pAnnot)->GetPDFAnnot()->GetSubtype()))
    return false;

  return (*pAnnot)->IsSignatureWidget() ||
         GetFormFillEnvironment()->GetInteractiveFormFiller()->OnKillFocus(
             pAnnot, nFlag);
}

bool CPDFSDK_WidgetHandler::SetIndexSelected(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                             int index,
                                             bool selected) {
  return !(*pAnnot)->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->SetIndexSelected(
             pAnnot, index, selected);
}

bool CPDFSDK_WidgetHandler::IsIndexSelected(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                                            int index) {
  return !(*pAnnot)->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->IsIndexSelected(
             pAnnot, index);
}

CFX_FloatRect CPDFSDK_WidgetHandler::GetViewBBox(CPDFSDK_Annot* pAnnot) {
  if (!pAnnot->IsSignatureWidget()) {
    return CFX_FloatRect(
        GetFormFillEnvironment()->GetInteractiveFormFiller()->GetViewBBox(
            pAnnot->GetPageView(), pAnnot));
  }
  return CFX_FloatRect();
}

WideString CPDFSDK_WidgetHandler::GetText(CPDFSDK_Annot* pAnnot) {
  if (!pAnnot->IsSignatureWidget()) {
    return GetFormFillEnvironment()->GetInteractiveFormFiller()->GetText(
        pAnnot);
  }
  return WideString();
}

WideString CPDFSDK_WidgetHandler::GetSelectedText(CPDFSDK_Annot* pAnnot) {
  if (!pAnnot->IsSignatureWidget()) {
    return GetFormFillEnvironment()
        ->GetInteractiveFormFiller()
        ->GetSelectedText(pAnnot);
  }
  return WideString();
}

void CPDFSDK_WidgetHandler::ReplaceSelection(CPDFSDK_Annot* pAnnot,
                                             const WideString& text) {
  if (!pAnnot->IsSignatureWidget()) {
    GetFormFillEnvironment()->GetInteractiveFormFiller()->ReplaceSelection(
        pAnnot, text);
  }
}

bool CPDFSDK_WidgetHandler::SelectAllText(CPDFSDK_Annot* pAnnot) {
  return !pAnnot->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->SelectAllText(
             pAnnot);
}

bool CPDFSDK_WidgetHandler::CanUndo(CPDFSDK_Annot* pAnnot) {
  return !pAnnot->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->CanUndo(pAnnot);
}

bool CPDFSDK_WidgetHandler::CanRedo(CPDFSDK_Annot* pAnnot) {
  return !pAnnot->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->CanRedo(pAnnot);
}

bool CPDFSDK_WidgetHandler::Undo(CPDFSDK_Annot* pAnnot) {
  return !pAnnot->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->Undo(pAnnot);
}

bool CPDFSDK_WidgetHandler::Redo(CPDFSDK_Annot* pAnnot) {
  return !pAnnot->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->Redo(pAnnot);
}

bool CPDFSDK_WidgetHandler::HitTest(CPDFSDK_Annot* pAnnot,
                                    const CFX_PointF& point) {
  DCHECK(pAnnot);
  return GetViewBBox(pAnnot).Contains(point);
}

bool CPDFSDK_WidgetHandler::IsFocusableAnnot(
    const CPDF_Annot::Subtype& annot_type) const {
  DCHECK_EQ(annot_type, CPDF_Annot::Subtype::WIDGET);
  return pdfium::Contains(GetFormFillEnvironment()->GetFocusableAnnotSubtypes(),
                          annot_type);
}

#ifdef PDF_ENABLE_XFA
std::unique_ptr<CPDFSDK_Annot> CPDFSDK_WidgetHandler::NewAnnotForXFA(
    CXFA_FFWidget* pWidget,
    CPDFSDK_PageView* pPageView) {
  NOTREACHED();
  return nullptr;
}

bool CPDFSDK_WidgetHandler::OnXFAChangedFocus(
    ObservedPtr<CPDFSDK_Annot>* pNewAnnot) {
  NOTREACHED();
  return false;
}
#endif
