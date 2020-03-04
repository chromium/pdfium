// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_fftextedit.h"

#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/cfwl_datetimepicker.h"
#include "xfa/fwl/cfwl_edit.h"
#include "xfa/fwl/cfwl_eventtarget.h"
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

CXFA_FFTextEdit::~CXFA_FFTextEdit() {
  if (GetNormalWidget()) {
    CFWL_NoteDriver* pNoteDriver =
        GetNormalWidget()->GetOwnerApp()->GetNoteDriver();
    pNoteDriver->UnregisterEventTarget(GetNormalWidget());
  }
}

bool CXFA_FFTextEdit::LoadWidget() {
  ASSERT(!IsLoaded());

  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retain_layout(m_pLayoutItem.Get());

  auto pNewWidget = pdfium::MakeUnique<CFWL_Edit>(
      GetFWLApp(), pdfium::MakeUnique<CFWL_WidgetProperties>(), nullptr);
  CFWL_Edit* pFWLEdit = pNewWidget.get();
  SetNormalWidget(std::move(pNewWidget));
  pFWLEdit->SetAdapterIface(this);

  CFWL_NoteDriver* pNoteDriver = pFWLEdit->GetOwnerApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(pFWLEdit, pFWLEdit);
  m_pOldDelegate = pFWLEdit->GetDelegate();
  pFWLEdit->SetDelegate(this);

  {
    CFWL_Widget::ScopedUpdateLock update_lock(pFWLEdit);
    UpdateWidgetProperty();
    pFWLEdit->SetText(m_pNode->GetValue(XFA_VALUEPICTURE_Display));
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
      dwStyle |= FWL_WGTSTYLE_VScroll;
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

  Optional<int32_t> numCells = m_pNode->GetNumberOfCells();
  if (!numCells) {
    pWidget->SetLimit(iMaxChars);
  } else if (*numCells == 0) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_CombText;
    pWidget->SetLimit(iMaxChars > 0 ? iMaxChars : 1);
  } else {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_CombText;
    pWidget->SetLimit(*numCells);
  }

  dwExtendedStyle |= GetAlignment();
  GetNormalWidget()->ModifyStyles(dwStyle, 0xFFFFFFFF);
  GetNormalWidget()->ModifyStylesEx(dwExtendedStyle, 0xFFFFFFFF);
}

bool CXFA_FFTextEdit::AcceptsFocusOnButtonDown(uint32_t dwFlags,
                                               const CFX_PointF& point,
                                               FWL_MouseCommand command) {
  if (command == FWL_MouseCommand::RightButtonDown && !m_pNode->IsOpenAccess())
    return false;
  if (!PtInActiveRect(point))
    return false;

  return true;
}

bool CXFA_FFTextEdit::OnLButtonDown(uint32_t dwFlags, const CFX_PointF& point) {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retainer(m_pLayoutItem.Get());

  if (!IsFocused()) {
    GetLayoutItem()->SetStatusBits(XFA_WidgetStatus_Focused);
    UpdateFWLData();
    InvalidateRect();
  }
  SetButtonDown(true);
  SendMessageToFWLWidget(pdfium::MakeUnique<CFWL_MessageMouse>(
      GetNormalWidget(), FWL_MouseCommand::LeftButtonDown, dwFlags,
      FWLToClient(point)));

  return true;
}

bool CXFA_FFTextEdit::OnRButtonDown(uint32_t dwFlags, const CFX_PointF& point) {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retainer(m_pLayoutItem.Get());

  if (!IsFocused()) {
    GetLayoutItem()->SetStatusBits(XFA_WidgetStatus_Focused);
    UpdateFWLData();
    InvalidateRect();
  }
  SetButtonDown(true);
  SendMessageToFWLWidget(pdfium::MakeUnique<CFWL_MessageMouse>(
      nullptr, FWL_MouseCommand::RightButtonDown, dwFlags, FWLToClient(point)));

  return true;
}

bool CXFA_FFTextEdit::OnRButtonUp(uint32_t dwFlags, const CFX_PointF& point) {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retainer(m_pLayoutItem.Get());

  if (!CXFA_FFField::OnRButtonUp(dwFlags, point))
    return false;

  GetDoc()->GetDocEnvironment()->PopupMenu(this, point);
  return true;
}

