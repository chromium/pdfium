// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_fftextedit.h"

#include <utility>

#include "core/fxcrt/check.h"
#include "xfa/fwl/cfwl_datetimepicker.h"
#include "xfa/fwl/cfwl_edit.h"
#include "xfa/fwl/cfwl_eventtextwillchange.h"
#include "xfa/fwl/cfwl_messagekillfocus.h"
#include "xfa/fwl/cfwl_messagesetfocus.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/parser/cxfa_barcode.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_para.h"

namespace {

CFWL_Edit* ToEdit(CFWL_Widget* widget) {
  return static_cast<CFWL_Edit*>(widget);
}

}  // namespace

CXFA_FFTextEdit::CXFA_FFTextEdit(CXFA_Node* pNode) : CXFA_FFField(pNode) {}

CXFA_FFTextEdit::~CXFA_FFTextEdit() = default;

void CXFA_FFTextEdit::PreFinalize() {
  if (GetNormalWidget()) {
    CFWL_NoteDriver* pNoteDriver =
        GetNormalWidget()->GetFWLApp()->GetNoteDriver();
    pNoteDriver->UnregisterEventTarget(GetNormalWidget());
  }
}

void CXFA_FFTextEdit::Trace(cppgc::Visitor* visitor) const {
  CXFA_FFField::Trace(visitor);
  visitor->Trace(old_delegate_);
}

bool CXFA_FFTextEdit::LoadWidget() {
  DCHECK(!IsLoaded());

  CFWL_Edit* pFWLEdit = cppgc::MakeGarbageCollected<CFWL_Edit>(
      GetFWLApp()->GetHeap()->GetAllocationHandle(), GetFWLApp(),
      CFWL_Widget::Properties(), nullptr);
  SetNormalWidget(pFWLEdit);
  pFWLEdit->SetAdapterIface(this);

  CFWL_NoteDriver* pNoteDriver = pFWLEdit->GetFWLApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(pFWLEdit, pFWLEdit);
  old_delegate_ = pFWLEdit->GetDelegate();
  pFWLEdit->SetDelegate(this);

  {
    CFWL_Widget::ScopedUpdateLock update_lock(pFWLEdit);
    UpdateWidgetProperty();
    pFWLEdit->SetText(node_->GetValue(XFA_ValuePicture::kDisplay));
  }

  return CXFA_FFField::LoadWidget();
}

void CXFA_FFTextEdit::UpdateWidgetProperty() {
  CFWL_Edit* pWidget = ToEdit(GetNormalWidget());
  if (!pWidget)
    return;

  uint32_t dwStyle = 0;
  uint32_t dwExtendedStyle =
      FWL_STYLEEXT_EDT_ShowScrollbarFocus | FWL_STYLEEXT_EDT_OuterScrollbar;
  dwExtendedStyle |= UpdateUIProperty();
  if (node_->IsMultiLine()) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_MultiLine | FWL_STYLEEXT_EDT_WantReturn;
    if (!node_->IsVerticalScrollPolicyOff()) {
      dwStyle |= FWL_STYLE_WGT_VScroll;
      dwExtendedStyle |= FWL_STYLEEXT_EDT_AutoVScroll;
    }
  } else if (!node_->IsHorizontalScrollPolicyOff()) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_AutoHScroll;
  }
  if (!node_->IsOpenAccess() || !GetDoc()->GetXFADoc()->IsInteractive()) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_ReadOnly;
    dwExtendedStyle |= FWL_STYLEEXT_EDT_MultiLine;
  }

  auto [eType, iMaxChars] = node_->GetMaxChars();
  if (eType == XFA_Element::ExData)
    iMaxChars = 0;

  std::optional<int32_t> numCells = node_->GetNumberOfCells();
  if (!numCells.has_value()) {
    pWidget->SetLimit(iMaxChars);
  } else if (numCells == 0) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_CombText;
    pWidget->SetLimit(iMaxChars > 0 ? iMaxChars : 1);
  } else {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_CombText;
    pWidget->SetLimit(numCells.value());
  }

  dwExtendedStyle |= GetAlignment();
  GetNormalWidget()->ModifyStyles(dwStyle, 0xFFFFFFFF);
  GetNormalWidget()->ModifyStyleExts(dwExtendedStyle, 0xFFFFFFFF);
}

