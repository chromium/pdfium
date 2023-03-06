// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffcheckbutton.h"

#include "third_party/base/check.h"
#include "v8/include/cppgc/visitor.h"
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
#include "xfa/fxfa/parser/cxfa_border.h"
#include "xfa/fxfa/parser/cxfa_caption.h"
#include "xfa/fxfa/parser/cxfa_checkbutton.h"
#include "xfa/fxfa/parser/cxfa_para.h"

CXFA_FFCheckButton::CXFA_FFCheckButton(CXFA_Node* pNode,
                                       CXFA_CheckButton* button)
    : CXFA_FFField(pNode), button_(button) {}

CXFA_FFCheckButton::~CXFA_FFCheckButton() = default;

void CXFA_FFCheckButton::Trace(cppgc::Visitor* visitor) const {
  CXFA_FFField::Trace(visitor);
  visitor->Trace(m_pOldDelegate);
  visitor->Trace(button_);
}

bool CXFA_FFCheckButton::LoadWidget() {
  DCHECK(!IsLoaded());

  CFWL_CheckBox* pCheckBox = cppgc::MakeGarbageCollected<CFWL_CheckBox>(
      GetFWLApp()->GetHeap()->GetAllocationHandle(), GetFWLApp());
  SetNormalWidget(pCheckBox);
  pCheckBox->SetAdapterIface(this);

  CFWL_NoteDriver* pNoteDriver = pCheckBox->GetFWLApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(pCheckBox, pCheckBox);
  m_pOldDelegate = pCheckBox->GetDelegate();
  pCheckBox->SetDelegate(this);
  if (m_pNode->IsRadioButton())
    pCheckBox->ModifyStyleExts(FWL_STYLEEXT_CKB_RadioButton, 0xFFFFFFFF);

  {
    CFWL_Widget::ScopedUpdateLock update_lock(pCheckBox);
    UpdateWidgetProperty();
    SetFWLCheckState(m_pNode->GetCheckState());
  }

  return CXFA_FFField::LoadWidget();
}

void CXFA_FFCheckButton::UpdateWidgetProperty() {
  auto* pCheckBox = static_cast<CFWL_CheckBox*>(GetNormalWidget());
  if (!pCheckBox)
    return;

  pCheckBox->SetBoxSize(m_pNode->GetCheckButtonSize());
  uint32_t dwStyleEx = FWL_STYLEEXT_CKB_SignShapeCross;
  switch (button_->GetMark()) {
    case XFA_AttributeValue::Check:
      dwStyleEx = FWL_STYLEEXT_CKB_SignShapeCheck;
      break;
    case XFA_AttributeValue::Circle:
      dwStyleEx = FWL_STYLEEXT_CKB_SignShapeCircle;
      break;
    case XFA_AttributeValue::Cross:
      break;
    case XFA_AttributeValue::Diamond:
      dwStyleEx = FWL_STYLEEXT_CKB_SignShapeDiamond;
      break;
    case XFA_AttributeValue::Square:
      dwStyleEx = FWL_STYLEEXT_CKB_SignShapeSquare;
      break;
    case XFA_AttributeValue::Star:
      dwStyleEx = FWL_STYLEEXT_CKB_SignShapeStar;
      break;
    default: {
      if (button_->IsRound())
        dwStyleEx = FWL_STYLEEXT_CKB_SignShapeCircle;
    } break;
  }
  if (button_->IsAllowNeutral())
    dwStyleEx |= FWL_STYLEEXT_CKB_3State;

  pCheckBox->ModifyStyleExts(
      dwStyleEx, FWL_STYLEEXT_CKB_SignShapeMask | FWL_STYLEEXT_CKB_3State);
}

