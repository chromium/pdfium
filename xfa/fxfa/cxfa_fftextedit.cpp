// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_fftextedit.h"

#include <utility>

#include "xfa/fwl/cfwl_datetimepicker.h"
#include "xfa/fwl/cfwl_edit.h"
#include "xfa/fwl/cfwl_eventcheckword.h"
#include "xfa/fwl/cfwl_eventtarget.h"
#include "xfa/fwl/cfwl_eventtextchanged.h"
#include "xfa/fwl/cfwl_messagekillfocus.h"
#include "xfa/fwl/cfwl_messagesetfocus.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_para.h"

namespace {

CFWL_Edit* ToEdit(CFWL_Widget* widget) {
  return static_cast<CFWL_Edit*>(widget);
}

}  // namespace

CXFA_FFTextEdit::CXFA_FFTextEdit(CXFA_Node* pNode)
    : CXFA_FFField(pNode), m_pOldDelegate(nullptr) {}

CXFA_FFTextEdit::~CXFA_FFTextEdit() {
  if (m_pNormalWidget) {
    CFWL_NoteDriver* pNoteDriver =
        m_pNormalWidget->GetOwnerApp()->GetNoteDriver();
    pNoteDriver->UnregisterEventTarget(m_pNormalWidget.get());
  }
}

bool CXFA_FFTextEdit::LoadWidget() {
  auto pNewWidget = pdfium::MakeUnique<CFWL_Edit>(
      GetFWLApp(), pdfium::MakeUnique<CFWL_WidgetProperties>(), nullptr);
  CFWL_Edit* pFWLEdit = pNewWidget.get();
  m_pNormalWidget = std::move(pNewWidget);
  m_pNormalWidget->SetLayoutItem(this);

  CFWL_NoteDriver* pNoteDriver =
      m_pNormalWidget->GetOwnerApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(m_pNormalWidget.get(),
                                   m_pNormalWidget.get());
  m_pOldDelegate = m_pNormalWidget->GetDelegate();
  m_pNormalWidget->SetDelegate(this);
  m_pNormalWidget->LockUpdate();
  UpdateWidgetProperty();

  pFWLEdit->SetText(
      m_pNode->GetWidgetAcc()->GetValue(XFA_VALUEPICTURE_Display));
  m_pNormalWidget->UnlockUpdate();
  return CXFA_FFField::LoadWidget();
}

void CXFA_FFTextEdit::UpdateWidgetProperty() {
  CFWL_Edit* pWidget = static_cast<CFWL_Edit*>(m_pNormalWidget.get());
  if (!pWidget)
    return;

  uint32_t dwStyle = 0;
  uint32_t dwExtendedStyle =
      FWL_STYLEEXT_EDT_ShowScrollbarFocus | FWL_STYLEEXT_EDT_OuterScrollbar;
  dwExtendedStyle |= UpdateUIProperty();
  if (m_pNode->GetWidgetAcc()->IsMultiLine()) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_MultiLine | FWL_STYLEEXT_EDT_WantReturn;
    if (!m_pNode->GetWidgetAcc()->IsVerticalScrollPolicyOff()) {
      dwStyle |= FWL_WGTSTYLE_VScroll;
      dwExtendedStyle |= FWL_STYLEEXT_EDT_AutoVScroll;
    }
  } else if (!m_pNode->GetWidgetAcc()->IsHorizontalScrollPolicyOff()) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_AutoHScroll;
  }
  if (!m_pNode->IsOpenAccess() || !GetDoc()->GetXFADoc()->IsInteractive()) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_ReadOnly;
    dwExtendedStyle |= FWL_STYLEEXT_EDT_MultiLine;
  }

  XFA_Element eType;
  int32_t iMaxChars;
  std::tie(eType, iMaxChars) = m_pNode->GetWidgetAcc()->GetMaxChars();
  if (eType == XFA_Element::ExData)
    iMaxChars = 0;

  Optional<int32_t> numCells = m_pNode->GetWidgetAcc()->GetNumberOfCells();
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
  m_pNormalWidget->ModifyStyles(dwStyle, 0xFFFFFFFF);
  m_pNormalWidget->ModifyStylesEx(dwExtendedStyle, 0xFFFFFFFF);
}

