// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffcheckbutton.h"

#include "core/fxcrt/check.h"
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
  visitor->Trace(old_delegate_);
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
  old_delegate_ = pCheckBox->GetDelegate();
  pCheckBox->SetDelegate(this);
  if (node_->IsRadioButton()) {
    pCheckBox->ModifyStyleExts(FWL_STYLEEXT_CKB_RadioButton, 0xFFFFFFFF);
  }

  {
    CFWL_Widget::ScopedUpdateLock update_lock(pCheckBox);
    UpdateWidgetProperty();
    SetFWLCheckState(node_->GetCheckState());
  }

  return CXFA_FFField::LoadWidget();
}

void CXFA_FFCheckButton::UpdateWidgetProperty() {
  auto* pCheckBox = static_cast<CFWL_CheckBox*>(GetNormalWidget());
  if (!pCheckBox)
    return;

  pCheckBox->SetBoxSize(node_->GetCheckButtonSize());
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

void CXFA_FFCheckButton::PerformLayout() {
  CXFA_FFWidget::PerformLayout();

  float fCheckSize = node_->GetCheckButtonSize();
  CXFA_Margin* margin = node_->GetMarginIfExists();
  CFX_RectF rtWidget = GetRectWithoutRotate();
  XFA_RectWithoutMargin(&rtWidget, margin);

  XFA_AttributeValue iCapPlacement = XFA_AttributeValue::Unknown;
  float fCapReserve = 0;
  CXFA_Caption* caption = node_->GetCaptionIfExists();
  if (caption && caption->IsVisible()) {
    caption_rect_ = rtWidget;
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
  CXFA_Para* para = node_->GetParaIfExists();
  if (para) {
    iHorzAlign = para->GetHorizontalAlign();
    iVertAlign = para->GetVerticalAlign();
  }

  uirect_ = rtWidget;
  CXFA_Margin* captionMargin = caption ? caption->GetMarginIfExists() : nullptr;
  switch (iCapPlacement) {
    case XFA_AttributeValue::Left: {
      caption_rect_.width = fCapReserve;
      CapLeftRightPlacement(captionMargin);
      uirect_.width -= fCapReserve;
      uirect_.left += fCapReserve;
      break;
    }
    case XFA_AttributeValue::Top: {
      caption_rect_.height = fCapReserve;
      XFA_RectWithoutMargin(&caption_rect_, captionMargin);
      uirect_.height -= fCapReserve;
      uirect_.top += fCapReserve;
      break;
    }
    case XFA_AttributeValue::Right: {
      caption_rect_.left = caption_rect_.right() - fCapReserve;
      caption_rect_.width = fCapReserve;
      CapLeftRightPlacement(captionMargin);
      uirect_.width -= fCapReserve;
      break;
    }
    case XFA_AttributeValue::Bottom: {
      caption_rect_.top = caption_rect_.bottom() - fCapReserve;
      caption_rect_.height = fCapReserve;
      XFA_RectWithoutMargin(&caption_rect_, captionMargin);
      uirect_.height -= fCapReserve;
      break;
    }
    case XFA_AttributeValue::Inline:
      break;
    default:
      iHorzAlign = XFA_AttributeValue::Right;
      break;
  }

  if (iHorzAlign == XFA_AttributeValue::Center)
    uirect_.left += (uirect_.width - fCheckSize) / 2;
  else if (iHorzAlign == XFA_AttributeValue::Right)
    uirect_.left = uirect_.right() - fCheckSize;

  if (iVertAlign == XFA_AttributeValue::Middle)
    uirect_.top += (uirect_.height - fCheckSize) / 2;
  else if (iVertAlign == XFA_AttributeValue::Bottom)
    uirect_.top = uirect_.bottom() - fCheckSize;

  uirect_.width = fCheckSize;
  uirect_.height = fCheckSize;
  AddUIMargin(iCapPlacement);
  check_box_rect_ = uirect_;
  CXFA_Border* borderUI = node_->GetUIBorder();
  if (borderUI) {
    CXFA_Margin* borderMargin = borderUI->GetMarginIfExists();
    XFA_RectWithoutMargin(&uirect_, borderMargin);
  }

  uirect_.Normalize();
  LayoutCaption();
  SetFWLRect();
  if (GetNormalWidget())
    GetNormalWidget()->Update();
}

void CXFA_FFCheckButton::CapLeftRightPlacement(
    const CXFA_Margin* captionMargin) {
  XFA_RectWithoutMargin(&caption_rect_, captionMargin);
  if (caption_rect_.height < 0) {
    caption_rect_.top += caption_rect_.height;
  }
  if (caption_rect_.width < 0) {
    caption_rect_.left += caption_rect_.width;
    caption_rect_.width = -caption_rect_.width;
  }
}

void CXFA_FFCheckButton::AddUIMargin(XFA_AttributeValue iCapPlacement) {
  CFX_RectF rtUIMargin = node_->GetUIMargin();
  uirect_.top -= rtUIMargin.top / 2 - rtUIMargin.height / 2;

  float fLeftAddRight = rtUIMargin.left + rtUIMargin.width;
  float fTopAddBottom = rtUIMargin.top + rtUIMargin.height;
  if (uirect_.width < fLeftAddRight) {
    if (iCapPlacement == XFA_AttributeValue::Right ||
        iCapPlacement == XFA_AttributeValue::Left) {
      uirect_.left -= fLeftAddRight - uirect_.width;
    } else {
      uirect_.left -= 2 * (fLeftAddRight - uirect_.width);
    }
    uirect_.width += 2 * (fLeftAddRight - uirect_.width);
  }
  if (uirect_.height < fTopAddBottom) {
    if (iCapPlacement == XFA_AttributeValue::Right)
      uirect_.left -= fTopAddBottom - uirect_.height;

    uirect_.top -= fTopAddBottom - uirect_.height;
    uirect_.height += 2 * (fTopAddBottom - uirect_.height);
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
  DrawBorderWithFlag(pGS, node_->GetUIBorder(), uirect_, mtRotate,
                     button_->IsRound());
  RenderCaption(pGS, mtRotate);
  DrawHighlight(pGS, mtRotate, highlight,
                button_->IsRound() ? kRoundShape : kSquareShape);
  CFX_Matrix mt(1, 0, 0, 1, check_box_rect_.left, check_box_rect_.top);
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
  node_->SetCheckState(FWLState2XFAState());
  return true;
}

bool CXFA_FFCheckButton::IsDataChanged() {
  XFA_CheckState eCheckState = FWLState2XFAState();
  return node_->GetCheckState() != eCheckState;
}

void CXFA_FFCheckButton::SetFWLCheckState(XFA_CheckState eCheckState) {
  if (eCheckState == XFA_CheckState::kNeutral)
    GetNormalWidget()->SetStates(FWL_STATE_CKB_Neutral);
  else if (eCheckState == XFA_CheckState::kOn)
    GetNormalWidget()->SetStates(FWL_STATE_CKB_Checked);
  else
    GetNormalWidget()->RemoveStates(FWL_STATE_CKB_Checked);
}

void CXFA_FFCheckButton::UpdateFWLData() {
  if (!GetNormalWidget()) {
    return;
  }
  SetFWLCheckState(node_->GetCheckState());
  GetNormalWidget()->Update();
}

void CXFA_FFCheckButton::OnProcessMessage(CFWL_Message* pMessage) {
  old_delegate_->OnProcessMessage(pMessage);
}

void CXFA_FFCheckButton::OnProcessEvent(CFWL_Event* pEvent) {
  CXFA_FFField::OnProcessEvent(pEvent);
  switch (pEvent->GetType()) {
    case CFWL_Event::Type::CheckStateChanged: {
      CXFA_EventParam eParam(XFA_EVENT_Change);
      eParam.prev_text_ = node_->GetValue(XFA_ValuePicture::kRaw);
      CXFA_Node* exclNode = node_->GetExclGroupIfExists();
      if (ProcessCommittedData()) {
        if (exclNode) {
          doc_view_->AddValidateNode(exclNode);
          doc_view_->AddCalculateNode(exclNode);
          exclNode->ProcessEvent(GetDocView(), XFA_AttributeValue::Change,
                                 &eParam);
        }
        node_->ProcessEvent(GetDocView(), XFA_AttributeValue::Change, &eParam);
      } else {
        SetFWLCheckState(node_->GetCheckState());
      }
      if (exclNode) {
        exclNode->ProcessEvent(GetDocView(), XFA_AttributeValue::Click,
                               &eParam);
      }
      node_->ProcessEvent(GetDocView(), XFA_AttributeValue::Click, &eParam);
      break;
    }
    default:
      break;
  }
  old_delegate_->OnProcessEvent(pEvent);
}

void CXFA_FFCheckButton::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                      const CFX_Matrix& matrix) {
  old_delegate_->OnDrawWidget(pGraphics, matrix);
}

FormFieldType CXFA_FFCheckButton::GetFormFieldType() {
  return FormFieldType::kXFA_CheckBox;
}
