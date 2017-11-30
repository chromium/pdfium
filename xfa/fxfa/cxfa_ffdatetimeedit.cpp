// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffdatetimeedit.h"

#include <utility>

#include "xfa/fwl/cfwl_datetimepicker.h"
#include "xfa/fwl/cfwl_eventselectchanged.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/parser/cxfa_localevalue.h"
#include "xfa/fxfa/parser/xfa_utils.h"

CXFA_FFDateTimeEdit::CXFA_FFDateTimeEdit(CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFTextEdit(pDataAcc) {}

CXFA_FFDateTimeEdit::~CXFA_FFDateTimeEdit() {}

CFX_RectF CXFA_FFDateTimeEdit::GetBBox(uint32_t dwStatus, bool bDrawFocus) {
  if (bDrawFocus)
    return CFX_RectF();
  return CXFA_FFWidget::GetBBox(dwStatus);
}

bool CXFA_FFDateTimeEdit::PtInActiveRect(const CFX_PointF& point) {
  auto* pPicker = static_cast<CFWL_DateTimePicker*>(m_pNormalWidget.get());
  return pPicker && pPicker->GetBBox().Contains(point);
}

bool CXFA_FFDateTimeEdit::LoadWidget() {
  auto pNewPicker = pdfium::MakeUnique<CFWL_DateTimePicker>(GetFWLApp());
  CFWL_DateTimePicker* pWidget = pNewPicker.get();
  m_pNormalWidget = std::move(pNewPicker);
  m_pNormalWidget->SetLayoutItem(this);

  CFWL_NoteDriver* pNoteDriver =
      m_pNormalWidget->GetOwnerApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(m_pNormalWidget.get(),
                                   m_pNormalWidget.get());
  m_pOldDelegate = m_pNormalWidget->GetDelegate();
  m_pNormalWidget->SetDelegate(this);
  m_pNormalWidget->LockUpdate();

  WideString wsText = m_pDataAcc->GetValue(XFA_VALUEPICTURE_Display);
  pWidget->SetEditText(wsText);

  CXFA_ValueData valueData = m_pDataAcc->GetFormValueData();
  if (valueData.HasValidNode()) {
    switch (valueData.GetChildValueClassID()) {
      case XFA_Element::Date: {
        if (!wsText.IsEmpty()) {
          CXFA_LocaleValue lcValue = XFA_GetLocaleValue(m_pDataAcc.Get());
          CFX_DateTime date = lcValue.GetDate();
          if (date.IsSet())
            pWidget->SetCurSel(date.GetYear(), date.GetMonth(), date.GetDay());
        }
      } break;
      default:
        break;
    }
  }
  UpdateWidgetProperty();
  m_pNormalWidget->UnlockUpdate();
  return CXFA_FFField::LoadWidget();
}

void CXFA_FFDateTimeEdit::UpdateWidgetProperty() {
  CFWL_DateTimePicker* pWidget =
      static_cast<CFWL_DateTimePicker*>(m_pNormalWidget.get());
  if (!pWidget)
    return;

  uint32_t dwExtendedStyle = FWL_STYLEEXT_DTP_ShortDateFormat;
  dwExtendedStyle |= UpdateUIProperty();
  dwExtendedStyle |= GetAlignment();
  m_pNormalWidget->ModifyStylesEx(dwExtendedStyle, 0xFFFFFFFF);

  uint32_t dwEditStyles = 0;
  pdfium::Optional<int32_t> numCells = m_pDataAcc->GetNumberOfCells();
  if (numCells && *numCells > 0) {
    dwEditStyles |= FWL_STYLEEXT_EDT_CombText;
    pWidget->SetEditLimit(*numCells);
  }
  if (!m_pDataAcc->IsOpenAccess() ||
      !m_pDataAcc->GetDoc()->GetXFADoc()->IsInteractive()) {
    dwEditStyles |= FWL_STYLEEXT_EDT_ReadOnly;
  }
  if (!m_pDataAcc->IsHorizontalScrollPolicyOff())
    dwEditStyles |= FWL_STYLEEXT_EDT_AutoHScroll;

  pWidget->ModifyEditStylesEx(dwEditStyles, 0xFFFFFFFF);
}

uint32_t CXFA_FFDateTimeEdit::GetAlignment() {
  CXFA_ParaData paraData = m_pDataAcc->GetParaData();
  if (!paraData.HasValidNode())
    return 0;

  uint32_t dwExtendedStyle = 0;
  switch (paraData.GetHorizontalAlign()) {
    case XFA_AttributeEnum::Center:
      dwExtendedStyle |= FWL_STYLEEXT_DTP_EditHCenter;
      break;
    case XFA_AttributeEnum::Justify:
      dwExtendedStyle |= FWL_STYLEEXT_DTP_EditJustified;
      break;
    case XFA_AttributeEnum::JustifyAll:
    case XFA_AttributeEnum::Radix:
      break;
    case XFA_AttributeEnum::Right:
      dwExtendedStyle |= FWL_STYLEEXT_DTP_EditHFar;
      break;
    default:
      dwExtendedStyle |= FWL_STYLEEXT_DTP_EditHNear;
      break;
  }

  switch (paraData.GetVerticalAlign()) {
    case XFA_AttributeEnum::Middle:
      dwExtendedStyle |= FWL_STYLEEXT_DTP_EditVCenter;
      break;
    case XFA_AttributeEnum::Bottom:
      dwExtendedStyle |= FWL_STYLEEXT_DTP_EditVFar;
      break;
    default:
      dwExtendedStyle |= FWL_STYLEEXT_DTP_EditVNear;
      break;
  }
  return dwExtendedStyle;
}

bool CXFA_FFDateTimeEdit::CommitData() {
  auto* pPicker = static_cast<CFWL_DateTimePicker*>(m_pNormalWidget.get());
  if (!m_pDataAcc->SetValue(XFA_VALUEPICTURE_Edit, pPicker->GetEditText()))
    return false;

  m_pDataAcc->UpdateUIDisplay(this);
  return true;
}

bool CXFA_FFDateTimeEdit::UpdateFWLData() {
  if (!m_pNormalWidget)
    return false;

  XFA_VALUEPICTURE eType = XFA_VALUEPICTURE_Display;
  if (IsFocused())
    eType = XFA_VALUEPICTURE_Edit;

  WideString wsText = m_pDataAcc->GetValue(eType);
  auto* normalWidget = static_cast<CFWL_DateTimePicker*>(m_pNormalWidget.get());
  normalWidget->SetEditText(wsText);
  if (IsFocused() && !wsText.IsEmpty()) {
    CXFA_LocaleValue lcValue = XFA_GetLocaleValue(m_pDataAcc.Get());
    CFX_DateTime date = lcValue.GetDate();
    if (lcValue.IsValid()) {
      if (date.IsSet())
        normalWidget->SetCurSel(date.GetYear(), date.GetMonth(), date.GetDay());
    }
  }
  m_pNormalWidget->Update();
  return true;
}

bool CXFA_FFDateTimeEdit::IsDataChanged() {
  if (m_dwStatus & XFA_WidgetStatus_TextEditValueChanged)
    return true;

  WideString wsText =
      static_cast<CFWL_DateTimePicker*>(m_pNormalWidget.get())->GetEditText();
  return m_pDataAcc->GetValue(XFA_VALUEPICTURE_Edit) != wsText;
}

void CXFA_FFDateTimeEdit::OnSelectChanged(CFWL_Widget* pWidget,
                                          int32_t iYear,
                                          int32_t iMonth,
                                          int32_t iDay) {
  WideString wsPicture = m_pDataAcc->GetPictureContent(XFA_VALUEPICTURE_Edit);

  CXFA_LocaleValue date(XFA_VT_DATE, GetDoc()->GetXFADoc()->GetLocalMgr());
  date.SetDate(CFX_DateTime(iYear, iMonth, iDay, 0, 0, 0, 0));

  WideString wsDate;
  date.FormatPatterns(wsDate, wsPicture, m_pDataAcc->GetLocale(),
                      XFA_VALUEPICTURE_Edit);

  auto* pDateTime = static_cast<CFWL_DateTimePicker*>(m_pNormalWidget.get());
  pDateTime->SetEditText(wsDate);
  pDateTime->Update();
  GetDoc()->GetDocEnvironment()->SetFocusWidget(GetDoc(), nullptr);

  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_Change;
  eParam.m_pTarget = m_pDataAcc.Get();
  eParam.m_wsNewText = m_pDataAcc->GetValue(XFA_VALUEPICTURE_Raw);
  m_pDataAcc->ProcessEvent(XFA_AttributeEnum::Change, &eParam);
}

void CXFA_FFDateTimeEdit::OnProcessEvent(CFWL_Event* pEvent) {
  if (pEvent->GetType() == CFWL_Event::Type::SelectChanged) {
    auto* event = static_cast<CFWL_EventSelectChanged*>(pEvent);
    OnSelectChanged(m_pNormalWidget.get(), event->iYear, event->iMonth,
                    event->iDay);
    return;
  }
  CXFA_FFTextEdit::OnProcessEvent(pEvent);
}