bool CXFA_FFTextEdit::OnLButtonDown(uint32_t dwFlags, const CFX_PointF& point) {
  if (!PtInActiveRect(point))
    return false;
  if (!IsFocused()) {
    m_dwStatus |= XFA_WidgetStatus_Focused;
    UpdateFWLData();
    AddInvalidateRect();
  }

  SetButtonDown(true);
  CFWL_MessageMouse ms(nullptr, m_pNormalWidget.get());
  ms.m_dwCmd = FWL_MouseCommand::LeftButtonDown;
  ms.m_dwFlags = dwFlags;
  ms.m_pos = FWLToClient(point);
  TranslateFWLMessage(&ms);
  return true;
}

bool CXFA_FFTextEdit::OnRButtonDown(uint32_t dwFlags, const CFX_PointF& point) {
  if (!m_pNode->IsOpenAccess())
    return false;
  if (!PtInActiveRect(point))
    return false;
  if (!IsFocused()) {
    m_dwStatus |= XFA_WidgetStatus_Focused;
    UpdateFWLData();
    AddInvalidateRect();
  }

  SetButtonDown(true);
  CFWL_MessageMouse ms(nullptr, nullptr);
  ms.m_dwCmd = FWL_MouseCommand::RightButtonDown;
  ms.m_dwFlags = dwFlags;
  ms.m_pos = FWLToClient(point);
  TranslateFWLMessage(&ms);
  return true;
}

bool CXFA_FFTextEdit::OnRButtonUp(uint32_t dwFlags, const CFX_PointF& point) {
  if (!CXFA_FFField::OnRButtonUp(dwFlags, point))
    return false;

  GetDoc()->GetDocEnvironment()->PopupMenu(this, point);
  return true;
}

bool CXFA_FFTextEdit::OnSetFocus(CXFA_FFWidget* pOldWidget) {
  m_dwStatus &= ~XFA_WidgetStatus_TextEditValueChanged;
  if (!IsFocused()) {
    m_dwStatus |= XFA_WidgetStatus_Focused;
    UpdateFWLData();
    AddInvalidateRect();
  }
  CXFA_FFWidget::OnSetFocus(pOldWidget);
  CFWL_MessageSetFocus ms(nullptr, m_pNormalWidget.get());
  TranslateFWLMessage(&ms);
  return true;
}

bool CXFA_FFTextEdit::OnKillFocus(CXFA_FFWidget* pNewWidget) {
  CFWL_MessageKillFocus ms(nullptr, m_pNormalWidget.get());
  TranslateFWLMessage(&ms);
  m_dwStatus &= ~XFA_WidgetStatus_Focused;

  SetEditScrollOffset();
  ProcessCommittedData();
  UpdateFWLData();
  AddInvalidateRect();
  CXFA_FFWidget::OnKillFocus(pNewWidget);

  m_dwStatus &= ~XFA_WidgetStatus_TextEditValueChanged;
  return true;
}

bool CXFA_FFTextEdit::CommitData() {
  WideString wsText = static_cast<CFWL_Edit*>(m_pNormalWidget.get())->GetText();
  if (m_pNode->GetWidgetAcc()->SetValue(XFA_VALUEPICTURE_Edit, wsText)) {
    m_pNode->GetWidgetAcc()->UpdateUIDisplay(GetDoc()->GetDocView(), this);
    return true;
  }
  ValidateNumberField(wsText);
  return false;
}

void CXFA_FFTextEdit::ValidateNumberField(const WideString& wsText) {
  CXFA_WidgetAcc* pAcc = GetNode()->GetWidgetAcc();
  if (!pAcc || pAcc->GetUIType() != XFA_Element::NumericEdit)
    return;

  IXFA_AppProvider* pAppProvider = GetApp()->GetAppProvider();
  if (!pAppProvider)
    return;

  WideString wsSomField = pAcc->GetNode()->GetSOMExpression();
  pAppProvider->MsgBox(WideString::Format(L"%ls can not contain %ls",
                                          wsText.c_str(), wsSomField.c_str()),
                       pAppProvider->GetAppTitle(), XFA_MBICON_Error,
                       XFA_MB_OK);
}

bool CXFA_FFTextEdit::IsDataChanged() {
  return (m_dwStatus & XFA_WidgetStatus_TextEditValueChanged) != 0;
}