bool CXFA_FFCheckButton::PerformLayout() {
  CXFA_FFWidget::PerformLayout();

  float fCheckSize = m_pNode->GetCheckButtonSize();
  CXFA_Margin* margin = m_pNode->GetMarginIfExists();
  CFX_RectF rtWidget = GetRectWithoutRotate();
  XFA_RectWithoutMargin(&rtWidget, margin);

  XFA_AttributeValue iCapPlacement = XFA_AttributeValue::Unknown;
  float fCapReserve = 0;
  CXFA_Caption* caption = m_pNode->GetCaptionIfExists();
  if (caption && caption->IsVisible()) {
    m_CaptionRect = rtWidget;
    iCapPlacement = caption->GetPlacementType();
    fCapReserve = caption->GetReserve();
    if (fCapReserve <= 0) {
      if (iCapPlacement == XFA_AttributeValue::Top ||
          iCapPlacement == XFA_AttributeValue::Bottom) {
        fCapReserve = rtWidget.height - fCheckSize;
      } else {
        fCapReserve = rtWidget.width - fCheckSize;
      }
    }
  }

  XFA_AttributeValue iHorzAlign = XFA_AttributeValue::Left;
  XFA_AttributeValue iVertAlign = XFA_AttributeValue::Top;
  CXFA_Para* para = m_pNode->GetParaIfExists();
  if (para) {
    iHorzAlign = para->GetHorizontalAlign();
    iVertAlign = para->GetVerticalAlign();
  }

  m_UIRect = rtWidget;
  CXFA_Margin* captionMargin = caption ? caption->GetMarginIfExists() : nullptr;
  switch (iCapPlacement) {
    case XFA_AttributeValue::Left: {
      m_CaptionRect.width = fCapReserve;
      CapLeftRightPlacement(captionMargin);
      m_UIRect.width -= fCapReserve;
      m_UIRect.left += fCapReserve;
      break;
    }
    case XFA_AttributeValue::Top: {
      m_CaptionRect.height = fCapReserve;
      XFA_RectWithoutMargin(&m_CaptionRect, captionMargin);
      m_UIRect.height -= fCapReserve;
      m_UIRect.top += fCapReserve;
      break;
    }
    case XFA_AttributeValue::Right: {
      m_CaptionRect.left = m_CaptionRect.right() - fCapReserve;
      m_CaptionRect.width = fCapReserve;
      CapLeftRightPlacement(captionMargin);
      m_UIRect.width -= fCapReserve;
      break;
    }
    case XFA_AttributeValue::Bottom: {
      m_CaptionRect.top = m_CaptionRect.bottom() - fCapReserve;
      m_CaptionRect.height = fCapReserve;
      XFA_RectWithoutMargin(&m_CaptionRect, captionMargin);
      m_UIRect.height -= fCapReserve;
      break;
    }
    case XFA_AttributeValue::Inline:
      break;
    default:
      iHorzAlign = XFA_AttributeValue::Right;
      break;
  }

  if (iHorzAlign == XFA_AttributeValue::Center)
    m_UIRect.left += (m_UIRect.width - fCheckSize) / 2;
  else if (iHorzAlign == XFA_AttributeValue::Right)
    m_UIRect.left = m_UIRect.right() - fCheckSize;

  if (iVertAlign == XFA_AttributeValue::Middle)
    m_UIRect.top += (m_UIRect.height - fCheckSize) / 2;
  else if (iVertAlign == XFA_AttributeValue::Bottom)
    m_UIRect.top = m_UIRect.bottom() - fCheckSize;

  m_UIRect.width = fCheckSize;
  m_UIRect.height = fCheckSize;
  AddUIMargin(iCapPlacement);
  m_CheckBoxRect = m_UIRect;
  CXFA_Border* borderUI = m_pNode->GetUIBorder();
  if (borderUI) {
    CXFA_Margin* borderMargin = borderUI->GetMarginIfExists();
    XFA_RectWithoutMargin(&m_UIRect, borderMargin);
  }

  m_UIRect.Normalize();
  LayoutCaption();
  SetFWLRect();
  if (GetNormalWidget())
    GetNormalWidget()->Update();

  return true;
}

void CXFA_FFCheckButton::CapLeftRightPlacement(
    const CXFA_Margin* captionMargin) {
  XFA_RectWithoutMargin(&m_CaptionRect, captionMargin);
  if (m_CaptionRect.height < 0)
    m_CaptionRect.top += m_CaptionRect.height;
  if (m_CaptionRect.width < 0) {
    m_CaptionRect.left += m_CaptionRect.width;
    m_CaptionRect.width = -m_CaptionRect.width;
  }
}

void CXFA_FFCheckButton::AddUIMargin(XFA_AttributeValue iCapPlacement) {
  CFX_RectF rtUIMargin = m_pNode->GetUIMargin();
  m_UIRect.top -= rtUIMargin.top / 2 - rtUIMargin.height / 2;

  float fLeftAddRight = rtUIMargin.left + rtUIMargin.width;
  float fTopAddBottom = rtUIMargin.top + rtUIMargin.height;
  if (m_UIRect.width < fLeftAddRight) {
    if (iCapPlacement == XFA_AttributeValue::Right ||
        iCapPlacement == XFA_AttributeValue::Left) {
      m_UIRect.left -= fLeftAddRight - m_UIRect.width;
    } else {
      m_UIRect.left -= 2 * (fLeftAddRight - m_UIRect.width);
    }
    m_UIRect.width += 2 * (fLeftAddRight - m_UIRect.width);
  }
  if (m_UIRect.height < fTopAddBottom) {
    if (iCapPlacement == XFA_AttributeValue::Right)
      m_UIRect.left -= fTopAddBottom - m_UIRect.height;

    m_UIRect.top -= fTopAddBottom - m_UIRect.height;
    m_UIRect.height += 2 * (fTopAddBottom - m_UIRect.height);
  }
}

