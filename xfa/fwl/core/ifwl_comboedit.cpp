// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/ifwl_comboedit.h"

#include "xfa/fde/cfde_txtedtengine.h"
#include "xfa/fwl/core/ifwl_combobox.h"

IFWL_ComboEdit::IFWL_ComboEdit(const IFWL_App* app,
                               const CFWL_WidgetImpProperties& properties,
                               IFWL_Widget* pOuter)
    : IFWL_Edit(app, properties, pOuter) {
  m_pOuter = static_cast<IFWL_ComboBox*>(pOuter);
}

void IFWL_ComboEdit::ClearSelected() {
  ClearSelections();
  Repaint(&m_rtClient);
}

void IFWL_ComboEdit::SetSelected() {
  FlagFocus(TRUE);
  EndCaret();
  AddSelRange(0);
}

void IFWL_ComboEdit::EndCaret() {
  m_pEdtEngine->MoveCaretPos(MC_End);
}

void IFWL_ComboEdit::FlagFocus(FX_BOOL bSet) {
  if (bSet) {
    m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;
  } else {
    m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Focused;
    ShowCaret(FALSE);
  }
}

void IFWL_ComboEdit::SetComboBoxFocus(FX_BOOL bSet) {
  m_pOuter->SetFocus(bSet);
}

CFWL_ComboEditImpDelegate::CFWL_ComboEditImpDelegate(IFWL_ComboEdit* pOwner)
    : CFWL_EditImpDelegate(pOwner), m_pOwner(pOwner) {}

void CFWL_ComboEditImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return;

  FX_BOOL backDefault = TRUE;
  switch (pMessage->GetClassID()) {
    case CFWL_MessageType::SetFocus: {
      m_pOwner->m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;
      backDefault = FALSE;
      break;
    }
    case CFWL_MessageType::KillFocus: {
      m_pOwner->m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Focused;
      backDefault = FALSE;
      break;
    }
    case CFWL_MessageType::Mouse: {
      CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
      if ((pMsg->m_dwCmd == FWL_MouseCommand::LeftButtonDown) &&
          ((m_pOwner->m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) == 0)) {
        m_pOwner->SetSelected();
        m_pOwner->SetComboBoxFocus(TRUE);
      }
      break;
    }
    default:
      break;
  }
  if (backDefault)
    CFWL_EditImpDelegate::OnProcessMessage(pMessage);
}
