// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_pushbutton.h"

#include "xfa/fde/cfde_textout.h"
#include "xfa/fwl/cfwl_event.h"
#include "xfa/fwl/cfwl_eventmouse.h"
#include "xfa/fwl/cfwl_messagekey.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_themetext.h"
#include "xfa/fwl/fwl_widgetdef.h"
#include "xfa/fwl/ifwl_themeprovider.h"

CFWL_PushButton::CFWL_PushButton(CFWL_App* app)
    : CFWL_Widget(app, Properties(), nullptr) {}

CFWL_PushButton::~CFWL_PushButton() = default;

FWL_Type CFWL_PushButton::GetClassID() const {
  return FWL_Type::PushButton;
}

void CFWL_PushButton::SetStates(uint32_t dwStates) {
  if (dwStates & FWL_STATE_WGT_Disabled) {
    m_Properties.m_dwStates = FWL_STATE_WGT_Disabled;
    return;
  }
  CFWL_Widget::SetStates(dwStates);
}

void CFWL_PushButton::Update() {
  if (IsLocked())
    return;

  m_ClientRect = GetClientRect();
  m_CaptionRect = m_ClientRect;
}

void CFWL_PushButton::DrawWidget(CFGAS_GEGraphics* pGraphics,
                                 const CFX_Matrix& matrix) {
  if (!pGraphics)
    return;

  if (HasBorder())
    DrawBorder(pGraphics, CFWL_ThemePart::Part::kBorder, matrix);

  DrawBkground(pGraphics, matrix);
}

void CFWL_PushButton::DrawBkground(CFGAS_GEGraphics* pGraphics,
                                   const CFX_Matrix& matrix) {
  CFWL_ThemeBackground param(this, pGraphics);
  param.m_iPart = CFWL_ThemePart::Part::kBackground;
  param.m_dwStates = GetPartStates();
  param.m_matrix = matrix;
  param.m_PartRect = m_ClientRect;
  if (m_Properties.m_dwStates & FWL_STATE_WGT_Focused)
    param.m_pRtData = &m_CaptionRect;
  GetThemeProvider()->DrawBackground(param);
}

uint32_t CFWL_PushButton::GetPartStates() {
  uint32_t dwStates = CFWL_PartState_Normal;
  if (m_Properties.m_dwStates & FWL_STATE_WGT_Focused)
    dwStates |= CFWL_PartState_Focused;
  if (m_Properties.m_dwStates & FWL_STATE_WGT_Disabled)
    dwStates = CFWL_PartState_Disabled;
  else if (m_Properties.m_dwStates & FWL_STATE_PSB_Pressed)
    dwStates |= CFWL_PartState_Pressed;
  else if (m_Properties.m_dwStates & FWL_STATE_PSB_Hovered)
    dwStates |= CFWL_PartState_Hovered;
  return dwStates;
}

void CFWL_PushButton::OnProcessMessage(CFWL_Message* pMessage) {
  if (!IsEnabled())
    return;

  switch (pMessage->GetType()) {
    case CFWL_Message::Type::kSetFocus:
      OnFocusGained();
      break;
    case CFWL_Message::Type::kKillFocus:
      OnFocusLost();
      break;
    case CFWL_Message::Type::kMouse: {
      CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
      switch (pMsg->m_dwCmd) {
        case CFWL_MessageMouse::MouseCommand::kLeftButtonDown:
          OnLButtonDown(pMsg);
          break;
        case CFWL_MessageMouse::MouseCommand::kLeftButtonUp:
          OnLButtonUp(pMsg);
          break;
        case CFWL_MessageMouse::MouseCommand::kMove:
          OnMouseMove(pMsg);
          break;
        case CFWL_MessageMouse::MouseCommand::kLeave:
          OnMouseLeave(pMsg);
          break;
        default:
          break;
      }
      break;
    }
    case CFWL_Message::Type::kKey: {
      CFWL_MessageKey* pKey = static_cast<CFWL_MessageKey*>(pMessage);
      if (pKey->m_dwCmd == CFWL_MessageKey::KeyCommand::kKeyDown)
        OnKeyDown(pKey);
      break;
    }
    default:
      break;
  }
  // Dst target could be |this|, continue only if not destroyed by above.
  if (pMessage->GetDstTarget())
    CFWL_Widget::OnProcessMessage(pMessage);
}

