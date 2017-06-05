// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/cxfa_ffnumericedit.h"

#include <utility>

#include "xfa/fwl/cfwl_edit.h"
#include "xfa/fwl/cfwl_eventvalidate.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fxfa/parser/cxfa_localevalue.h"

CXFA_FFNumericEdit::CXFA_FFNumericEdit(CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFTextEdit(pDataAcc) {}

CXFA_FFNumericEdit::~CXFA_FFNumericEdit() {}

bool CXFA_FFNumericEdit::LoadWidget() {
  auto pNewEdit = pdfium::MakeUnique<CFWL_Edit>(
      GetFWLApp(), pdfium::MakeUnique<CFWL_WidgetProperties>(), nullptr);
  CFWL_Edit* pWidget = pNewEdit.get();
  m_pNormalWidget = std::move(pNewEdit);
  m_pNormalWidget->SetLayoutItem(this);

  CFWL_NoteDriver* pNoteDriver =
      m_pNormalWidget->GetOwnerApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(m_pNormalWidget.get(),
                                   m_pNormalWidget.get());
  m_pOldDelegate = m_pNormalWidget->GetDelegate();
  m_pNormalWidget->SetDelegate(this);
  m_pNormalWidget->LockUpdate();

  CFX_WideString wsText;
  m_pDataAcc->GetValue(wsText, XFA_VALUEPICTURE_Display);
  pWidget->SetText(wsText);
  UpdateWidgetProperty();
  m_pNormalWidget->UnlockUpdate();
  return CXFA_FFField::LoadWidget();
}

void CXFA_FFNumericEdit::UpdateWidgetProperty() {
  CFWL_Edit* pWidget = static_cast<CFWL_Edit*>(m_pNormalWidget.get());
  if (!pWidget)
    return;

  uint32_t dwExtendedStyle =
      FWL_STYLEEXT_EDT_ShowScrollbarFocus | FWL_STYLEEXT_EDT_OuterScrollbar |
      FWL_STYLEEXT_EDT_Validate | FWL_STYLEEXT_EDT_Number |
      FWL_STYLEEXT_EDT_LastLineHeight;
  dwExtendedStyle |= UpdateUIProperty();
  if (m_pDataAcc->GetHorizontalScrollPolicy() != XFA_ATTRIBUTEENUM_Off)
    dwExtendedStyle |= FWL_STYLEEXT_EDT_AutoHScroll;

  int32_t iNumCells = m_pDataAcc->GetNumberOfCells();
  if (iNumCells > 0) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_CombText;
    pWidget->SetLimit(iNumCells);
  }
  dwExtendedStyle |= GetAlignment();
  if (m_pDataAcc->GetAccess() != XFA_ATTRIBUTEENUM_Open ||
      !m_pDataAcc->GetDoc()->GetXFADoc()->IsInteractive()) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_ReadOnly;
  }
  m_pNormalWidget->ModifyStylesEx(dwExtendedStyle, 0xFFFFFFFF);
}

void CXFA_FFNumericEdit::OnProcessEvent(CFWL_Event* pEvent) {
  if (pEvent->GetType() == CFWL_Event::Type::Validate) {
    CFWL_EventValidate* event = static_cast<CFWL_EventValidate*>(pEvent);
    event->bValidate = OnValidate(m_pNormalWidget.get(), event->wsInsert);
    return;
  }
  CXFA_FFTextEdit::OnProcessEvent(pEvent);
}

bool CXFA_FFNumericEdit::OnValidate(CFWL_Widget* pWidget,
                                    CFX_WideString& wsText) {
  CFX_WideString wsPattern;
  m_pDataAcc->GetPictureContent(wsPattern, XFA_VALUEPICTURE_Edit);
  if (!wsPattern.IsEmpty())
    return true;

  int32_t iLeads = 0;
  m_pDataAcc->GetLeadDigits(iLeads);

  int32_t iFracs = 0;
  m_pDataAcc->GetFracDigits(iFracs);

  CFX_WideString wsFormat;
  CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(m_pDataAcc.Get());
  widgetValue.GetNumericFormat(wsFormat, iLeads, iFracs);
  return widgetValue.ValidateNumericTemp(wsText, wsFormat,
                                         m_pDataAcc->GetLocal());
}
