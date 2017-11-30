// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffcheckbutton.h"

#include <utility>
#include "third_party/base/ptr_util.h"
#include "xfa/fwl/cfwl_checkbox.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_widgetmgr.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffexclgroup.h"
#include "xfa/fxfa/cxfa_fffield.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"

CXFA_FFCheckButton::CXFA_FFCheckButton(CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFField(pDataAcc), m_pOldDelegate(nullptr) {}

CXFA_FFCheckButton::~CXFA_FFCheckButton() {}

bool CXFA_FFCheckButton::LoadWidget() {
  auto pNew = pdfium::MakeUnique<CFWL_CheckBox>(GetFWLApp());
  CFWL_CheckBox* pCheckBox = pNew.get();
  m_pNormalWidget = std::move(pNew);
  m_pNormalWidget->SetLayoutItem(this);

  CFWL_NoteDriver* pNoteDriver =
      m_pNormalWidget->GetOwnerApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(m_pNormalWidget.get(),
                                   m_pNormalWidget.get());
  m_pOldDelegate = m_pNormalWidget->GetDelegate();
  m_pNormalWidget->SetDelegate(this);
  if (m_pDataAcc->IsRadioButton())
    pCheckBox->ModifyStylesEx(FWL_STYLEEXT_CKB_RadioButton, 0xFFFFFFFF);

  m_pNormalWidget->LockUpdate();
  UpdateWidgetProperty();
  SetFWLCheckState(m_pDataAcc->GetCheckState());
  m_pNormalWidget->UnlockUpdate();
  return CXFA_FFField::LoadWidget();
}

void CXFA_FFCheckButton::UpdateWidgetProperty() {
  auto* pCheckBox = static_cast<CFWL_CheckBox*>(m_pNormalWidget.get());
  if (!pCheckBox)
    return;

  pCheckBox->SetBoxSize(m_pDataAcc->GetCheckButtonSize());
  uint32_t dwStyleEx = FWL_STYLEEXT_CKB_SignShapeCross;
  switch (m_pDataAcc->GetCheckButtonMark()) {
    case XFA_AttributeEnum::Check:
      dwStyleEx = FWL_STYLEEXT_CKB_SignShapeCheck;
      break;
    case XFA_AttributeEnum::Circle:
      dwStyleEx = FWL_STYLEEXT_CKB_SignShapeCircle;
      break;
    case XFA_AttributeEnum::Cross:
      break;
    case XFA_AttributeEnum::Diamond:
      dwStyleEx = FWL_STYLEEXT_CKB_SignShapeDiamond;
      break;
    case XFA_AttributeEnum::Square:
      dwStyleEx = FWL_STYLEEXT_CKB_SignShapeSquare;
      break;
    case XFA_AttributeEnum::Star:
      dwStyleEx = FWL_STYLEEXT_CKB_SignShapeStar;
      break;
    default: {
      if (m_pDataAcc->IsCheckButtonRound())
        dwStyleEx = FWL_STYLEEXT_CKB_SignShapeCircle;
    } break;
  }
  if (m_pDataAcc->IsAllowNeutral())
    dwStyleEx |= FWL_STYLEEXT_CKB_3State;

  pCheckBox->ModifyStylesEx(
      dwStyleEx, FWL_STYLEEXT_CKB_SignShapeMask | FWL_STYLEEXT_CKB_3State);
}

bool CXFA_FFCheckButton::PerformLayout() {
  CXFA_FFWidget::PerformLayout();

  float fCheckSize = m_pDataAcc->GetCheckButtonSize();
  CXFA_MarginData marginData = m_pDataAcc->GetMarginData();
  CFX_RectF rtWidget = GetRectWithoutRotate();
  if (marginData.HasValidNode())
    XFA_RectWidthoutMargin(rtWidget, marginData);

  XFA_AttributeEnum iCapPlacement = XFA_AttributeEnum::Unknown;
  float fCapReserve = 0;
  CXFA_CaptionData captionData = m_pDataAcc->GetCaptionData();
  if (captionData.HasValidNode() && captionData.IsVisible()) {
    m_rtCaption = rtWidget;
    iCapPlacement = captionData.GetPlacementType();
    fCapReserve = captionData.GetReserve();
    if (fCapReserve <= 0) {
      if (iCapPlacement == XFA_AttributeEnum::Top ||
          iCapPlacement == XFA_AttributeEnum::Bottom) {
        fCapReserve = rtWidget.height - fCheckSize;
      } else {
        fCapReserve = rtWidget.width - fCheckSize;
      }
    }
  }

  XFA_AttributeEnum iHorzAlign = XFA_AttributeEnum::Left;
  XFA_AttributeEnum iVertAlign = XFA_AttributeEnum::Top;
  CXFA_ParaData paraData = m_pDataAcc->GetParaData();
  if (paraData.HasValidNode()) {
    iHorzAlign = paraData.GetHorizontalAlign();
    iVertAlign = paraData.GetVerticalAlign();
  }

  m_rtUI = rtWidget;
  CXFA_MarginData captionMarginData = captionData.GetMarginData();
  switch (iCapPlacement) {
    case XFA_AttributeEnum::Left: {
      m_rtCaption.width = fCapReserve;
      CapLeftRightPlacement(captionMarginData);
      m_rtUI.width -= fCapReserve;
      m_rtUI.left += fCapReserve;
      break;
    }
    case XFA_AttributeEnum::Top: {
      m_rtCaption.height = fCapReserve;
      XFA_RectWidthoutMargin(m_rtCaption, captionMarginData);
      m_rtUI.height -= fCapReserve;
      m_rtUI.top += fCapReserve;
      break;
    }
    case XFA_AttributeEnum::Right: {
      m_rtCaption.left = m_rtCaption.right() - fCapReserve;
      m_rtCaption.width = fCapReserve;
      CapLeftRightPlacement(captionMarginData);
      m_rtUI.width -= fCapReserve;
      break;
    }
    case XFA_AttributeEnum::Bottom: {
      m_rtCaption.top = m_rtCaption.bottom() - fCapReserve;
      m_rtCaption.height = fCapReserve;
      XFA_RectWidthoutMargin(m_rtCaption, captionMarginData);
      m_rtUI.height -= fCapReserve;
      break;
    }
    case XFA_AttributeEnum::Inline:
      break;
    default:
      iHorzAlign = XFA_AttributeEnum::Right;
      break;
  }

  if (iHorzAlign == XFA_AttributeEnum::Center)
    m_rtUI.left += (m_rtUI.width - fCheckSize) / 2;
  else if (iHorzAlign == XFA_AttributeEnum::Right)
    m_rtUI.left = m_rtUI.right() - fCheckSize;

  if (iVertAlign == XFA_AttributeEnum::Middle)
    m_rtUI.top += (m_rtUI.height - fCheckSize) / 2;
  else if (iVertAlign == XFA_AttributeEnum::Bottom)
    m_rtUI.top = m_rtUI.bottom() - fCheckSize;

  m_rtUI.width = fCheckSize;
  m_rtUI.height = fCheckSize;
  AddUIMargin(iCapPlacement);
  m_rtCheckBox = m_rtUI;
  CXFA_BorderData borderUIData = m_pDataAcc->GetUIBorderData();
  if (borderUIData.HasValidNode()) {
    CXFA_MarginData borderMarginData = borderUIData.GetMarginData();
    if (borderMarginData.HasValidNode())
      XFA_RectWidthoutMargin(m_rtUI, borderMarginData);
  }

  m_rtUI.Normalize();
  LayoutCaption();
  SetFWLRect();
  if (m_pNormalWidget)
    m_pNormalWidget->Update();

  return true;
}

void CXFA_FFCheckButton::CapLeftRightPlacement(
    const CXFA_MarginData& captionMarginData) {
  XFA_RectWidthoutMargin(m_rtCaption, captionMarginData);
  if (m_rtCaption.height < 0)
    m_rtCaption.top += m_rtCaption.height;
  if (m_rtCaption.width < 0) {
    m_rtCaption.left += m_rtCaption.width;
    m_rtCaption.width = -m_rtCaption.width;
  }
}

void CXFA_FFCheckButton::AddUIMargin(XFA_AttributeEnum iCapPlacement) {
  CFX_RectF rtUIMargin = m_pDataAcc->GetUIMargin();
  m_rtUI.top -= rtUIMargin.top / 2 - rtUIMargin.height / 2;

  float fLeftAddRight = rtUIMargin.left + rtUIMargin.width;
  float fTopAddBottom = rtUIMargin.top + rtUIMargin.height;
  if (m_rtUI.width < fLeftAddRight) {
    if (iCapPlacement == XFA_AttributeEnum::Right ||
        iCapPlacement == XFA_AttributeEnum::Left) {
      m_rtUI.left -= fLeftAddRight - m_rtUI.width;
    } else {
      m_rtUI.left -= 2 * (fLeftAddRight - m_rtUI.width);
    }
    m_rtUI.width += 2 * (fLeftAddRight - m_rtUI.width);
  }
  if (m_rtUI.height < fTopAddBottom) {
    if (iCapPlacement == XFA_AttributeEnum::Right)
      m_rtUI.left -= fTopAddBottom - m_rtUI.height;

    m_rtUI.top -= fTopAddBottom - m_rtUI.height;
    m_rtUI.height += 2 * (fTopAddBottom - m_rtUI.height);
  }
}

void CXFA_FFCheckButton::RenderWidget(CXFA_Graphics* pGS,
                                      const CFX_Matrix& matrix,
                                      uint32_t dwStatus) {
  if (!IsMatchVisibleStatus(dwStatus))
    return;

  CFX_Matrix mtRotate = GetRotateMatrix();
  mtRotate.Concat(matrix);

  CXFA_FFWidget::RenderWidget(pGS, mtRotate, dwStatus);
  DrawBorderWithFlags(
      pGS, m_pDataAcc->GetUIBorderData(), m_rtUI, mtRotate,
      m_pDataAcc->IsCheckButtonRound() ? XFA_DRAWBOX_ForceRound : 0);
  RenderCaption(pGS, &mtRotate);
  DrawHighlight(pGS, &mtRotate, dwStatus, m_pDataAcc->IsCheckButtonRound());
  CFX_Matrix mt(1, 0, 0, 1, m_rtCheckBox.left, m_rtCheckBox.top);
  mt.Concat(mtRotate);
  GetApp()->GetFWLWidgetMgr()->OnDrawWidget(m_pNormalWidget.get(), pGS, mt);
}

bool CXFA_FFCheckButton::OnLButtonUp(uint32_t dwFlags,
                                     const CFX_PointF& point) {
  if (!m_pNormalWidget || !IsButtonDown())
    return false;

  SetButtonDown(false);
  CFWL_MessageMouse ms(nullptr, m_pNormalWidget.get());
  ms.m_dwCmd = FWL_MouseCommand::LeftButtonUp;
  ms.m_dwFlags = dwFlags;
  ms.m_pos = FWLToClient(point);
  TranslateFWLMessage(&ms);
  return true;
}

XFA_CHECKSTATE CXFA_FFCheckButton::FWLState2XFAState() {
  uint32_t dwState = m_pNormalWidget->GetStates();
  if (dwState & FWL_STATE_CKB_Checked)
    return XFA_CHECKSTATE_On;
  if (dwState & FWL_STATE_CKB_Neutral)
    return XFA_CHECKSTATE_Neutral;
  return XFA_CHECKSTATE_Off;
}

bool CXFA_FFCheckButton::CommitData() {
  XFA_CHECKSTATE eCheckState = FWLState2XFAState();
  m_pDataAcc->SetCheckState(eCheckState, true);
  return true;
}

bool CXFA_FFCheckButton::IsDataChanged() {
  XFA_CHECKSTATE eCheckState = FWLState2XFAState();
  return m_pDataAcc->GetCheckState() != eCheckState;
}

void CXFA_FFCheckButton::SetFWLCheckState(XFA_CHECKSTATE eCheckState) {
  if (eCheckState == XFA_CHECKSTATE_Neutral)
    m_pNormalWidget->SetStates(FWL_STATE_CKB_Neutral);
  else if (eCheckState == XFA_CHECKSTATE_On)
    m_pNormalWidget->SetStates(FWL_STATE_CKB_Checked);
  else
    m_pNormalWidget->RemoveStates(FWL_STATE_CKB_Checked);
}

bool CXFA_FFCheckButton::UpdateFWLData() {
  if (!m_pNormalWidget)
    return false;

  XFA_CHECKSTATE eState = m_pDataAcc->GetCheckState();
  SetFWLCheckState(eState);
  m_pNormalWidget->Update();
  return true;
}

void CXFA_FFCheckButton::OnProcessMessage(CFWL_Message* pMessage) {
  m_pOldDelegate->OnProcessMessage(pMessage);
}

void CXFA_FFCheckButton::OnProcessEvent(CFWL_Event* pEvent) {
  CXFA_FFField::OnProcessEvent(pEvent);
  switch (pEvent->GetType()) {
    case CFWL_Event::Type::CheckStateChanged: {
      CXFA_EventParam eParam;
      eParam.m_eType = XFA_EVENT_Change;
      eParam.m_wsNewText = m_pDataAcc->GetValue(XFA_VALUEPICTURE_Raw);

      CXFA_WidgetAcc* pFFExclGroup = m_pDataAcc->GetExclGroup();
      if (ProcessCommittedData()) {
        eParam.m_pTarget = pFFExclGroup;
        if (pFFExclGroup) {
          m_pDocView->AddValidateWidget(pFFExclGroup);
          m_pDocView->AddCalculateWidgetAcc(pFFExclGroup);
          pFFExclGroup->ProcessEvent(XFA_AttributeEnum::Change, &eParam);
        }
        eParam.m_pTarget = m_pDataAcc.Get();
        m_pDataAcc->ProcessEvent(XFA_AttributeEnum::Change, &eParam);
      } else {
        SetFWLCheckState(m_pDataAcc->GetCheckState());
      }
      if (pFFExclGroup) {
        eParam.m_pTarget = pFFExclGroup;
        pFFExclGroup->ProcessEvent(XFA_AttributeEnum::Click, &eParam);
      }
      eParam.m_pTarget = m_pDataAcc.Get();
      m_pDataAcc->ProcessEvent(XFA_AttributeEnum::Click, &eParam);
      break;
    }
    default:
      break;
  }
  m_pOldDelegate->OnProcessEvent(pEvent);
}

void CXFA_FFCheckButton::OnDrawWidget(CXFA_Graphics* pGraphics,
                                      const CFX_Matrix& matrix) {
  m_pOldDelegate->OnDrawWidget(pGraphics, matrix);
}