bool CXFA_FFTextEdit::AcceptsFocusOnButtonDown(
    Mask<XFA_FWL_KeyFlag> dwFlags,
    const CFX_PointF& point,
    CFWL_MessageMouse::MouseCommand command) {
  if (command == CFWL_MessageMouse::MouseCommand::kRightButtonDown &&
      !node_->IsOpenAccess()) {
    return false;
  }
  if (!PtInActiveRect(point))
    return false;

  return true;
}

bool CXFA_FFTextEdit::OnLButtonDown(Mask<XFA_FWL_KeyFlag> dwFlags,
                                    const CFX_PointF& point) {
  if (!IsFocused()) {
    GetLayoutItem()->SetStatusBits(XFA_WidgetStatus::kFocused);
    UpdateFWLData();
    InvalidateRect();
  }
  SetButtonDown(true);
  CFWL_MessageMouse msg(GetNormalWidget(),
                        CFWL_MessageMouse::MouseCommand::kLeftButtonDown,
                        dwFlags, FWLToClient(point));
  SendMessageToFWLWidget(&msg);
  return true;
}

bool CXFA_FFTextEdit::OnRButtonDown(Mask<XFA_FWL_KeyFlag> dwFlags,
                                    const CFX_PointF& point) {
  if (!IsFocused()) {
    GetLayoutItem()->SetStatusBits(XFA_WidgetStatus::kFocused);
    UpdateFWLData();
    InvalidateRect();
  }
  SetButtonDown(true);
  CFWL_MessageMouse msg(nullptr,
                        CFWL_MessageMouse::MouseCommand::kRightButtonDown,
                        dwFlags, FWLToClient(point));
  SendMessageToFWLWidget(&msg);
  return true;
}

bool CXFA_FFTextEdit::OnRButtonUp(Mask<XFA_FWL_KeyFlag> dwFlags,
                                  const CFX_PointF& point) {
  if (!CXFA_FFField::OnRButtonUp(dwFlags, point))
    return false;

  GetDoc()->PopupMenu(this, point);
  return true;
}

bool CXFA_FFTextEdit::OnSetFocus(CXFA_FFWidget* pOldWidget) {
  GetLayoutItem()->ClearStatusBits(XFA_WidgetStatus::kTextEditValueChanged);
  if (!IsFocused()) {
    GetLayoutItem()->SetStatusBits(XFA_WidgetStatus::kFocused);
    UpdateFWLData();
    InvalidateRect();
  }
  if (!CXFA_FFWidget::OnSetFocus(pOldWidget))
    return false;

  CFWL_MessageSetFocus msg(GetNormalWidget());
  SendMessageToFWLWidget(&msg);
  return true;
}

bool CXFA_FFTextEdit::OnKillFocus(CXFA_FFWidget* pNewWidget) {
  CFWL_MessageKillFocus msg(GetNormalWidget());
  SendMessageToFWLWidget(&msg);

  GetLayoutItem()->ClearStatusBits(XFA_WidgetStatus::kFocused);
  SetEditScrollOffset();
  ProcessCommittedData();
  UpdateFWLData();
  InvalidateRect();

  if (!CXFA_FFWidget::OnKillFocus(pNewWidget))
    return false;

  GetLayoutItem()->ClearStatusBits(XFA_WidgetStatus::kTextEditValueChanged);
  return true;
}

bool CXFA_FFTextEdit::CommitData() {
  WideString wsText = ToEdit(GetNormalWidget())->GetText();
  if (node_->SetValue(XFA_ValuePicture::kEdit, wsText)) {
    GetDoc()->GetDocView()->UpdateUIDisplay(node_.Get(), this);
    return true;
  }
  ValidateNumberField(wsText);
  return false;
}

