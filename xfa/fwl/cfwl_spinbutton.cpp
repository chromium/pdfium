// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_spinbutton.h"

#include <memory>
#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/cfwl_event.h"
#include "xfa/fwl/cfwl_messagekey.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_timerinfo.h"
#include "xfa/fwl/cfwl_widgetproperties.h"
#include "xfa/fwl/ifwl_themeprovider.h"

namespace {
const int kElapseTime = 200;

}  // namespace

CFWL_SpinButton::CFWL_SpinButton(
    const CFWL_App* app,
    std::unique_ptr<CFWL_WidgetProperties> properties)
    : CFWL_Widget(app, std::move(properties), nullptr),
      m_dwUpState(CFWL_PartState_Normal),
      m_dwDnState(CFWL_PartState_Normal),
      m_iButtonIndex(0),
      m_bLButtonDwn(false),
      m_pTimerInfo(nullptr),
      m_Timer(this) {
  m_rtClient.Reset();
  m_rtUpButton.Reset();
  m_rtDnButton.Reset();
  m_pProperties->m_dwStyleExes |= FWL_STYLEEXE_SPB_Vert;
}

CFWL_SpinButton::~CFWL_SpinButton() {}

FWL_Type CFWL_SpinButton::GetClassID() const {
  return FWL_Type::SpinButton;
}

void CFWL_SpinButton::Update() {
  if (IsLocked())
    return;

  m_rtClient = GetClientRect();
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXE_SPB_Vert) {
    m_rtUpButton = CFX_RectF(m_rtClient.top, m_rtClient.left, m_rtClient.width,
                             m_rtClient.height / 2);
    m_rtDnButton =
        CFX_RectF(m_rtClient.left, m_rtClient.top + m_rtClient.height / 2,
                  m_rtClient.width, m_rtClient.height / 2);
  } else {
    m_rtUpButton = CFX_RectF(m_rtClient.TopLeft(), m_rtClient.width / 2,
                             m_rtClient.height);
    m_rtDnButton =
        CFX_RectF(m_rtClient.left + m_rtClient.width / 2, m_rtClient.top,
                  m_rtClient.width / 2, m_rtClient.height);
  }
}

FWL_WidgetHit CFWL_SpinButton::HitTest(const CFX_PointF& point) {
  if (m_rtClient.Contains(point))
    return FWL_WidgetHit::Client;
  if (HasBorder() && (m_rtClient.Contains(point)))
    return FWL_WidgetHit::Border;
  if (m_rtUpButton.Contains(point))
    return FWL_WidgetHit::UpButton;
  if (m_rtDnButton.Contains(point))
    return FWL_WidgetHit::DownButton;
  return FWL_WidgetHit::Unknown;
}

void CFWL_SpinButton::DrawWidget(CFX_Graphics* pGraphics,
                                 const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return;

  CFX_RectF rtClip(m_rtClient);
  if (pMatrix)
    pMatrix->TransformRect(rtClip);

  IFWL_ThemeProvider* pTheme = GetAvailableTheme();
  if (HasBorder())
    DrawBorder(pGraphics, CFWL_Part::Border, pTheme, pMatrix);

  DrawUpButton(pGraphics, pTheme, pMatrix);
  DrawDownButton(pGraphics, pTheme, pMatrix);
}

void CFWL_SpinButton::DisableButton() {
  m_dwDnState = CFWL_PartState_Disabled;
}

bool CFWL_SpinButton::IsUpButtonEnabled() {
  return m_dwUpState != CFWL_PartState_Disabled;
}

bool CFWL_SpinButton::IsDownButtonEnabled() {
  return m_dwDnState != CFWL_PartState_Disabled;
}

void CFWL_SpinButton::DrawUpButton(CFX_Graphics* pGraphics,
                                   IFWL_ThemeProvider* pTheme,
                                   const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground params;
  params.m_pWidget = this;
  params.m_iPart = CFWL_Part::UpButton;
  params.m_pGraphics = pGraphics;
  params.m_dwStates = m_dwUpState + 1;
  if (pMatrix)
    params.m_matrix.Concat(*pMatrix);

  params.m_rtPart = m_rtUpButton;
  pTheme->DrawBackground(&params);
}