void CXFA_FFCheckButton::RenderWidget(CFGAS_GEGraphics* pGS,
                                      const CFX_Matrix& matrix,
                                      HighlightOption highlight) {
  if (!HasVisibleStatus())
    return;

  CFX_Matrix mtRotate = GetRotateMatrix();
  mtRotate.Concat(matrix);

  CXFA_FFWidget::RenderWidget(pGS, mtRotate, highlight);
  DrawBorderWithFlag(pGS, m_pNode->GetUIBorder(), m_UIRect, mtRotate,
                     button_->IsRound());
  RenderCaption(pGS, mtRotate);
  DrawHighlight(pGS, mtRotate, highlight,
                button_->IsRound() ? kRoundShape : kSquareShape);
  CFX_Matrix mt(1, 0, 0, 1, m_CheckBoxRect.left, m_CheckBoxRect.top);
  mt.Concat(mtRotate);
  GetApp()->GetFWLWidgetMgr()->OnDrawWidget(GetNormalWidget(), pGS, mt);
}

bool CXFA_FFCheckButton::OnLButtonUp(Mask<XFA_FWL_KeyFlag> dwFlags,
                                     const CFX_PointF& point) {
  if (!GetNormalWidget() || !IsButtonDown())
    return false;

  SetButtonDown(false);
  CFWL_MessageMouse msg(GetNormalWidget(),
                        CFWL_MessageMouse::MouseCommand::kLeftButtonUp, dwFlags,
                        FWLToClient(point));
  SendMessageToFWLWidget(&msg);
  return true;
}

XFA_CheckState CXFA_FFCheckButton::FWLState2XFAState() {
  uint32_t dwState = GetNormalWidget()->GetStates();
  if (dwState & FWL_STATE_CKB_Checked)
    return XFA_CheckState::kOn;
  if (dwState & FWL_STATE_CKB_Neutral)
    return XFA_CheckState::kNeutral;
  return XFA_CheckState::kOff;
}

bool CXFA_FFCheckButton::CommitData() {
  m_pNode->SetCheckState(FWLState2XFAState());
  return true;
}

bool CXFA_FFCheckButton::IsDataChanged() {
  XFA_CheckState eCheckState = FWLState2XFAState();
  return m_pNode->GetCheckState() != eCheckState;
}

void CXFA_FFCheckButton::SetFWLCheckState(XFA_CheckState eCheckState) {
  if (eCheckState == XFA_CheckState::kNeutral)
    GetNormalWidget()->SetStates(FWL_STATE_CKB_Neutral);
  else if (eCheckState == XFA_CheckState::kOn)
    GetNormalWidget()->SetStates(FWL_STATE_CKB_Checked);
  else
    GetNormalWidget()->RemoveStates(FWL_STATE_CKB_Checked);
}

bool CXFA_FFCheckButton::UpdateFWLData() {
  if (!GetNormalWidget())
    return false;

  SetFWLCheckState(m_pNode->GetCheckState());
  GetNormalWidget()->Update();
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
      eParam.m_wsPrevText = m_pNode->GetValue(XFA_ValuePicture::kRaw);

      CXFA_Node* exclNode = m_pNode->GetExclGroupIfExists();
      if (ProcessCommittedData()) {
        if (exclNode) {
          m_pDocView->AddValidateNode(exclNode);
          m_pDocView->AddCalculateNode(exclNode);
          exclNode->ProcessEvent(GetDocView(), XFA_AttributeValue::Change,
                                 &eParam);
        }
        m_pNode->ProcessEvent(GetDocView(), XFA_AttributeValue::Change,
                              &eParam);
      } else {
        SetFWLCheckState(m_pNode->GetCheckState());
      }
      if (exclNode) {
        exclNode->ProcessEvent(GetDocView(), XFA_AttributeValue::Click,
                               &eParam);
      }
      m_pNode->ProcessEvent(GetDocView(), XFA_AttributeValue::Click, &eParam);
      break;
    }
    default:
      break;
  }
  m_pOldDelegate->OnProcessEvent(pEvent);
}

void CXFA_FFCheckButton::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                      const CFX_Matrix& matrix) {
  m_pOldDelegate->OnDrawWidget(pGraphics, matrix);
}

FormFieldType CXFA_FFCheckButton::GetFormFieldType() {
  return FormFieldType::kXFA_CheckBox;
}
