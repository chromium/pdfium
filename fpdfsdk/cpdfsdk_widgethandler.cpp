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

CPDFSDK_WidgetHandler::CPDFSDK_WidgetHandler() = default;

CPDFSDK_WidgetHandler::~CPDFSDK_WidgetHandler() = default;

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
    pWidget->ResetAppearance(absl::nullopt, CPDFSDK_Widget::kValueUnchanged);
  return pWidget;
}

void CPDFSDK_WidgetHandler::OnDraw(CPDFSDK_Annot* pAnnot,
                                   CFX_RenderDevice* pDevice,
                                   const CFX_Matrix& mtUser2Device,
                                   bool bDrawAnnots) {
  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot);
  if (pWidget->IsSignatureWidget()) {
    pWidget->DrawAppearance(pDevice, mtUser2Device,
                            CPDF_Annot::AppearanceMode::kNormal);
  } else {
    GetFormFillEnvironment()->GetInteractiveFormFiller()->OnDraw(
        pWidget->GetPageView(), pWidget, pDevice, mtUser2Device);
  }
}

bool CPDFSDK_WidgetHandler::OnChar(CPDFSDK_Annot* pAnnot,
                                   uint32_t nChar,
                                   Mask<FWL_EVENTFLAG> nFlags) {
  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot);
  return !pWidget->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->OnChar(
             pWidget, nChar, nFlags);
}

bool CPDFSDK_WidgetHandler::OnKeyDown(CPDFSDK_Annot* pAnnot,
                                      FWL_VKEYCODE nKeyCode,
                                      Mask<FWL_EVENTFLAG> nFlag) {
  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot);
  return !pWidget->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->OnKeyDown(
             pWidget, nKeyCode, nFlag);
}

bool CPDFSDK_WidgetHandler::OnSetFocus(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                                       Mask<FWL_EVENTFLAG> nFlag) {
  if (!IsFocusableAnnot(pAnnot->GetPDFAnnot()->GetSubtype()))
    return false;

  ObservedPtr<CPDFSDK_Widget> pWidget(ToCPDFSDKWidget(pAnnot.Get()));
  return pWidget->IsSignatureWidget() ||
         GetFormFillEnvironment()->GetInteractiveFormFiller()->OnSetFocus(
             pWidget, nFlag);
}

bool CPDFSDK_WidgetHandler::OnKillFocus(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                                        Mask<FWL_EVENTFLAG> nFlag) {
  if (!IsFocusableAnnot(pAnnot->GetPDFAnnot()->GetSubtype()))
    return false;

  ObservedPtr<CPDFSDK_Widget> pWidget(ToCPDFSDKWidget(pAnnot.Get()));
  return pWidget->IsSignatureWidget() ||
         GetFormFillEnvironment()->GetInteractiveFormFiller()->OnKillFocus(
             pWidget, nFlag);
}

bool CPDFSDK_WidgetHandler::SetIndexSelected(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                                             int index,
                                             bool selected) {
  ObservedPtr<CPDFSDK_Widget> pWidget(ToCPDFSDKWidget(pAnnot.Get()));
  return !pWidget->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->SetIndexSelected(
             pWidget, index, selected);
}

bool CPDFSDK_WidgetHandler::IsIndexSelected(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                                            int index) {
  ObservedPtr<CPDFSDK_Widget> pWidget(ToCPDFSDKWidget(pAnnot.Get()));
  return !pWidget->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->IsIndexSelected(
             pWidget, index);
}

WideString CPDFSDK_WidgetHandler::GetText(CPDFSDK_Annot* pAnnot) {
  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot);
  if (pWidget->IsSignatureWidget())
    return WideString();

  return GetFormFillEnvironment()->GetInteractiveFormFiller()->GetText(pWidget);
}

WideString CPDFSDK_WidgetHandler::GetSelectedText(CPDFSDK_Annot* pAnnot) {
  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot);
  if (pWidget->IsSignatureWidget())
    return WideString();

  return GetFormFillEnvironment()->GetInteractiveFormFiller()->GetSelectedText(
      pWidget);
}

void CPDFSDK_WidgetHandler::ReplaceSelection(CPDFSDK_Annot* pAnnot,
                                             const WideString& text) {
  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot);
  if (pWidget->IsSignatureWidget())
    return;

  GetFormFillEnvironment()->GetInteractiveFormFiller()->ReplaceSelection(
      pWidget, text);
}

bool CPDFSDK_WidgetHandler::SelectAllText(CPDFSDK_Annot* pAnnot) {
  CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot);
  return !pWidget->IsSignatureWidget() &&
         GetFormFillEnvironment()->GetInteractiveFormFiller()->SelectAllText(
             pWidget);
}

bool CPDFSDK_WidgetHandler::IsFocusableAnnot(
    const CPDF_Annot::Subtype& annot_type) const {
  DCHECK_EQ(annot_type, CPDF_Annot::Subtype::WIDGET);
  return pdfium::Contains(GetFormFillEnvironment()->GetFocusableAnnotSubtypes(),
                          annot_type);
}