void CFWL_SpinButton::DrawDownButton(CFX_Graphics* pGraphics,
                                     IFWL_ThemeProvider* pTheme,
                                     const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground params;
  params.m_pWidget = this;
  params.m_iPart = CFWL_Part::DownButton;
  params.m_pGraphics = pGraphics;
  params.m_dwStates = m_dwDnState + 1;
  if (pMatrix)
    params.m_matrix.Concat(*pMatrix);

  params.m_rtPart = m_rtDnButton;
  pTheme->DrawBackground(&params);
}

void CFWL_SpinButton::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return;

  switch (pMessage->GetType()) {
    case CFWL_Message::Type::SetFocus: {
      OnFocusChanged(pMessage, true);
      break;
    }
    case CFWL_Message::Type::KillFocus: {
      OnFocusChanged(pMessage, false);
      break;
    }
    case CFWL_Message::Type::Mouse: {
      CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
      switch (pMsg->m_dwCmd) {
        case FWL_MouseCommand::LeftButtonDown:
          OnLButtonDown(pMsg);
          break;
        case FWL_MouseCommand::LeftButtonUp:
          OnLButtonUp(pMsg);
          break;
        case FWL_MouseCommand::Move:
          OnMouseMove(pMsg);
          break;
        case FWL_MouseCommand::Leave:
          OnMouseLeave(pMsg);
          break;
        default:
          break;
      }
      break;
    }
    case CFWL_Message::Type::Key: {
      CFWL_MessageKey* pKey = static_cast<CFWL_MessageKey*>(pMessage);
      if (pKey->m_dwCmd == FWL_KeyCommand::KeyDown)
        OnKeyDown(pKey);
      break;
    }
    default:
      break;
  }
  CFWL_Widget::OnProcessMessage(pMessage);
}

void CFWL_SpinButton::OnDrawWidget(CFX_Graphics* pGraphics,
                                   const CFX_Matrix* pMatrix) {
  DrawWidget(pGraphics, pMatrix);
}

void CFWL_SpinButton::OnFocusChanged(CFWL_Message* pMsg, bool bSet) {
  if (bSet)
    m_pProperties->m_dwStates |= (FWL_WGTSTATE_Focused);
  else
    m_pProperties->m_dwStates &= ~(FWL_WGTSTATE_Focused);

  RepaintRect(m_rtClient);
}

void CFWL_SpinButton::OnLButtonDown(CFWL_MessageMouse* pMsg) {
  m_bLButtonDwn = true;
  SetGrab(true);
  SetFocus(true);

  bool bUpPress = m_rtUpButton.Contains(pMsg->m_pos) && IsUpButtonEnabled();
  bool bDnPress = m_rtDnButton.Contains(pMsg->m_pos) && IsDownButtonEnabled();
  if (!bUpPress && !bDnPress)
    return;
  if (bUpPress) {
    m_iButtonIndex = 0;
    m_dwUpState = CFWL_PartState_Pressed;
  }
  if (bDnPress) {
    m_iButtonIndex = 1;
    m_dwDnState = CFWL_PartState_Pressed;
  }

  CFWL_Event wmPosChanged(CFWL_Event::Type::Click, this);
  DispatchEvent(&wmPosChanged);

  RepaintRect(bUpPress ? m_rtUpButton : m_rtDnButton);
  m_pTimerInfo = m_Timer.StartTimer(kElapseTime, true);
}

void CFWL_SpinButton::OnLButtonUp(CFWL_MessageMouse* pMsg) {
  if (m_pProperties->m_dwStates & CFWL_PartState_Disabled)
    return;

  m_bLButtonDwn = false;
  SetGrab(false);
  SetFocus(false);
  if (m_pTimerInfo) {
    m_pTimerInfo->StopTimer();
    m_pTimerInfo = nullptr;
  }
  bool bRepaint = false;
  CFX_RectF rtInvalidate;
  if (m_dwUpState == CFWL_PartState_Pressed && IsUpButtonEnabled()) {
    m_dwUpState = CFWL_PartState_Normal;
    bRepaint = true;
    rtInvalidate = m_rtUpButton;
  } else if (m_dwDnState == CFWL_PartState_Pressed && IsDownButtonEnabled()) {
    m_dwDnState = CFWL_PartState_Normal;
    bRepaint = true;
    rtInvalidate = m_rtDnButton;
  }
  if (bRepaint)
    RepaintRect(rtInvalidate);
}