bool CXFA_FFTextEdit::OnSetFocus(CXFA_FFWidget* pOldWidget) {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retainer(m_pLayoutItem.Get());

  ObservedPtr<CXFA_FFWidget> pOldWatched(pOldWidget);
  GetLayoutItem()->ClearStatusBits(XFA_WidgetStatus_TextEditValueChanged);
  if (!IsFocused()) {
    GetLayoutItem()->SetStatusBits(XFA_WidgetStatus_Focused);
    UpdateFWLData();
    InvalidateRect();
  }
  if (!CXFA_FFWidget::OnSetFocus(pOldWatched.Get()))
    return false;

  SendMessageToFWLWidget(
      pdfium::MakeUnique<CFWL_MessageSetFocus>(nullptr, GetNormalWidget()));

  return true;
}

bool CXFA_FFTextEdit::OnKillFocus(CXFA_FFWidget* pNewWidget) {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retainer(m_pLayoutItem.Get());

  ObservedPtr<CXFA_FFWidget> pNewWatched(pNewWidget);
  SendMessageToFWLWidget(
      pdfium::MakeUnique<CFWL_MessageKillFocus>(nullptr, GetNormalWidget()));

  GetLayoutItem()->ClearStatusBits(XFA_WidgetStatus_Focused);
  SetEditScrollOffset();
  ProcessCommittedData();
  UpdateFWLData();
  InvalidateRect();

  if (!CXFA_FFWidget::OnKillFocus(pNewWatched.Get()))
    return false;

  GetLayoutItem()->ClearStatusBits(XFA_WidgetStatus_TextEditValueChanged);
  return true;
}

bool CXFA_FFTextEdit::CommitData() {
  WideString wsText = ToEdit(GetNormalWidget())->GetText();
  if (m_pNode->SetValue(XFA_VALUEPICTURE_Edit, wsText)) {
    GetDoc()->GetDocView()->UpdateUIDisplay(m_pNode.Get(), this);
    return true;
  }
  ValidateNumberField(wsText);
  return false;
}

void CXFA_FFTextEdit::ValidateNumberField(const WideString& wsText) {
  if (GetNode()->GetFFWidgetType() != XFA_FFWidgetType::kNumericEdit)
    return;

  IXFA_AppProvider* pAppProvider = GetAppProvider();
  if (!pAppProvider)
    return;

  WideString wsSomField = GetNode()->GetSOMExpression();
  pAppProvider->MsgBox(
      wsText + WideString::FromASCII(" can not contain ") + wsSomField,
      pAppProvider->GetAppTitle(), static_cast<uint32_t>(AlertIcon::kError),
      static_cast<uint32_t>(AlertButton::kOK));
}

bool CXFA_FFTextEdit::IsDataChanged() {
  return GetLayoutItem()->TestStatusBits(XFA_WidgetStatus_TextEditValueChanged);
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

  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retainer(m_pLayoutItem.Get());
  XFA_VALUEPICTURE eType = XFA_VALUEPICTURE_Display;
  if (IsFocused())
    eType = XFA_VALUEPICTURE_Edit;

  bool bUpdate = false;
  if (m_pNode->GetFFWidgetType() == XFA_FFWidgetType::kTextEdit &&
      !m_pNode->GetNumberOfCells()) {
    XFA_Element elementType;
    int32_t iMaxChars;
    std::tie(elementType, iMaxChars) = m_pNode->GetMaxChars();
    if (elementType == XFA_Element::ExData)
      iMaxChars = eType == XFA_VALUEPICTURE_Edit ? iMaxChars : 0;
    if (pEdit->GetLimit() != iMaxChars) {
      pEdit->SetLimit(iMaxChars);
      bUpdate = true;
    }
  } else if (m_pNode->GetFFWidgetType() == XFA_FFWidgetType::kBarcode) {
    int32_t nDataLen = 0;
    if (eType == XFA_VALUEPICTURE_Edit) {
      nDataLen = static_cast<CXFA_Barcode*>(m_pNode->GetUIChildNode())
                     ->GetDataLength()
                     .value_or(0);
    }

    pEdit->SetLimit(nDataLen);
    bUpdate = true;
  }
  WideString wsText = m_pNode->GetValue(eType);
  WideString wsOldText = pEdit->GetText();
  if (wsText != wsOldText || (eType == XFA_VALUEPICTURE_Edit && bUpdate)) {
    pEdit->SetTextSkipNotify(wsText);
    bUpdate = true;
  }
  if (bUpdate)
    GetNormalWidget()->Update();

  return true;
}