void CXFA_FFTextEdit::ValidateNumberField(const WideString& wsText) {
  if (GetNode()->GetFFWidgetType() != XFA_FFWidgetType::kNumericEdit)
    return;

  CXFA_FFApp::CallbackIface* pAppProvider = GetAppProvider();
  if (!pAppProvider)
    return;

  WideString wsSomField = GetNode()->GetSOMExpression();
  pAppProvider->MsgBox(
      wsText + WideString::FromASCII(" can not contain ") + wsSomField,
      pAppProvider->GetAppTitle(), static_cast<uint32_t>(AlertIcon::kError),
      static_cast<uint32_t>(AlertButton::kOK));
}

bool CXFA_FFTextEdit::IsDataChanged() {
  return GetLayoutItem()->TestStatusBits(
      XFA_WidgetStatus::kTextEditValueChanged);
}

uint32_t CXFA_FFTextEdit::GetAlignment() {
  CXFA_Para* para = node_->GetParaIfExists();
  if (!para)
    return 0;

  uint32_t dwExtendedStyle = 0;
  switch (para->GetHorizontalAlign()) {
    case XFA_AttributeValue::Center:
      dwExtendedStyle |= FWL_STYLEEXT_EDT_HCenter;
      break;
    case XFA_AttributeValue::Justify:
      dwExtendedStyle |= FWL_STYLEEXT_EDT_Justified;
      break;
    case XFA_AttributeValue::JustifyAll:
    case XFA_AttributeValue::Radix:
      break;
    case XFA_AttributeValue::Right:
      dwExtendedStyle |= FWL_STYLEEXT_EDT_HFar;
      break;
    default:
      dwExtendedStyle |= FWL_STYLEEXT_EDT_HNear;
      break;
  }

  switch (para->GetVerticalAlign()) {
    case XFA_AttributeValue::Middle:
      dwExtendedStyle |= FWL_STYLEEXT_EDT_VCenter;
      break;
    case XFA_AttributeValue::Bottom:
      dwExtendedStyle |= FWL_STYLEEXT_EDT_VFar;
      break;
    default:
      dwExtendedStyle |= FWL_STYLEEXT_EDT_VNear;
      break;
  }
  return dwExtendedStyle;
}

void CXFA_FFTextEdit::UpdateFWLData() {
  CFWL_Edit* pEdit = ToEdit(GetNormalWidget());
  if (!pEdit)
    return;

  XFA_ValuePicture eType = XFA_ValuePicture::kDisplay;
  if (IsFocused())
    eType = XFA_ValuePicture::kEdit;

  bool bUpdate = false;
  if (node_->GetFFWidgetType() == XFA_FFWidgetType::kTextEdit &&
      !node_->GetNumberOfCells().has_value()) {
    auto [elementType, iMaxChars] = node_->GetMaxChars();
    if (elementType == XFA_Element::ExData)
      iMaxChars = eType == XFA_ValuePicture::kEdit ? iMaxChars : 0;
    if (pEdit->GetLimit() != iMaxChars) {
      pEdit->SetLimit(iMaxChars);
      bUpdate = true;
    }
  } else if (node_->GetFFWidgetType() == XFA_FFWidgetType::kBarcode) {
    int32_t nDataLen = 0;
    if (eType == XFA_ValuePicture::kEdit) {
      nDataLen = static_cast<CXFA_Barcode*>(node_->GetUIChildNode())
                     ->GetDataLength()
                     .value_or(0);
    }

    pEdit->SetLimit(nDataLen);
    bUpdate = true;
  }
  WideString wsText = node_->GetValue(eType);
  WideString wsOldText = pEdit->GetText();
  if (wsText != wsOldText || (eType == XFA_ValuePicture::kEdit && bUpdate)) {
    pEdit->SetTextSkipNotify(wsText);
    bUpdate = true;
  }
  if (bUpdate) {
    GetNormalWidget()->Update();
  }
}

