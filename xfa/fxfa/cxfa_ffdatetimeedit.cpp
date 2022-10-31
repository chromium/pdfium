// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffdatetimeedit.h"

#include "core/fxcrt/cfx_datetime.h"
#include "third_party/base/check.h"
#include "xfa/fwl/cfwl_datetimepicker.h"
#include "xfa/fwl/cfwl_eventselectchanged.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/parser/cxfa_localevalue.h"
#include "xfa/fxfa/parser/cxfa_para.h"
#include "xfa/fxfa/parser/cxfa_value.h"
#include "xfa/fxfa/parser/xfa_utils.h"

CXFA_FFDateTimeEdit::CXFA_FFDateTimeEdit(CXFA_Node* pNode)
    : CXFA_FFTextEdit(pNode) {}

CXFA_FFDateTimeEdit::~CXFA_FFDateTimeEdit() = default;

CFWL_DateTimePicker* CXFA_FFDateTimeEdit::GetPickerWidget() {
  return static_cast<CFWL_DateTimePicker*>(GetNormalWidget());
}

CFX_RectF CXFA_FFDateTimeEdit::GetBBox(FocusOption focus) {
  if (focus == kDrawFocus)
    return CFX_RectF();
  return CXFA_FFWidget::GetBBox(kDoNotDrawFocus);
}

bool CXFA_FFDateTimeEdit::PtInActiveRect(const CFX_PointF& point) {
  CFWL_DateTimePicker* pPicker = GetPickerWidget();
  return pPicker && pPicker->GetBBox().Contains(point);
}

bool CXFA_FFDateTimeEdit::LoadWidget() {
  DCHECK(!IsLoaded());

  CFWL_DateTimePicker* pWidget =
      cppgc::MakeGarbageCollected<CFWL_DateTimePicker>(
          GetFWLApp()->GetHeap()->GetAllocationHandle(), GetFWLApp());
  SetNormalWidget(pWidget);
  pWidget->SetAdapterIface(this);

  CFWL_NoteDriver* pNoteDriver = pWidget->GetFWLApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(pWidget, pWidget);
  m_pOldDelegate = pWidget->GetDelegate();
  pWidget->SetDelegate(this);

  {
    CFWL_Widget::ScopedUpdateLock update_lock(pWidget);
    WideString wsText = m_pNode->GetValue(XFA_ValuePicture::kDisplay);
    pWidget->SetEditText(wsText);

    CXFA_Value* value = m_pNode->GetFormValueIfExists();
    if (value) {
      switch (value->GetChildValueClassID()) {
        case XFA_Element::Date: {
          if (!wsText.IsEmpty()) {
            CXFA_LocaleValue lcValue = XFA_GetLocaleValue(m_pNode.Get());
            CFX_DateTime date = lcValue.GetDate();
            if (date.IsSet())
              pWidget->SetCurSel(date.GetYear(), date.GetMonth(),
                                 date.GetDay());
          }
        } break;
        default:
          break;
      }
    }
    UpdateWidgetProperty();
  }

  return CXFA_FFField::LoadWidget();
}

void CXFA_FFDateTimeEdit::UpdateWidgetProperty() {
  CFWL_DateTimePicker* pPicker = GetPickerWidget();
  if (!pPicker)
    return;

  uint32_t dwExtendedStyle = FWL_STYLEEXT_DTP_ShortDateFormat;
  dwExtendedStyle |= UpdateUIProperty();
  dwExtendedStyle |= GetAlignment();
  GetNormalWidget()->ModifyStyleExts(dwExtendedStyle, 0xFFFFFFFF);

  uint32_t dwEditStyles = 0;
  absl::optional<int32_t> numCells = m_pNode->GetNumberOfCells();
  if (numCells.has_value() && numCells.value() > 0) {
    dwEditStyles |= FWL_STYLEEXT_EDT_CombText;
    pPicker->SetEditLimit(numCells.value());
  }
  if (!m_pNode->IsOpenAccess() || !GetDoc()->GetXFADoc()->IsInteractive())
    dwEditStyles |= FWL_STYLEEXT_EDT_ReadOnly;
  if (!m_pNode->IsHorizontalScrollPolicyOff())
    dwEditStyles |= FWL_STYLEEXT_EDT_AutoHScroll;

  pPicker->ModifyEditStyleExts(dwEditStyles, 0xFFFFFFFF);
}

uint32_t CXFA_FFDateTimeEdit::GetAlignment() {
  CXFA_Para* para = m_pNode->GetParaIfExists();
  if (!para)
    return 0;

  uint32_t dwExtendedStyle = 0;
  switch (para->GetHorizontalAlign()) {
    case XFA_AttributeValue::Center:
      dwExtendedStyle |= FWL_STYLEEXT_DTP_EditHCenter;
      break;
    case XFA_AttributeValue::Justify:
      dwExtendedStyle |= FWL_STYLEEXT_DTP_EditJustified;
      break;
    case XFA_AttributeValue::JustifyAll:
    case XFA_AttributeValue::Radix:
      break;
    case XFA_AttributeValue::Right:
      dwExtendedStyle |= FWL_STYLEEXT_DTP_EditHFar;
      break;
    default:
      dwExtendedStyle |= FWL_STYLEEXT_DTP_EditHNear;
      break;
  }

  switch (para->GetVerticalAlign()) {
    case XFA_AttributeValue::Middle:
      dwExtendedStyle |= FWL_STYLEEXT_DTP_EditVCenter;
      break;
    case XFA_AttributeValue::Bottom:
      dwExtendedStyle |= FWL_STYLEEXT_DTP_EditVFar;
      break;
    default:
      dwExtendedStyle |= FWL_STYLEEXT_DTP_EditVNear;
      break;
  }
  return dwExtendedStyle;
}