void CXFA_FFTextEdit::OnTextWillChange(CFWL_Widget* pWidget,
                                       CFWL_EventTextWillChange* event) {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retainer(m_pLayoutItem.Get());

  GetLayoutItem()->SetStatusBits(XFA_WidgetStatus_TextEditValueChanged);

  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_Change;
  eParam.m_wsChange = event->change_text;
  eParam.m_pTarget = m_pNode.Get();
  eParam.m_wsPrevText = event->previous_text;
  eParam.m_iSelStart = static_cast<int32_t>(event->selection_start);
  eParam.m_iSelEnd = static_cast<int32_t>(event->selection_end);

  m_pNode->ProcessEvent(GetDocView(), XFA_AttributeValue::Change, &eParam);

  // Copy the data back out of the EventParam and into the TextChanged event so
  // it can propagate back to the calling widget.
  event->cancelled = eParam.m_bCancelAction;
  event->change_text = std::move(eParam.m_wsChange);
  event->selection_start = static_cast<size_t>(eParam.m_iSelStart);
  event->selection_end = static_cast<size_t>(eParam.m_iSelEnd);
}

void CXFA_FFTextEdit::OnTextFull(CFWL_Widget* pWidget) {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retainer(m_pLayoutItem.Get());

  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_Full;
  eParam.m_pTarget = m_pNode.Get();
  m_pNode->ProcessEvent(GetDocView(), XFA_AttributeValue::Full, &eParam);
}

void CXFA_FFTextEdit::OnProcessMessage(CFWL_Message* pMessage) {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retainer(m_pLayoutItem.Get());

  m_pOldDelegate->OnProcessMessage(pMessage);
}

void CXFA_FFTextEdit::OnProcessEvent(CFWL_Event* pEvent) {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retain_layout(m_pLayoutItem.Get());

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

void CXFA_FFTextEdit::OnDrawWidget(CXFA_Graphics* pGraphics,
                                   const CFX_Matrix& matrix) {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retainer(m_pLayoutItem.Get());

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
  if (ToEdit(GetNormalWidget())->GetStylesEx() & FWL_STYLEEXT_EDT_ReadOnly)
    return false;
  return ToEdit(GetNormalWidget())->HasSelection();
}

bool CXFA_FFTextEdit::CanPaste() {
  return !(ToEdit(GetNormalWidget())->GetStylesEx() &
           FWL_STYLEEXT_EDT_ReadOnly);
}

bool CXFA_FFTextEdit::CanSelectAll() {
  return ToEdit(GetNormalWidget())->GetTextLength() > 0;
}

bool CXFA_FFTextEdit::Undo() {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retain_layout(m_pLayoutItem.Get());

  return ToEdit(GetNormalWidget())->Undo();
}

bool CXFA_FFTextEdit::Redo() {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retain_layout(m_pLayoutItem.Get());

  return ToEdit(GetNormalWidget())->Redo();
}

Optional<WideString> CXFA_FFTextEdit::Copy() {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retain_layout(m_pLayoutItem.Get());

  return ToEdit(GetNormalWidget())->Copy();
}

Optional<WideString> CXFA_FFTextEdit::Cut() {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retain_layout(m_pLayoutItem.Get());

  return ToEdit(GetNormalWidget())->Cut();
}

bool CXFA_FFTextEdit::Paste(const WideString& wsPaste) {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retain_layout(m_pLayoutItem.Get());

  return ToEdit(GetNormalWidget())->Paste(wsPaste);
}

void CXFA_FFTextEdit::SelectAll() {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retain_layout(m_pLayoutItem.Get());

  ToEdit(GetNormalWidget())->SelectAll();
}

void CXFA_FFTextEdit::Delete() {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retain_layout(m_pLayoutItem.Get());

  ToEdit(GetNormalWidget())->ClearText();
}

void CXFA_FFTextEdit::DeSelect() {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retain_layout(m_pLayoutItem.Get());

  ToEdit(GetNormalWidget())->ClearSelection();
}

WideString CXFA_FFTextEdit::GetText() {
  return ToEdit(GetNormalWidget())->GetText();
}

FormFieldType CXFA_FFTextEdit::GetFormFieldType() {
  return FormFieldType::kXFA_TextField;
}
