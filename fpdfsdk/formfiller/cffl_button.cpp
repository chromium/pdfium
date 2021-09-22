// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/formfiller/cffl_button.h"

#include "core/fpdfdoc/cpdf_formcontrol.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "third_party/base/check.h"

CFFL_Button::CFFL_Button(CFFL_InteractiveFormFiller* pFormFiller,
                         CPDFSDK_Widget* pWidget)
    : CFFL_FormField(pFormFiller, pWidget) {}

CFFL_Button::~CFFL_Button() = default;

void CFFL_Button::OnMouseEnter(CPDFSDK_PageView* pPageView) {
  m_bMouseIn = true;
  InvalidateRect(GetViewBBox(pPageView));
}

void CFFL_Button::OnMouseExit(CPDFSDK_PageView* pPageView) {
  m_bMouseIn = false;
  InvalidateRect(GetViewBBox(pPageView));
  m_pTimer.reset();
  DCHECK(m_pWidget);
}

bool CFFL_Button::OnLButtonDown(CPDFSDK_PageView* pPageView,
                                CPDFSDK_Widget* pWidget,
                                Mask<FWL_EVENTFLAG> nFlags,
                                const CFX_PointF& point) {
  if (!pWidget->GetRect().Contains(point))
    return false;

  m_bMouseDown = true;
  m_bValid = true;
  InvalidateRect(GetViewBBox(pPageView));
  return true;
}

bool CFFL_Button::OnLButtonUp(CPDFSDK_PageView* pPageView,
                              CPDFSDK_Widget* pWidget,
                              Mask<FWL_EVENTFLAG> nFlags,
                              const CFX_PointF& point) {
  if (!pWidget->GetRect().Contains(point))
    return false;

  m_bMouseDown = false;
  InvalidateRect(GetViewBBox(pPageView));
  return true;
}

bool CFFL_Button::OnMouseMove(CPDFSDK_PageView* pPageView,
                              Mask<FWL_EVENTFLAG> nFlags,
                              const CFX_PointF& point) {
  return true;
}

void CFFL_Button::OnDraw(CPDFSDK_PageView* pPageView,
                         CPDFSDK_Widget* pWidget,
                         CFX_RenderDevice* pDevice,
                         const CFX_Matrix& mtUser2Device) {
  DCHECK(pPageView);
  CPDF_FormControl* pCtrl = pWidget->GetFormControl();
  if (pCtrl->GetHighlightingMode() != CPDF_FormControl::kPush) {
    pWidget->DrawAppearance(pDevice, mtUser2Device,
                            CPDF_Annot::AppearanceMode::kNormal, nullptr);
    return;
  }
  if (m_bMouseDown) {
    if (pWidget->IsWidgetAppearanceValid(CPDF_Annot::AppearanceMode::kDown)) {
      pWidget->DrawAppearance(pDevice, mtUser2Device,
                              CPDF_Annot::AppearanceMode::kDown, nullptr);
    } else {
      pWidget->DrawAppearance(pDevice, mtUser2Device,
                              CPDF_Annot::AppearanceMode::kNormal, nullptr);
    }
    return;
  }
  if (m_bMouseIn) {
    if (pWidget->IsWidgetAppearanceValid(
            CPDF_Annot::AppearanceMode::kRollover)) {
      pWidget->DrawAppearance(pDevice, mtUser2Device,
                              CPDF_Annot::AppearanceMode::kRollover, nullptr);
    } else {
      pWidget->DrawAppearance(pDevice, mtUser2Device,
                              CPDF_Annot::AppearanceMode::kNormal, nullptr);
    }
    return;
  }

  pWidget->DrawAppearance(pDevice, mtUser2Device,
                          CPDF_Annot::AppearanceMode::kNormal, nullptr);
}

void CFFL_Button::OnDrawDeactive(CPDFSDK_PageView* pPageView,
                                 CPDFSDK_Widget* pWidget,
                                 CFX_RenderDevice* pDevice,
                                 const CFX_Matrix& mtUser2Device) {
  OnDraw(pPageView, pWidget, pDevice, mtUser2Device);
}
