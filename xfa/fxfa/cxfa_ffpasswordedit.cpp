// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffpasswordedit.h"

#include "core/fxcrt/check.h"
#include "xfa/fwl/cfwl_edit.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_passwordedit.h"

CXFA_FFPasswordEdit::CXFA_FFPasswordEdit(CXFA_Node* pNode,
                                         CXFA_PasswordEdit* password_node)
    : CXFA_FFTextEdit(pNode), password_node_(password_node) {}

CXFA_FFPasswordEdit::~CXFA_FFPasswordEdit() = default;

void CXFA_FFPasswordEdit::Trace(cppgc::Visitor* visitor) const {
  CXFA_FFTextEdit::Trace(visitor);
  visitor->Trace(password_node_);
}

bool CXFA_FFPasswordEdit::LoadWidget() {
  DCHECK(!IsLoaded());

  CFWL_Edit* pWidget = cppgc::MakeGarbageCollected<CFWL_Edit>(
      GetFWLApp()->GetHeap()->GetAllocationHandle(), GetFWLApp(),
      CFWL_Widget::Properties(), nullptr);
  SetNormalWidget(pWidget);
  pWidget->SetAdapterIface(this);

  CFWL_NoteDriver* pNoteDriver = pWidget->GetFWLApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(pWidget, pWidget);
  m_pOldDelegate = pWidget->GetDelegate();
  pWidget->SetDelegate(this);

  {
    CFWL_Widget::ScopedUpdateLock update_lock(pWidget);
    pWidget->SetText(m_pNode->GetValue(XFA_ValuePicture::kDisplay));
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
  GetNormalWidget()->ModifyStyleExts(dwExtendedStyle, 0xFFFFFFFF);
}
