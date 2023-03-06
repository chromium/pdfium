// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_fftextedit.h"

#include <utility>

#include "third_party/base/check.h"
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
  visitor->Trace(m_pOldDelegate);
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
  m_pOldDelegate = pFWLEdit->GetDelegate();
  pFWLEdit->SetDelegate(this);

  {
    CFWL_Widget::ScopedUpdateLock update_lock(pFWLEdit);
    UpdateWidgetProperty();
    pFWLEdit->SetText(m_pNode->GetValue(XFA_ValuePicture::kDisplay));
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
  if (m_pNode->IsMultiLine()) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_MultiLine | FWL_STYLEEXT_EDT_WantReturn;
    if (!m_pNode->IsVerticalScrollPolicyOff()) {
      dwStyle |= FWL_STYLE_WGT_VScroll;
      dwExtendedStyle |= FWL_STYLEEXT_EDT_AutoVScroll;
    }
  } else if (!m_pNode->IsHorizontalScrollPolicyOff()) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_AutoHScroll;
  }
  if (!m_pNode->IsOpenAccess() || !GetDoc()->GetXFADoc()->IsInteractive()) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_ReadOnly;
    dwExtendedStyle |= FWL_STYLEEXT_EDT_MultiLine;
  }

  XFA_Element eType;
  int32_t iMaxChars;
  std::tie(eType, iMaxChars) = m_pNode->GetMaxChars();
  if (eType == XFA_Element::ExData)
    iMaxChars = 0;

  absl::optional<int32_t> numCells = m_pNode->GetNumberOfCells();
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
      !m_pNode->IsOpenAccess()) {
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
  if (m_pNode->SetValue(XFA_ValuePicture::kEdit, wsText)) {
    GetDoc()->GetDocView()->UpdateUIDisplay(m_pNode.Get(), this);
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
  CXFA_Para* para = m_pNode->GetParaIfExists();
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

bool CXFA_FFTextEdit::UpdateFWLData() {
  CFWL_Edit* pEdit = ToEdit(GetNormalWidget());
  if (!pEdit)
    return false;

  XFA_ValuePicture eType = XFA_ValuePicture::kDisplay;
  if (IsFocused())
    eType = XFA_ValuePicture::kEdit;

  bool bUpdate = false;
  if (m_pNode->GetFFWidgetType() == XFA_FFWidgetType::kTextEdit &&
      !m_pNode->GetNumberOfCells().has_value()) {
    XFA_Element elementType;
    int32_t iMaxChars;
    std::tie(elementType, iMaxChars) = m_pNode->GetMaxChars();
    if (elementType == XFA_Element::ExData)
      iMaxChars = eType == XFA_ValuePicture::kEdit ? iMaxChars : 0;
    if (pEdit->GetLimit() != iMaxChars) {
      pEdit->SetLimit(iMaxChars);
      bUpdate = true;
    }
  } else if (m_pNode->GetFFWidgetType() == XFA_FFWidgetType::kBarcode) {
    int32_t nDataLen = 0;
    if (eType == XFA_ValuePicture::kEdit) {
      nDataLen = static_cast<CXFA_Barcode*>(m_pNode->GetUIChildNode())
                     ->GetDataLength()
                     .value_or(0);
    }

    pEdit->SetLimit(nDataLen);
    bUpdate = true;
  }
  WideString wsText = m_pNode->GetValue(eType);
  WideString wsOldText = pEdit->GetText();
  if (wsText != wsOldText || (eType == XFA_ValuePicture::kEdit && bUpdate)) {
    pEdit->SetTextSkipNotify(wsText);
    bUpdate = true;
  }
  if (bUpdate)
    GetNormalWidget()->Update();

  return true;
}

void CXFA_FFTextEdit::OnTextWillChange(CFWL_Widget* pWidget,
                                       CFWL_EventTextWillChange* event) {
  GetLayoutItem()->SetStatusBits(XFA_WidgetStatus::kTextEditValueChanged);

  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_Change;
  eParam.m_wsChange = event->GetChangeText();
  eParam.m_wsPrevText = event->GetPreviousText();
  eParam.m_iSelStart = static_cast<int32_t>(event->GetSelectionStart());
  eParam.m_iSelEnd = static_cast<int32_t>(event->GetSelectionEnd());
  m_pNode->ProcessEvent(GetDocView(), XFA_AttributeValue::Change, &eParam);

  // Copy the data back out of the EventParam and into the TextChanged event so
  // it can propagate back to the calling widget.
  event->SetCancelled(eParam.m_bCancelAction);
  event->SetChangeText(eParam.m_wsChange);
  event->SetSelectionStart(static_cast<size_t>(eParam.m_iSelStart));
  event->SetSelectionEnd(static_cast<size_t>(eParam.m_iSelEnd));
}

void CXFA_FFTextEdit::OnTextFull(CFWL_Widget* pWidget) {
  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_Full;
  m_pNode->ProcessEvent(GetDocView(), XFA_AttributeValue::Full, &eParam);
}

void CXFA_FFTextEdit::OnProcessMessage(CFWL_Message* pMessage) {
  m_pOldDelegate->OnProcessMessage(pMessage);
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
  m_pOldDelegate->OnProcessEvent(pEvent);
}

void CXFA_FFTextEdit::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                   const CFX_Matrix& matrix) {
  m_pOldDelegate->OnDrawWidget(pGraphics, matrix);
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

absl::optional<WideString> CXFA_FFTextEdit::Copy() {
  return ToEdit(GetNormalWidget())->Copy();
}

absl::optional<WideString> CXFA_FFTextEdit::Cut() {
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