uint32_t CXFA_FFTextEdit::GetAlignment() {
  CXFA_Para* para = m_pNode->GetParaIfExists();
  if (!para)
    return 0;

  uint32_t dwExtendedStyle = 0;
  switch (para->GetHorizontalAlign()) {
    case XFA_AttributeEnum::Center:
      dwExtendedStyle |= FWL_STYLEEXT_EDT_HCenter;
      break;
    case XFA_AttributeEnum::Justify:
      dwExtendedStyle |= FWL_STYLEEXT_EDT_Justified;
      break;
    case XFA_AttributeEnum::JustifyAll:
    case XFA_AttributeEnum::Radix:
      break;
    case XFA_AttributeEnum::Right:
      dwExtendedStyle |= FWL_STYLEEXT_EDT_HFar;
      break;
    default:
      dwExtendedStyle |= FWL_STYLEEXT_EDT_HNear;
      break;
  }

  switch (para->GetVerticalAlign()) {
    case XFA_AttributeEnum::Middle:
      dwExtendedStyle |= FWL_STYLEEXT_EDT_VCenter;
      break;
    case XFA_AttributeEnum::Bottom:
      dwExtendedStyle |= FWL_STYLEEXT_EDT_VFar;
      break;
    default:
      dwExtendedStyle |= FWL_STYLEEXT_EDT_VNear;
      break;
  }
  return dwExtendedStyle;
}

bool CXFA_FFTextEdit::UpdateFWLData() {
  if (!m_pNormalWidget)
    return false;

  CFWL_Edit* pEdit = static_cast<CFWL_Edit*>(m_pNormalWidget.get());
  XFA_VALUEPICTURE eType = XFA_VALUEPICTURE_Display;
  if (IsFocused())
    eType = XFA_VALUEPICTURE_Edit;

  bool bUpdate = false;
  if (m_pNode->GetWidgetAcc()->GetUIType() == XFA_Element::TextEdit &&
      !m_pNode->GetWidgetAcc()->GetNumberOfCells()) {
    XFA_Element elementType;
    int32_t iMaxChars;
    std::tie(elementType, iMaxChars) = m_pNode->GetWidgetAcc()->GetMaxChars();
    if (elementType == XFA_Element::ExData)
      iMaxChars = eType == XFA_VALUEPICTURE_Edit ? iMaxChars : 0;
    if (pEdit->GetLimit() != iMaxChars) {
      pEdit->SetLimit(iMaxChars);
      bUpdate = true;
    }
  } else if (m_pNode->GetWidgetAcc()->GetUIType() == XFA_Element::Barcode) {
    int32_t nDataLen = 0;
    if (eType == XFA_VALUEPICTURE_Edit)
      nDataLen = m_pNode->GetBarcodeAttribute_DataLength().value_or(0);

    pEdit->SetLimit(nDataLen);
    bUpdate = true;
  }

  WideString wsText = m_pNode->GetWidgetAcc()->GetValue(eType);
  WideString wsOldText = pEdit->GetText();
  if (wsText != wsOldText || (eType == XFA_VALUEPICTURE_Edit && bUpdate)) {
    pEdit->SetText(wsText);
    bUpdate = true;
  }
  if (bUpdate)
    m_pNormalWidget->Update();

  return true;
}

void CXFA_FFTextEdit::OnTextChanged(CFWL_Widget* pWidget,
                                    const WideString& wsChanged,
                                    const WideString& wsPrevText) {
  m_dwStatus |= XFA_WidgetStatus_TextEditValueChanged;
  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_Change;
  eParam.m_wsChange = wsChanged;
  eParam.m_pTarget = m_pNode->GetWidgetAcc();
  eParam.m_wsPrevText = wsPrevText;
  CFWL_Edit* pEdit = static_cast<CFWL_Edit*>(m_pNormalWidget.get());
  if (m_pNode->GetWidgetAcc()->GetUIType() == XFA_Element::DateTimeEdit) {
    CFWL_DateTimePicker* pDateTime = (CFWL_DateTimePicker*)pEdit;
    eParam.m_wsNewText = pDateTime->GetEditText();
    if (pDateTime->HasSelection()) {
      size_t count;
      std::tie(eParam.m_iSelStart, count) = pDateTime->GetSelection();
      eParam.m_iSelEnd = eParam.m_iSelStart + count;
    }
  } else {
    eParam.m_wsNewText = pEdit->GetText();
    if (pEdit->HasSelection())
      std::tie(eParam.m_iSelStart, eParam.m_iSelEnd) = pEdit->GetSelection();
  }
  m_pNode->ProcessEvent(GetDocView(), XFA_AttributeEnum::Change, &eParam);
}