void CXFA_FFTextEdit::OnTextWillChange(CFWL_Widget* pWidget,
                                       CFWL_EventTextWillChange* event) {
  GetLayoutItem()->SetStatusBits(XFA_WidgetStatus::kTextEditValueChanged);

  CXFA_EventParam eParam(XFA_EVENT_Change);
  eParam.change_ = event->GetChangeText();
  eParam.prev_text_ = event->GetPreviousText();
  eParam.sel_start_ = static_cast<int32_t>(event->GetSelectionStart());
  eParam.sel_end_ = static_cast<int32_t>(event->GetSelectionEnd());
  node_->ProcessEvent(GetDocView(), XFA_AttributeValue::Change, &eParam);

  // Copy the data back out of the EventParam and into the TextChanged event so
  // it can propagate back to the calling widget.
  event->SetCancelled(eParam.cancel_action_);
  event->SetChangeText(eParam.change_);
  event->SetSelectionStart(static_cast<size_t>(eParam.sel_start_));
  event->SetSelectionEnd(static_cast<size_t>(eParam.sel_end_));
}

void CXFA_FFTextEdit::OnTextFull(CFWL_Widget* pWidget) {
  CXFA_EventParam eParam(XFA_EVENT_Full);
  node_->ProcessEvent(GetDocView(), XFA_AttributeValue::Full, &eParam);
}

void CXFA_FFTextEdit::OnProcessMessage(CFWL_Message* pMessage) {
  old_delegate_->OnProcessMessage(pMessage);
}

void CXFA_FFTextEdit::OnProcessEvent(CFWL_Event* pEvent) {
  CXFA_FFField::OnProcessEvent(pEvent);
  switch (pEvent->GetType()) {
    case CFWL_Event::Type::TextWillChange:
      OnTextWillChange(GetNormalWidget(),
                       static_cast<CFWL_EventTextWillChange*>(pEvent));
      break;
    case CFWL_Event::Type::TextFull:
      OnTextFull(GetNormalWidget());
      break;
    default:
      break;
  }
  old_delegate_->OnProcessEvent(pEvent);
}

void CXFA_FFTextEdit::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                   const CFX_Matrix& matrix) {
  old_delegate_->OnDrawWidget(pGraphics, matrix);
}

bool CXFA_FFTextEdit::CanUndo() {
  return ToEdit(GetNormalWidget())->CanUndo();
}

bool CXFA_FFTextEdit::CanRedo() {
  return ToEdit(GetNormalWidget())->CanRedo();
}

bool CXFA_FFTextEdit::CanCopy() {
  return ToEdit(GetNormalWidget())->HasSelection();
}

bool CXFA_FFTextEdit::CanCut() {
  if (ToEdit(GetNormalWidget())->GetStyleExts() & FWL_STYLEEXT_EDT_ReadOnly)
    return false;
  return ToEdit(GetNormalWidget())->HasSelection();
}

bool CXFA_FFTextEdit::CanPaste() {
  return !(ToEdit(GetNormalWidget())->GetStyleExts() &
           FWL_STYLEEXT_EDT_ReadOnly);
}

bool CXFA_FFTextEdit::CanSelectAll() {
  return ToEdit(GetNormalWidget())->GetTextLength() > 0;
}

bool CXFA_FFTextEdit::Undo() {
  return ToEdit(GetNormalWidget())->Undo();
}

bool CXFA_FFTextEdit::Redo() {
  return ToEdit(GetNormalWidget())->Redo();
}

std::optional<WideString> CXFA_FFTextEdit::Copy() {
  return ToEdit(GetNormalWidget())->Copy();
}

std::optional<WideString> CXFA_FFTextEdit::Cut() {
  return ToEdit(GetNormalWidget())->Cut();
}

bool CXFA_FFTextEdit::Paste(const WideString& wsPaste) {
  return ToEdit(GetNormalWidget())->Paste(wsPaste);
}

void CXFA_FFTextEdit::SelectAll() {
  ToEdit(GetNormalWidget())->SelectAll();
}

void CXFA_FFTextEdit::Delete() {
  ToEdit(GetNormalWidget())->ClearText();
}

void CXFA_FFTextEdit::DeSelect() {
  ToEdit(GetNormalWidget())->ClearSelection();
}

WideString CXFA_FFTextEdit::GetText() {
  return ToEdit(GetNormalWidget())->GetText();
}

FormFieldType CXFA_FFTextEdit::GetFormFieldType() {
  return FormFieldType::kXFA_TextField;
}