bool CXFA_FFDateTimeEdit::CommitData() {
  CFWL_DateTimePicker* pPicker = GetPickerWidget();
  if (!m_pNode->SetValue(XFA_ValuePicture::kEdit, pPicker->GetEditText()))
    return false;

  GetDoc()->GetDocView()->UpdateUIDisplay(m_pNode.Get(), this);
  return true;
}

bool CXFA_FFDateTimeEdit::UpdateFWLData() {
  if (!GetNormalWidget())
    return false;

  XFA_ValuePicture eType = XFA_ValuePicture::kDisplay;
  if (IsFocused())
    eType = XFA_ValuePicture::kEdit;

  WideString wsText = m_pNode->GetValue(eType);
  CFWL_DateTimePicker* pPicker = GetPickerWidget();
  pPicker->SetEditText(wsText);
  if (IsFocused() && !wsText.IsEmpty()) {
    CXFA_LocaleValue lcValue = XFA_GetLocaleValue(m_pNode.Get());
    CFX_DateTime date = lcValue.GetDate();
    if (lcValue.IsValid()) {
      if (date.IsSet())
        pPicker->SetCurSel(date.GetYear(), date.GetMonth(), date.GetDay());
    }
  }
  GetNormalWidget()->Update();
  return true;
}

bool CXFA_FFDateTimeEdit::IsDataChanged() {
  if (GetLayoutItem()->TestStatusBits(XFA_WidgetStatus::kTextEditValueChanged))
    return true;

  WideString wsText = GetPickerWidget()->GetEditText();
  return m_pNode->GetValue(XFA_ValuePicture::kEdit) != wsText;
}

void CXFA_FFDateTimeEdit::OnSelectChanged(CFWL_Widget* pWidget,
                                          int32_t iYear,
                                          int32_t iMonth,
                                          int32_t iDay) {
  WideString wsPicture = m_pNode->GetPictureContent(XFA_ValuePicture::kEdit);
  CXFA_LocaleValue date(CXFA_LocaleValue::ValueType::kDate,
                        GetDoc()->GetXFADoc()->GetLocaleMgr());
  date.SetDate(CFX_DateTime(iYear, iMonth, iDay, 0, 0, 0, 0));

  WideString wsDate;
  date.FormatPatterns(wsDate, wsPicture, m_pNode->GetLocale(),
                      XFA_ValuePicture::kEdit);

  CFWL_DateTimePicker* pPicker = GetPickerWidget();
  pPicker->SetEditText(wsDate);
  pPicker->Update();
  GetDoc()->SetFocusWidget(nullptr);

  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_Change;
  eParam.m_pTarget = m_pNode.Get();
  eParam.m_wsPrevText = m_pNode->GetValue(XFA_ValuePicture::kRaw);
  m_pNode->ProcessEvent(GetDocView(), XFA_AttributeValue::Change, &eParam);
}

void CXFA_FFDateTimeEdit::OnProcessEvent(CFWL_Event* pEvent) {
  if (pEvent->GetType() == CFWL_Event::Type::SelectChanged) {
    auto* event = static_cast<CFWL_EventSelectChanged*>(pEvent);
    OnSelectChanged(GetNormalWidget(), event->GetYear(), event->GetMonth(),
                    event->GetDay());
    return;
  }
  CXFA_FFTextEdit::OnProcessEvent(pEvent);
}

bool CXFA_FFDateTimeEdit::CanUndo() {
  return GetPickerWidget()->CanUndo();
}

bool CXFA_FFDateTimeEdit::CanRedo() {
  return GetPickerWidget()->CanRedo();
}

bool CXFA_FFDateTimeEdit::CanCopy() {
  return GetPickerWidget()->HasSelection();
}

bool CXFA_FFDateTimeEdit::CanCut() {
  if (GetPickerWidget()->GetStyleExts() & FWL_STYLEEXT_EDT_ReadOnly)
    return false;
  return GetPickerWidget()->HasSelection();
}

bool CXFA_FFDateTimeEdit::CanPaste() {
  return !(GetPickerWidget()->GetStyleExts() & FWL_STYLEEXT_EDT_ReadOnly);
}

bool CXFA_FFDateTimeEdit::CanSelectAll() {
  return GetPickerWidget()->GetEditTextLength() > 0;
}

absl::optional<WideString> CXFA_FFDateTimeEdit::Copy() {
  return GetPickerWidget()->Copy();
}

bool CXFA_FFDateTimeEdit::Undo() {
  return GetPickerWidget()->Undo();
}

bool CXFA_FFDateTimeEdit::Redo() {
  return GetPickerWidget()->Redo();
}

absl::optional<WideString> CXFA_FFDateTimeEdit::Cut() {
  return GetPickerWidget()->Cut();
}

bool CXFA_FFDateTimeEdit::Paste(const WideString& wsPaste) {
  return GetPickerWidget()->Paste(wsPaste);
}

void CXFA_FFDateTimeEdit::SelectAll() {
  GetPickerWidget()->SelectAll();
}

void CXFA_FFDateTimeEdit::Delete() {
  GetPickerWidget()->ClearText();
}

void CXFA_FFDateTimeEdit::DeSelect() {
  GetPickerWidget()->ClearSelection();
}

WideString CXFA_FFDateTimeEdit::GetText() {
  return GetPickerWidget()->GetEditText();
}