void CFWL_PushButton::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                   const CFX_Matrix& matrix) {
  DrawWidget(pGraphics, matrix);
}

void CFWL_PushButton::OnFocusGained() {
  m_Properties.m_dwStates |= FWL_STATE_WGT_Focused;
  RepaintRect(m_ClientRect);
}

void CFWL_PushButton::OnFocusLost() {
  m_Properties.m_dwStates &= ~FWL_STATE_WGT_Focused;
  RepaintRect(m_ClientRect);
}

void CFWL_PushButton::OnLButtonDown(CFWL_MessageMouse* pMsg) {
  m_bBtnDown = true;
  m_Properties.m_dwStates |= FWL_STATE_PSB_Hovered;
  m_Properties.m_dwStates |= FWL_STATE_PSB_Pressed;
  RepaintRect(m_ClientRect);
}

void CFWL_PushButton::OnLButtonUp(CFWL_MessageMouse* pMsg) {
  m_bBtnDown = false;
  if (m_ClientRect.Contains(pMsg->m_pos)) {
    m_Properties.m_dwStates &= ~FWL_STATE_PSB_Pressed;
    m_Properties.m_dwStates |= FWL_STATE_PSB_Hovered;
  } else {
    m_Properties.m_dwStates &= ~FWL_STATE_PSB_Hovered;
    m_Properties.m_dwStates &= ~FWL_STATE_PSB_Pressed;
  }
  if (m_ClientRect.Contains(pMsg->m_pos)) {
    CFWL_Event wmClick(CFWL_Event::Type::Click, this);
    DispatchEvent(&wmClick);
  }
  RepaintRect(m_ClientRect);
}

void CFWL_PushButton::OnMouseMove(CFWL_MessageMouse* pMsg) {
  bool bRepaint = false;
  if (m_bBtnDown) {
    if (m_ClientRect.Contains(pMsg->m_pos)) {
      if ((m_Properties.m_dwStates & FWL_STATE_PSB_Pressed) == 0) {
        m_Properties.m_dwStates |= FWL_STATE_PSB_Pressed;
        bRepaint = true;
      }
      if (m_Properties.m_dwStates & FWL_STATE_PSB_Hovered) {
        m_Properties.m_dwStates &= ~FWL_STATE_PSB_Hovered;
        bRepaint = true;
      }
    } else {
      if (m_Properties.m_dwStates & FWL_STATE_PSB_Pressed) {
        m_Properties.m_dwStates &= ~FWL_STATE_PSB_Pressed;
        bRepaint = true;
      }
      if ((m_Properties.m_dwStates & FWL_STATE_PSB_Hovered) == 0) {
        m_Properties.m_dwStates |= FWL_STATE_PSB_Hovered;
        bRepaint = true;
      }
    }
  } else {
    if (!m_ClientRect.Contains(pMsg->m_pos))
      return;
    if ((m_Properties.m_dwStates & FWL_STATE_PSB_Hovered) == 0) {
      m_Properties.m_dwStates |= FWL_STATE_PSB_Hovered;
      bRepaint = true;
    }
  }
  if (bRepaint)
    RepaintRect(m_ClientRect);
}

void CFWL_PushButton::OnMouseLeave(CFWL_MessageMouse* pMsg) {
  m_bBtnDown = false;
  m_Properties.m_dwStates &= ~FWL_STATE_PSB_Hovered;
  m_Properties.m_dwStates &= ~FWL_STATE_PSB_Pressed;
  RepaintRect(m_ClientRect);
}

void CFWL_PushButton::OnKeyDown(CFWL_MessageKey* pMsg) {
  if (pMsg->m_dwKeyCode != XFA_FWL_VKEY_Return)
    return;

  CFWL_EventMouse wmMouse(this, nullptr,
                          CFWL_MessageMouse::MouseCommand::kLeftButtonUp);
  DispatchEvent(&wmMouse);
  if (!wmMouse.GetSrcTarget())
    return;

  CFWL_Event wmClick(CFWL_Event::Type::Click, this);
  DispatchEvent(&wmClick);
}
