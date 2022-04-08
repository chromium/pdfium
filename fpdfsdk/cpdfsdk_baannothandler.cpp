// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_baannothandler.h"

#include <memory>
#include <vector>

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfdoc/cpdf_action.h"
#include "core/fpdfdoc/cpdf_interactiveform.h"
#include "core/fxge/cfx_drawutils.h"
#include "fpdfsdk/cpdfsdk_actionhandler.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/cpdfsdk_baannot.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/formfiller/cffl_formfield.h"
#include "public/fpdf_fwlevent.h"
#include "third_party/base/check.h"
#include "third_party/base/containers/contains.h"

CPDFSDK_BAAnnotHandler::CPDFSDK_BAAnnotHandler() = default;

CPDFSDK_BAAnnotHandler::~CPDFSDK_BAAnnotHandler() = default;

std::unique_ptr<CPDFSDK_Annot> CPDFSDK_BAAnnotHandler::NewAnnot(
    CPDF_Annot* pAnnot,
    CPDFSDK_PageView* pPageView) {
  CHECK(pPageView);
  return std::make_unique<CPDFSDK_BAAnnot>(pAnnot, pPageView);
}

void CPDFSDK_BAAnnotHandler::OnDraw(CPDFSDK_Annot* pAnnot,
                                    CFX_RenderDevice* pDevice,
                                    const CFX_Matrix& mtUser2Device,
                                    bool bDrawAnnots) {
  if (pAnnot->AsXFAWidget())
    return;

  if (!pAnnot->AsBAAnnot()->IsVisible())
    return;

  const CPDF_Annot::Subtype annot_type = pAnnot->GetAnnotSubtype();
  if (bDrawAnnots && annot_type == CPDF_Annot::Subtype::POPUP) {
    pAnnot->AsBAAnnot()->DrawAppearance(pDevice, mtUser2Device,
                                        CPDF_Annot::AppearanceMode::kNormal);
    return;
  }

  if (is_annotation_focused_ && IsFocusableAnnot(annot_type) &&
      pAnnot == GetFormFillEnvironment()->GetFocusAnnot()) {
    CFX_FloatRect view_bounding_box = pAnnot->AsBAAnnot()->GetViewBBox();
    if (view_bounding_box.IsEmpty())
      return;

    view_bounding_box.Normalize();

    CFX_DrawUtils::DrawFocusRect(pDevice, mtUser2Device, view_bounding_box);
  }
}

bool CPDFSDK_BAAnnotHandler::OnChar(CPDFSDK_Annot* pAnnot,
                                    uint32_t nChar,
                                    Mask<FWL_EVENTFLAG> nFlags) {
  return false;
}

bool CPDFSDK_BAAnnotHandler::OnKeyDown(CPDFSDK_Annot* pAnnot,
                                       FWL_VKEYCODE nKeyCode,
                                       Mask<FWL_EVENTFLAG> nFlag) {
  DCHECK(pAnnot);

  // OnKeyDown() is implemented only for link annotations for now. As
  // OnKeyDown() is implemented for other subtypes, following check should be
  // modified.
  if (nKeyCode != FWL_VKEY_Return ||
      pAnnot->GetAnnotSubtype() != CPDF_Annot::Subtype::LINK) {
    return false;
  }

  CPDFSDK_BAAnnot* ba_annot = pAnnot->AsBAAnnot();
  CPDF_Action action = ba_annot->GetAAction(CPDF_AAction::kKeyStroke);

  if (action.GetDict()) {
    return GetFormFillEnvironment()->GetActionHandler()->DoAction_Link(
        action, CPDF_AAction::kKeyStroke, GetFormFillEnvironment(), nFlag);
  }

  return GetFormFillEnvironment()->GetActionHandler()->DoAction_Destination(
      ba_annot->GetDestination(), GetFormFillEnvironment());
}

bool CPDFSDK_BAAnnotHandler::IsFocusableAnnot(
    const CPDF_Annot::Subtype& annot_type) const {
  DCHECK(annot_type != CPDF_Annot::Subtype::WIDGET);

  return pdfium::Contains(GetFormFillEnvironment()->GetFocusableAnnotSubtypes(),
                          annot_type);
}

void CPDFSDK_BAAnnotHandler::InvalidateRect(CPDFSDK_Annot* annot) {
  CPDFSDK_BAAnnot* ba_annot = annot->AsBAAnnot();
  CFX_FloatRect view_bounding_box = ba_annot->GetViewBBox();
  if (!view_bounding_box.IsEmpty()) {
    view_bounding_box.Inflate(1, 1);
    view_bounding_box.Normalize();
    FX_RECT rect = view_bounding_box.GetOuterRect();
    GetFormFillEnvironment()->Invalidate(ba_annot->GetPage(), rect);
  }
}

bool CPDFSDK_BAAnnotHandler::OnSetFocus(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                                        Mask<FWL_EVENTFLAG> nFlag) {
  if (!IsFocusableAnnot(pAnnot->GetAnnotSubtype()))
    return false;

  is_annotation_focused_ = true;
  InvalidateRect(pAnnot.Get());
  return true;
}

bool CPDFSDK_BAAnnotHandler::OnKillFocus(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                                         Mask<FWL_EVENTFLAG> nFlag) {
  if (!IsFocusableAnnot(pAnnot->GetAnnotSubtype()))
    return false;

  is_annotation_focused_ = false;
  InvalidateRect(pAnnot.Get());
  return true;
}

bool CPDFSDK_BAAnnotHandler::SetIndexSelected(
    ObservedPtr<CPDFSDK_Annot>& pAnnot,
    int index,
    bool selected) {
  return false;
}

bool CPDFSDK_BAAnnotHandler::IsIndexSelected(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                                             int index) {
  return false;
}

WideString CPDFSDK_BAAnnotHandler::GetText(CPDFSDK_Annot* pAnnot) {
  return WideString();
}

WideString CPDFSDK_BAAnnotHandler::GetSelectedText(CPDFSDK_Annot* pAnnot) {
  return WideString();
}

void CPDFSDK_BAAnnotHandler::ReplaceSelection(CPDFSDK_Annot* pAnnot,
                                              const WideString& text) {}

bool CPDFSDK_BAAnnotHandler::SelectAllText(CPDFSDK_Annot* pAnnot) {
  return false;
}
