// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_comboboxproxy.h"

#include <memory>
#include <utility>

#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_combobox.h"
#include "xfa/fwl/cfwl_messagekillfocus.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_notedriver.h"

CFWL_ComboBoxProxy::CFWL_ComboBoxProxy(
    CFWL_ComboBox* pComboBox,
    const CFWL_App* app,
    std::unique_ptr<CFWL_WidgetProperties> properties,
    CFWL_Widget* pOuter)
    : CFWL_FormProxy(app, std::move(properties), pOuter),
      m_bLButtonDown(false),
      m_bLButtonUpSelf(false),
      m_pComboBox(pComboBox) {}

CFWL_ComboBoxProxy::~CFWL_ComboBoxProxy() {}

void CFWL_ComboBoxProxy::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return;

  switch (pMessage->GetType()) {
    case CFWL_Message::Type::Mouse: {
      CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
      switch (pMsg->m_dwCmd) {
        case FWL_MouseCommand::LeftButtonDown:
          OnLButtonDown(pMsg);
          break;
        case FWL_MouseCommand::LeftButtonUp:
          OnLButtonUp(pMsg);
          break;
        default:
          break;
      }
      break;
    }
    case CFWL_Message::Type::KillFocus:
      OnFocusChanged(pMessage, false);
      break;
    case CFWL_Message::Type::SetFocus:
      OnFocusChanged(pMessage, true);
      break;
    default:
      break;
  }
  CFWL_Widget::OnProcessMessage(pMessage);
}

void CFWL_ComboBoxProxy::OnDrawWidget(CXFA_Graphics* pGraphics,
                                      const CFX_Matrix& matrix) {
  m_pComboBox->DrawStretchHandler(pGraphics, &matrix);
}

void CFWL_ComboBoxProxy::OnLButtonDown(CFWL_Message* pMessage) {
  const CFWL_App* pApp = GetOwnerApp();
  if (!pApp)
    return;

  CFWL_NoteDriver* pDriver =
      static_cast<CFWL_NoteDriver*>(pApp->GetNoteDriver());
  CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
  if (CFX_RectF(0, 0, GetWidgetRect().Size()).Contains(pMsg->m_pos)) {
    m_bLButtonDown = true;
    pDriver->SetGrab(this, true);
  } else {
    m_bLButtonDown = false;
    pDriver->SetGrab(this, false);
    m_pComboBox->ShowDropList(false);
  }
}

void CFWL_ComboBoxProxy::OnLButtonUp(CFWL_Message* pMessage) {
  m_bLButtonDown = false;
  const CFWL_App* pApp = GetOwnerApp();
  if (!pApp)
    return;

  CFWL_NoteDriver* pDriver =
      static_cast<CFWL_NoteDriver*>(pApp->GetNoteDriver());
  pDriver->SetGrab(this, false);
  if (!m_bLButtonUpSelf) {
    m_bLButtonUpSelf = true;
    return;
  }

  CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
  if (!CFX_RectF(0, 0, GetWidgetRect().Size()).Contains(pMsg->m_pos) &&
      m_pComboBox->IsDropListVisible()) {
    m_pComboBox->ShowDropList(false);
  }
}

void CFWL_ComboBoxProxy::OnFocusChanged(CFWL_Message* pMessage, bool bSet) {
  if (bSet)
    return;

  CFWL_MessageKillFocus* pMsg = static_cast<CFWL_MessageKillFocus*>(pMessage);
  if (!pMsg->m_pSetFocus)
    m_pComboBox->ShowDropList(false);
}