void CFWL_SpinButton::OnMouseMove(CFWL_MessageMouse* pMsg) {
  if (m_bLButtonDwn)
    return;

  bool bRepaint = false;
  CFX_RectF rtInvlidate;
  if (m_rtUpButton.Contains(pMsg->m_pos)) {
    if (IsUpButtonEnabled()) {
      if (m_dwUpState == CFWL_PartState_Hovered) {
        m_dwUpState = CFWL_PartState_Hovered;
        bRepaint = true;
        rtInvlidate = m_rtUpButton;
      }
      if (m_dwDnState != CFWL_PartState_Normal && IsDownButtonEnabled()) {
        m_dwDnState = CFWL_PartState_Normal;
        if (bRepaint)
          rtInvlidate.Union(m_rtDnButton);
        else
          rtInvlidate = m_rtDnButton;

        bRepaint = true;
      }
    }
    if (!IsDownButtonEnabled())
      DisableButton();

  } else if (m_rtDnButton.Contains(pMsg->m_pos)) {
    if (IsDownButtonEnabled()) {
      if (m_dwDnState != CFWL_PartState_Hovered) {
        m_dwDnState = CFWL_PartState_Hovered;
        bRepaint = true;
        rtInvlidate = m_rtDnButton;
      }
      if (m_dwUpState != CFWL_PartState_Normal && IsUpButtonEnabled()) {
        m_dwUpState = CFWL_PartState_Normal;
        if (bRepaint)
          rtInvlidate.Union(m_rtUpButton);
        else
          rtInvlidate = m_rtUpButton;
        bRepaint = true;
      }
    }
  } else if (m_dwUpState != CFWL_PartState_Normal ||
             m_dwDnState != CFWL_PartState_Normal) {
    if (m_dwUpState != CFWL_PartState_Normal) {
      m_dwUpState = CFWL_PartState_Normal;
      bRepaint = true;
      rtInvlidate = m_rtUpButton;
    }
    if (m_dwDnState != CFWL_PartState_Normal) {
      m_dwDnState = CFWL_PartState_Normal;
      if (bRepaint)
        rtInvlidate.Union(m_rtDnButton);
      else
        rtInvlidate = m_rtDnButton;

      bRepaint = true;
    }
  }
  if (bRepaint)
    RepaintRect(rtInvlidate);
}

void CFWL_SpinButton::OnMouseLeave(CFWL_MessageMouse* pMsg) {
  if (!pMsg)
    return;
  if (m_dwUpState != CFWL_PartState_Normal && IsUpButtonEnabled())
    m_dwUpState = CFWL_PartState_Normal;
  if (m_dwDnState != CFWL_PartState_Normal && IsDownButtonEnabled())
    m_dwDnState = CFWL_PartState_Normal;

  RepaintRect(m_rtClient);
}

void CFWL_SpinButton::OnKeyDown(CFWL_MessageKey* pMsg) {
  bool bUp =
      pMsg->m_dwKeyCode == FWL_VKEY_Up || pMsg->m_dwKeyCode == FWL_VKEY_Left;
  bool bDown =
      pMsg->m_dwKeyCode == FWL_VKEY_Down || pMsg->m_dwKeyCode == FWL_VKEY_Right;
  if (!bUp && !bDown)
    return;

  bool bUpEnable = IsUpButtonEnabled();
  bool bDownEnable = IsDownButtonEnabled();
  if (!bUpEnable && !bDownEnable)
    return;

  CFWL_Event wmPosChanged(CFWL_Event::Type::Click, this);
  DispatchEvent(&wmPosChanged);

  RepaintRect(bUpEnable ? m_rtUpButton : m_rtDnButton);
}

CFWL_SpinButton::Timer::Timer(CFWL_SpinButton* pToolTip)
    : CFWL_Timer(pToolTip) {}

void CFWL_SpinButton::Timer::Run(CFWL_TimerInfo* pTimerInfo) {
  CFWL_SpinButton* pButton = static_cast<CFWL_SpinButton*>(m_pWidget);

  if (!pButton->m_pTimerInfo)
    return;

  CFWL_Event wmPosChanged(CFWL_Event::Type::Click, pButton);
  pButton->DispatchEvent(&wmPosChanged);
}
