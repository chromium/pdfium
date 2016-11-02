// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/ifwl_comboboxproxy.h"

#include "xfa/fwl/core/fwl_noteimp.h"
#include "xfa/fwl/core/ifwl_app.h"
#include "xfa/fwl/core/ifwl_combobox.h"

IFWL_ComboBoxProxy::IFWL_ComboBoxProxy(
    IFWL_ComboBox* pComboBox,
    const IFWL_App* app,
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter)
    : IFWL_FormProxy(app, properties, pOuter),
      m_bLButtonDown(FALSE),
      m_bLButtonUpSelf(FALSE),
      m_pComboBox(pComboBox) {}

IFWL_ComboBoxProxy::~IFWL_ComboBoxProxy() {}

void IFWL_ComboBoxProxy::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return;

  switch (pMessage->GetClassID()) {
    case CFWL_MessageType::Mouse: {
      CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
      switch (pMsg->m_dwCmd) {
        case FWL_MouseCommand::LeftButtonDown:
          OnLButtonDown(pMsg);
          break;
        case FWL_MouseCommand::LeftButtonUp:
          OnLButtonUp(pMsg);
          break;
        case FWL_MouseCommand::Move:
          break;
        default:
          break;
      }
      break;
    }
    case CFWL_MessageType::Deactivate:
      OnDeactive(static_cast<CFWL_MsgDeactivate*>(pMessage));
      break;
    case CFWL_MessageType::KillFocus:
      OnFocusChanged(static_cast<CFWL_MsgKillFocus*>(pMessage), FALSE);
      break;
    case CFWL_MessageType::SetFocus:
      OnFocusChanged(static_cast<CFWL_MsgKillFocus*>(pMessage), TRUE);
      break;
    default:
      break;
  }
  IFWL_Widget::OnProcessMessage(pMessage);
}

void IFWL_ComboBoxProxy::OnDrawWidget(CFX_Graphics* pGraphics,
                                      const CFX_Matrix* pMatrix) {
  m_pComboBox->DrawStretchHandler(pGraphics, pMatrix);
}

void IFWL_ComboBoxProxy::OnLButtonDown(CFWL_MsgMouse* pMsg) {
  const IFWL_App* pApp = GetOwnerApp();
  if (!pApp)
    return;

  CFWL_NoteDriver* pDriver =
      static_cast<CFWL_NoteDriver*>(pApp->GetNoteDriver());
  CFX_RectF rtWidget;
  GetWidgetRect(rtWidget);
  rtWidget.left = rtWidget.top = 0;
  if (rtWidget.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_bLButtonDown = TRUE;
    pDriver->SetGrab(this, TRUE);
  } else {
    m_bLButtonDown = FALSE;
    pDriver->SetGrab(this, FALSE);
    m_pComboBox->ShowDropList(FALSE);
  }
}

void IFWL_ComboBoxProxy::OnLButtonUp(CFWL_MsgMouse* pMsg) {
  m_bLButtonDown = FALSE;
  const IFWL_App* pApp = GetOwnerApp();
  if (!pApp)
    return;

  CFWL_NoteDriver* pDriver =
      static_cast<CFWL_NoteDriver*>(pApp->GetNoteDriver());
  pDriver->SetGrab(this, FALSE);
  if (m_bLButtonUpSelf) {
    CFX_RectF rect;
    GetWidgetRect(rect);
    rect.left = rect.top = 0;
    if (!rect.Contains(pMsg->m_fx, pMsg->m_fy) &&
        m_pComboBox->IsDropListShowed()) {
      m_pComboBox->ShowDropList(FALSE);
    }
  } else {
    m_bLButtonUpSelf = TRUE;
  }
}

void IFWL_ComboBoxProxy::OnDeactive(CFWL_MsgDeactivate* pMsg) {
  m_pComboBox->ShowDropList(FALSE);
}

void IFWL_ComboBoxProxy::OnFocusChanged(CFWL_MsgKillFocus* pMsg, FX_BOOL bSet) {
  if (bSet)
    return;

  if (!pMsg->m_pSetFocus)
    m_pComboBox->ShowDropList(FALSE);
}