void CXFA_FFTextEdit::OnTextFull(CFWL_Widget* pWidget) {
  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_Full;
  eParam.m_pTarget = m_pNode->GetWidgetAcc();
  m_pNode->ProcessEvent(GetDocView(), XFA_AttributeEnum::Full, &eParam);
}

bool CXFA_FFTextEdit::CheckWord(const ByteStringView& sWord) {
  return sWord.IsEmpty() ||
         m_pNode->GetWidgetAcc()->GetUIType() != XFA_Element::TextEdit;
}

void CXFA_FFTextEdit::OnProcessMessage(CFWL_Message* pMessage) {
  m_pOldDelegate->OnProcessMessage(pMessage);
}

void CXFA_FFTextEdit::OnProcessEvent(CFWL_Event* pEvent) {
  CXFA_FFField::OnProcessEvent(pEvent);
  switch (pEvent->GetType()) {
    case CFWL_Event::Type::TextChanged: {
      CFWL_EventTextChanged* event =
          static_cast<CFWL_EventTextChanged*>(pEvent);
      WideString wsChange;
      OnTextChanged(m_pNormalWidget.get(), wsChange, event->wsPrevText);
      break;
    }
    case CFWL_Event::Type::TextFull: {
      OnTextFull(m_pNormalWidget.get());
      break;
    }
    case CFWL_Event::Type::CheckWord: {
      WideString wstr(L"FWL_EVENT_DTP_SelectChanged");
      CFWL_EventCheckWord* event = static_cast<CFWL_EventCheckWord*>(pEvent);
      event->bCheckWord = CheckWord(event->bsWord.AsStringView());
      break;
    }
    default:
      break;
  }
  m_pOldDelegate->OnProcessEvent(pEvent);
}

void CXFA_FFTextEdit::OnDrawWidget(CXFA_Graphics* pGraphics,
                                   const CFX_Matrix& matrix) {
  m_pOldDelegate->OnDrawWidget(pGraphics, matrix);
}

bool CXFA_FFTextEdit::CanUndo() {
  return ToEdit(m_pNormalWidget.get())->CanUndo();
}

bool CXFA_FFTextEdit::CanRedo() {
  return ToEdit(m_pNormalWidget.get())->CanRedo();
}

bool CXFA_FFTextEdit::Undo() {
  return ToEdit(m_pNormalWidget.get())->Undo();
}

bool CXFA_FFTextEdit::Redo() {
  return ToEdit(m_pNormalWidget.get())->Redo();
}

bool CXFA_FFTextEdit::CanCopy() {
  return ToEdit(m_pNormalWidget.get())->HasSelection();
}

bool CXFA_FFTextEdit::CanCut() {
  if (ToEdit(m_pNormalWidget.get())->GetStylesEx() & FWL_STYLEEXT_EDT_ReadOnly)
    return false;
  return ToEdit(m_pNormalWidget.get())->HasSelection();
}

bool CXFA_FFTextEdit::CanPaste() {
  return !(ToEdit(m_pNormalWidget.get())->GetStylesEx() &
           FWL_STYLEEXT_EDT_ReadOnly);
}

bool CXFA_FFTextEdit::CanSelectAll() {
  return ToEdit(m_pNormalWidget.get())->GetTextLength() > 0;
}

Optional<WideString> CXFA_FFTextEdit::Copy() {
  return ToEdit(m_pNormalWidget.get())->Copy();
}

Optional<WideString> CXFA_FFTextEdit::Cut() {
  return ToEdit(m_pNormalWidget.get())->Cut();
}

bool CXFA_FFTextEdit::Paste(const WideString& wsPaste) {
  return ToEdit(m_pNormalWidget.get())->Paste(wsPaste);
}

void CXFA_FFTextEdit::SelectAll() {
  ToEdit(m_pNormalWidget.get())->SelectAll();
}

void CXFA_FFTextEdit::Delete() {
  ToEdit(m_pNormalWidget.get())->ClearText();
}

void CXFA_FFTextEdit::DeSelect() {
  ToEdit(m_pNormalWidget.get())->ClearSelection();
}

FormFieldType CXFA_FFTextEdit::GetFormFieldType() {
  return FormFieldType::kXFA_TextField;
}
