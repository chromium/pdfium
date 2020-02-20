// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffpasswordedit.h"

#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/cfwl_edit.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_passwordedit.h"

CXFA_FFPasswordEdit::CXFA_FFPasswordEdit(CXFA_Node* pNode,
                                         CXFA_PasswordEdit* password_node)
    : CXFA_FFTextEdit(pNode), password_node_(password_node) {}

CXFA_FFPasswordEdit::~CXFA_FFPasswordEdit() = default;

bool CXFA_FFPasswordEdit::LoadWidget() {
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

void CXFA_FFPasswordEdit::UpdateWidgetProperty() {
  CFWL_Edit* pWidget = static_cast<CFWL_Edit*>(GetNormalWidget());
  if (!pWidget)
    return;

  uint32_t dwExtendedStyle = FWL_STYLEEXT_EDT_ShowScrollbarFocus |
                             FWL_STYLEEXT_EDT_OuterScrollbar |
                             FWL_STYLEEXT_EDT_Password;
  dwExtendedStyle |= UpdateUIProperty();

  WideString password = password_node_->GetPasswordChar();
  if (!password.IsEmpty())
    pWidget->SetAliasChar(password[0]);
  if (!m_pNode->IsHorizontalScrollPolicyOff())
    dwExtendedStyle |= FWL_STYLEEXT_EDT_AutoHScroll;
  if (!m_pNode->IsOpenAccess() || !GetDoc()->GetXFADoc()->IsInteractive())
    dwExtendedStyle |= FWL_STYLEEXT_EDT_ReadOnly;

  dwExtendedStyle |= GetAlignment();
  GetNormalWidget()->ModifyStylesEx(dwExtendedStyle, 0xFFFFFFFF);
}
