// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffsignature.h"

#include "third_party/base/check.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_fffield.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/parser/cxfa_border.h"

CXFA_FFSignature::CXFA_FFSignature(CXFA_Node* pNode) : CXFA_FFField(pNode) {}

CXFA_FFSignature::~CXFA_FFSignature() = default;

bool CXFA_FFSignature::LoadWidget() {
  DCHECK(!IsLoaded());
  return CXFA_FFField::LoadWidget();
}

void CXFA_FFSignature::RenderWidget(CFGAS_GEGraphics* pGS,
                                    const CFX_Matrix& matrix,
                                    HighlightOption highlight) {
  if (!HasVisibleStatus())
    return;

  CFX_Matrix mtRotate = GetRotateMatrix();
  mtRotate.Concat(matrix);

  CXFA_FFWidget::RenderWidget(pGS, mtRotate, highlight);

  DrawBorder(pGS, m_pNode->GetUIBorder(), m_UIRect, mtRotate);
  RenderCaption(pGS, mtRotate);
  DrawHighlight(pGS, mtRotate, highlight, kSquareShape);
}

bool CXFA_FFSignature::OnMouseEnter() {
  return false;
}

bool CXFA_FFSignature::OnMouseExit() {
  return false;
}

bool CXFA_FFSignature::AcceptsFocusOnButtonDown(
    uint32_t dwFlags,
    const CFX_PointF& point,
    CFWL_MessageMouse::MouseCommand command) {
  return false;
}

bool CXFA_FFSignature::OnLButtonDown(uint32_t dwFlags,
                                     const CFX_PointF& point) {
  return false;
}

bool CXFA_FFSignature::OnLButtonUp(uint32_t dwFlags, const CFX_PointF& point) {
  return false;
}

bool CXFA_FFSignature::OnLButtonDblClk(uint32_t dwFlags,
                                       const CFX_PointF& point) {
  return false;
}

bool CXFA_FFSignature::OnMouseMove(uint32_t dwFlags, const CFX_PointF& point) {
  return false;
}

bool CXFA_FFSignature::OnMouseWheel(uint32_t dwFlags,
                                    const CFX_PointF& point,
                                    const CFX_Vector& delta) {
  return false;
}

bool CXFA_FFSignature::OnRButtonDown(uint32_t dwFlags,
                                     const CFX_PointF& point) {
  return false;
}

bool CXFA_FFSignature::OnRButtonUp(uint32_t dwFlags, const CFX_PointF& point) {
  return false;
}

bool CXFA_FFSignature::OnRButtonDblClk(uint32_t dwFlags,
                                       const CFX_PointF& point) {
  return false;
}

bool CXFA_FFSignature::OnKeyDown(XFA_FWL_VKEYCODE dwKeyCode, uint32_t dwFlags) {
  return false;
}

bool CXFA_FFSignature::OnKeyUp(XFA_FWL_VKEYCODE dwKeyCode, uint32_t dwFlags) {
  return false;
}

bool CXFA_FFSignature::OnChar(uint32_t nChar, uint32_t dwFlags) {
  return false;
}

FWL_WidgetHit CXFA_FFSignature::HitTest(const CFX_PointF& point) {
  auto* pNorm = GetNormalWidget();
  if (pNorm && pNorm->HitTest(FWLToClient(point)) != FWL_WidgetHit::Unknown)
    return FWL_WidgetHit::Client;
  if (!GetRectWithoutRotate().Contains(point))
    return FWL_WidgetHit::Unknown;
  if (m_CaptionRect.Contains(point))
    return FWL_WidgetHit::Titlebar;
  return FWL_WidgetHit::Client;
}

FormFieldType CXFA_FFSignature::GetFormFieldType() {
  return FormFieldType::kXFA_Signature;
}
