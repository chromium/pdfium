// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffnumericedit.h"

#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/cfwl_edit.h"
#include "xfa/fwl/cfwl_eventvalidate.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/parser/cxfa_localevalue.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/xfa_utils.h"

CXFA_FFNumericEdit::CXFA_FFNumericEdit(CXFA_Node* pNode)
    : CXFA_FFTextEdit(pNode) {}

CXFA_FFNumericEdit::~CXFA_FFNumericEdit() = default;

bool CXFA_FFNumericEdit::LoadWidget() {
  ASSERT(!IsLoaded());

  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retain_layout(m_pLayoutItem.Get());

  auto pNewEdit = pdfium::MakeUnique<CFWL_Edit>(
      GetFWLApp(), pdfium::MakeUnique<CFWL_WidgetProperties>(), nullptr);
  CFWL_Edit* pWidget = pNewEdit.get();
  SetNormalWidget(std::move(pNewEdit));
  pWidget->SetAdapterIface(this);

  CFWL_NoteDriver* pNoteDriver = pWidget->GetOwnerApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(pWidget, pWidget);
  m_pOldDelegate = pWidget->GetDelegate();
  pWidget->SetDelegate(this);

  {
    CFWL_Widget::ScopedUpdateLock update_lock(pWidget);
    pWidget->SetText(m_pNode->GetValue(XFA_VALUEPICTURE_Display));
    UpdateWidgetProperty();
  }

  return CXFA_FFField::LoadWidget();
}

void CXFA_FFNumericEdit::UpdateWidgetProperty() {
  CFWL_Edit* pWidget = static_cast<CFWL_Edit*>(GetNormalWidget());
  if (!pWidget)
    return;

  uint32_t dwExtendedStyle =
      FWL_STYLEEXT_EDT_ShowScrollbarFocus | FWL_STYLEEXT_EDT_OuterScrollbar |
      FWL_STYLEEXT_EDT_Validate | FWL_STYLEEXT_EDT_Number;
  dwExtendedStyle |= UpdateUIProperty();
  if (!m_pNode->IsHorizontalScrollPolicyOff())
    dwExtendedStyle |= FWL_STYLEEXT_EDT_AutoHScroll;

  Optional<int32_t> numCells = m_pNode->GetNumberOfCells();
  if (numCells && *numCells > 0) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_CombText;
    pWidget->SetLimit(*numCells);
  }
  dwExtendedStyle |= GetAlignment();
  if (!m_pNode->IsOpenAccess() || !GetDoc()->GetXFADoc()->IsInteractive())
    dwExtendedStyle |= FWL_STYLEEXT_EDT_ReadOnly;

  GetNormalWidget()->ModifyStylesEx(dwExtendedStyle, 0xFFFFFFFF);
}

void CXFA_FFNumericEdit::OnProcessEvent(CFWL_Event* pEvent) {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retain_layout(m_pLayoutItem.Get());

  if (pEvent->GetType() == CFWL_Event::Type::Validate) {
    CFWL_EventValidate* event = static_cast<CFWL_EventValidate*>(pEvent);
    event->bValidate = OnValidate(GetNormalWidget(), event->wsInsert);
    return;
  }
  CXFA_FFTextEdit::OnProcessEvent(pEvent);
}

bool CXFA_FFNumericEdit::OnValidate(CFWL_Widget* pWidget, WideString& wsText) {
  // Prevents destruction of the CXFA_ContentLayoutItem that owns |this|.
  RetainPtr<CXFA_ContentLayoutItem> retainer(m_pLayoutItem.Get());

  WideString wsPattern = m_pNode->GetPictureContent(XFA_VALUEPICTURE_Edit);
  if (!wsPattern.IsEmpty())
    return true;

  WideString wsFormat;
  CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(m_pNode.Get());
  widgetValue.GetNumericFormat(wsFormat, m_pNode->GetLeadDigits(),
                               m_pNode->GetFracDigits());
  return widgetValue.ValidateNumericTemp(wsText, wsFormat,
                                         m_pNode->GetLocale());
}
