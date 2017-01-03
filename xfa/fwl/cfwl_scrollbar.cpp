// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_scrollbar.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_messagemousewheel.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_themepart.h"
#include "xfa/fwl/cfwl_timerinfo.h"
#include "xfa/fwl/ifwl_themeprovider.h"

#define FWL_SCROLLBAR_Elapse 500

namespace {

const float kMinThumbSize = 5.0f;

}  // namespace

CFWL_ScrollBar::CFWL_ScrollBar(
    const CFWL_App* app,
    std::unique_ptr<CFWL_WidgetProperties> properties,
    CFWL_Widget* pOuter)
    : CFWL_Widget(app, std::move(properties), pOuter),
      m_pTimerInfo(nullptr),
      m_fRangeMin(0),
      m_fRangeMax(-1),
      m_fPageSize(0),
      m_fStepSize(0),
      m_fPos(0),
      m_fTrackPos(0),
      m_iMinButtonState(CFWL_PartState_Normal),
      m_iMaxButtonState(CFWL_PartState_Normal),
      m_iThumbButtonState(CFWL_PartState_Normal),
      m_iMinTrackState(CFWL_PartState_Normal),
      m_iMaxTrackState(CFWL_PartState_Normal),
      m_fLastTrackPos(0),
      m_cpTrackPointX(0),
      m_cpTrackPointY(0),
      m_iMouseWheel(0),
      m_bMouseDown(false),
      m_fButtonLen(0),
      m_bMinSize(false),
      m_Timer(this) {
  m_rtClient.Reset();
  m_rtThumb.Reset();
  m_rtMinBtn.Reset();
  m_rtMaxBtn.Reset();
  m_rtMinTrack.Reset();
  m_rtMaxTrack.Reset();
}

CFWL_ScrollBar::~CFWL_ScrollBar() {}

FWL_Type CFWL_ScrollBar::GetClassID() const {
  return FWL_Type::ScrollBar;
}

void CFWL_ScrollBar::Update() {
  if (IsLocked())
    return;
  if (!m_pProperties->m_pThemeProvider)
    m_pProperties->m_pThemeProvider = GetAvailableTheme();

  Layout();
}

void CFWL_ScrollBar::DrawWidget(CFX_Graphics* pGraphics,
                                const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return;
  if (!m_pProperties->m_pThemeProvider)
    return;

  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  if (HasBorder())
    DrawBorder(pGraphics, CFWL_Part::Border, pTheme, pMatrix);
  DrawTrack(pGraphics, pTheme, true, pMatrix);
  DrawTrack(pGraphics, pTheme, false, pMatrix);
  DrawArrowBtn(pGraphics, pTheme, true, pMatrix);
  DrawArrowBtn(pGraphics, pTheme, false, pMatrix);
  DrawThumb(pGraphics, pTheme, pMatrix);
}

void CFWL_ScrollBar::SetTrackPos(FX_FLOAT fTrackPos) {
  m_fTrackPos = fTrackPos;
  m_rtThumb = CalcThumbButtonRect(m_rtThumb);
  m_rtMinTrack = CalcMinTrackRect(m_rtMinTrack);
  m_rtMaxTrack = CalcMaxTrackRect(m_rtMaxTrack);
}

bool CFWL_ScrollBar::DoScroll(CFWL_EventScroll::Code dwCode, FX_FLOAT fPos) {
  if (dwCode == CFWL_EventScroll::Code::None)
    return false;
  return OnScroll(dwCode, fPos);
}

void CFWL_ScrollBar::DrawTrack(CFX_Graphics* pGraphics,
                               IFWL_ThemeProvider* pTheme,
                               bool bLower,
                               const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground param;
  param.m_pWidget = this;
  param.m_iPart = bLower ? CFWL_Part::LowerTrack : CFWL_Part::UpperTrack;
  param.m_dwStates = (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)
                         ? CFWL_PartState_Disabled
                         : (bLower ? m_iMinTrackState : m_iMaxTrackState);
  param.m_pGraphics = pGraphics;
  param.m_matrix.Concat(*pMatrix);
  param.m_rtPart = bLower ? m_rtMinTrack : m_rtMaxTrack;
  pTheme->DrawBackground(&param);
}

void CFWL_ScrollBar::DrawArrowBtn(CFX_Graphics* pGraphics,
                                  IFWL_ThemeProvider* pTheme,
                                  bool bMinBtn,
                                  const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground param;
  param.m_pWidget = this;
  param.m_iPart = bMinBtn ? CFWL_Part::ForeArrow : CFWL_Part::BackArrow;
  param.m_dwStates = (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)
                         ? CFWL_PartState_Disabled
                         : (bMinBtn ? m_iMinButtonState : m_iMaxButtonState);
  param.m_pGraphics = pGraphics;
  param.m_matrix.Concat(*pMatrix);
  param.m_rtPart = bMinBtn ? m_rtMinBtn : m_rtMaxBtn;
  if (param.m_rtPart.height > 0 && param.m_rtPart.width > 0)
    pTheme->DrawBackground(&param);
}

void CFWL_ScrollBar::DrawThumb(CFX_Graphics* pGraphics,
                               IFWL_ThemeProvider* pTheme,
                               const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground param;
  param.m_pWidget = this;
  param.m_iPart = CFWL_Part::Thumb;
  param.m_dwStates = (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)
                         ? CFWL_PartState_Disabled
                         : m_iThumbButtonState;
  param.m_pGraphics = pGraphics;
  param.m_matrix.Concat(*pMatrix);
  param.m_rtPart = m_rtThumb;
  pTheme->DrawBackground(&param);
}

void CFWL_ScrollBar::Layout() {
  m_rtClient = GetClientRect();

  CalcButtonLen();
  m_rtMinBtn = CalcMinButtonRect();
  m_rtMaxBtn = CalcMaxButtonRect();
  m_rtThumb = CalcThumbButtonRect(m_rtThumb);
  m_rtMinTrack = CalcMinTrackRect(m_rtMinTrack);
  m_rtMaxTrack = CalcMaxTrackRect(m_rtMaxTrack);
}

void CFWL_ScrollBar::CalcButtonLen() {
  m_fButtonLen = IsVertical() ? m_rtClient.width : m_rtClient.height;
  FX_FLOAT fLength = IsVertical() ? m_rtClient.height : m_rtClient.width;
  if (fLength < m_fButtonLen * 2) {
    m_fButtonLen = fLength / 2;
    m_bMinSize = true;
  } else {
    m_bMinSize = false;
  }
}

CFX_RectF CFWL_ScrollBar::CalcMinButtonRect() {
  CFX_RectF rect;
  if (IsVertical())
    rect.Set(m_rtClient.left, m_rtClient.top, m_rtClient.width, m_fButtonLen);
  else
    rect.Set(m_rtClient.left, m_rtClient.top, m_fButtonLen, m_rtClient.height);
  return rect;
}

CFX_RectF CFWL_ScrollBar::CalcMaxButtonRect() {
  CFX_RectF rect;
  if (IsVertical()) {
    rect.Set(m_rtClient.left, m_rtClient.bottom() - m_fButtonLen,
             m_rtClient.width, m_fButtonLen);
  } else {
    rect.Set(m_rtClient.right() - m_fButtonLen, m_rtClient.top, m_fButtonLen,
             m_rtClient.height);
  }
  return rect;
}

CFX_RectF CFWL_ScrollBar::CalcThumbButtonRect(const CFX_RectF& rtThumb) {
  CFX_RectF rect;
  rect.Reset();

  if (!IsEnabled())
    return rect;

  if (m_bMinSize) {
    rect.left = rtThumb.left;
    rect.top = rtThumb.top;
    return rect;
  }

  FX_FLOAT fRange = m_fRangeMax - m_fRangeMin;
  if (fRange < 0) {
    if (IsVertical())
      rect.Set(m_rtClient.left, m_rtMaxBtn.bottom(), m_rtClient.width, 0);
    else
      rect.Set(m_rtMaxBtn.right(), m_rtClient.top, 0, m_rtClient.height);
    return rect;
  }

  CFX_RectF rtClient = m_rtClient;
  FX_FLOAT fLength = IsVertical() ? rtClient.height : rtClient.width;
  FX_FLOAT fSize = m_fButtonLen;
  fLength -= fSize * 2.0f;
  if (fLength < fSize)
    fLength = 0.0f;

  FX_FLOAT fThumbSize = fLength * fLength / (fRange + fLength);
  fThumbSize = std::max(fThumbSize, kMinThumbSize);

  FX_FLOAT fDiff = std::max(fLength - fThumbSize, 0.0f);
  FX_FLOAT fTrackPos =
      std::max(std::min(m_fTrackPos, m_fRangeMax), m_fRangeMin);
  if (!fRange)
    return rect;

  FX_FLOAT iPos = fSize + fDiff * (fTrackPos - m_fRangeMin) / fRange;
  rect.left = rtClient.left;
  rect.top = rtClient.top;
  if (IsVertical()) {
    rect.top += iPos;
    rect.width = rtClient.width;
    rect.height = fThumbSize;
  } else {
    rect.left += iPos;
    rect.width = fThumbSize;
    rect.height = rtClient.height;
  }
  return rect;
}

CFX_RectF CFWL_ScrollBar::CalcMinTrackRect(const CFX_RectF& rtMinRect) {
  CFX_RectF rect;
  rect.Reset();

  if (m_bMinSize) {
    rect.left = rtMinRect.left;
    rect.top = rtMinRect.top;
    return rect;
  }

  rect.left = m_rtClient.left;
  rect.top = m_rtClient.top;
  if (IsVertical()) {
    rect.width = m_rtClient.width;
    rect.height = (m_rtThumb.top + m_rtThumb.bottom()) / 2;
  } else {
    rect.width = (m_rtThumb.left + m_rtThumb.right()) / 2;
    rect.height = m_rtClient.height;
  }
  return rect;
}

CFX_RectF CFWL_ScrollBar::CalcMaxTrackRect(const CFX_RectF& rtMaxRect) {
  CFX_RectF rect;
  if (m_bMinSize) {
    rect.Set(rtMaxRect.left, rtMaxRect.top, 0, 0);
    return rect;
  }

  if (IsVertical()) {
    FX_FLOAT iy = (m_rtThumb.top + m_rtThumb.bottom()) / 2;
    rect.Set(m_rtClient.left, iy, m_rtClient.width, m_rtClient.bottom() - iy);
  } else {
    FX_FLOAT ix = (m_rtThumb.left + m_rtThumb.right()) / 2;
    rect.Set(ix, m_rtClient.top, m_rtClient.height - ix, m_rtClient.height);
  }
  return rect;
}

FX_FLOAT CFWL_ScrollBar::GetTrackPointPos(FX_FLOAT fx, FX_FLOAT fy) {
  FX_FLOAT fDiffX = fx - m_cpTrackPointX;
  FX_FLOAT fDiffY = fy - m_cpTrackPointY;
  FX_FLOAT fRange = m_fRangeMax - m_fRangeMin;
  FX_FLOAT fPos;

  if (IsVertical()) {
    fPos = fRange * fDiffY /
           (m_rtMaxBtn.top - m_rtMinBtn.bottom() - m_rtThumb.height);
  } else {
    fPos = fRange * fDiffX /
           (m_rtMaxBtn.left - m_rtMinBtn.right() - m_rtThumb.width);
  }

  fPos += m_fLastTrackPos;
  return std::min(std::max(fPos, m_fRangeMin), m_fRangeMax);
}

bool CFWL_ScrollBar::SendEvent() {
  if (m_iMinButtonState == CFWL_PartState_Pressed) {
    DoScroll(CFWL_EventScroll::Code::StepBackward, m_fTrackPos);
    return false;
  }
  if (m_iMaxButtonState == CFWL_PartState_Pressed) {
    DoScroll(CFWL_EventScroll::Code::StepForward, m_fTrackPos);
    return false;
  }
  if (m_iMinTrackState == CFWL_PartState_Pressed) {
    DoScroll(CFWL_EventScroll::Code::PageBackward, m_fTrackPos);
    return m_rtThumb.Contains(m_cpTrackPointX, m_cpTrackPointY);
  }
  if (m_iMaxTrackState == CFWL_PartState_Pressed) {
    DoScroll(CFWL_EventScroll::Code::PageForward, m_fTrackPos);
    return m_rtThumb.Contains(m_cpTrackPointX, m_cpTrackPointY);
  }
  if (m_iMouseWheel) {
    CFWL_EventScroll::Code dwCode = m_iMouseWheel < 0
                                        ? CFWL_EventScroll::Code::StepForward
                                        : CFWL_EventScroll::Code::StepBackward;
    DoScroll(dwCode, m_fTrackPos);
  }
  return true;
}

bool CFWL_ScrollBar::OnScroll(CFWL_EventScroll::Code dwCode, FX_FLOAT fPos) {
  CFWL_EventScroll ev(this);
  ev.m_iScrollCode = dwCode;
  ev.m_fPos = fPos;
  DispatchEvent(&ev);
  return true;
}

void CFWL_ScrollBar::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return;

  CFWL_Message::Type type = pMessage->GetType();
  if (type == CFWL_Message::Type::Mouse) {
    CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
    switch (pMsg->m_dwCmd) {
      case FWL_MouseCommand::LeftButtonDown:
        OnLButtonDown(pMsg->m_dwFlags, pMsg->m_fx, pMsg->m_fy);
        break;
      case FWL_MouseCommand::LeftButtonUp:
        OnLButtonUp(pMsg->m_dwFlags, pMsg->m_fx, pMsg->m_fy);
        break;
      case FWL_MouseCommand::Move:
        OnMouseMove(pMsg->m_dwFlags, pMsg->m_fx, pMsg->m_fy);
        break;
      case FWL_MouseCommand::Leave:
        OnMouseLeave();
        break;
      default:
        break;
    }
  } else if (type == CFWL_Message::Type::MouseWheel) {
    CFWL_MessageMouseWheel* pMsg =
        static_cast<CFWL_MessageMouseWheel*>(pMessage);
    OnMouseWheel(pMsg->m_fx, pMsg->m_fy, pMsg->m_dwFlags, pMsg->m_fDeltaX,
                 pMsg->m_fDeltaY);
  }
}

void CFWL_ScrollBar::OnDrawWidget(CFX_Graphics* pGraphics,
                                  const CFX_Matrix* pMatrix) {
  DrawWidget(pGraphics, pMatrix);
}

void CFWL_ScrollBar::OnLButtonDown(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy) {
  if (!IsEnabled())
    return;

  m_bMouseDown = true;
  SetGrab(true);
  m_cpTrackPointX = fx;
  m_cpTrackPointY = fy;
  m_fLastTrackPos = m_fTrackPos;
  if (m_rtMinBtn.Contains(fx, fy))
    DoMouseDown(0, m_rtMinBtn, m_iMinButtonState, fx, fy);
  else if (m_rtThumb.Contains(fx, fy))
    DoMouseDown(1, m_rtThumb, m_iThumbButtonState, fx, fy);
  else if (m_rtMaxBtn.Contains(fx, fy))
    DoMouseDown(2, m_rtMaxBtn, m_iMaxButtonState, fx, fy);
  else if (m_rtMinTrack.Contains(fx, fy))
    DoMouseDown(3, m_rtMinTrack, m_iMinTrackState, fx, fy);
  else
    DoMouseDown(4, m_rtMaxTrack, m_iMaxTrackState, fx, fy);

  if (!SendEvent())
    m_pTimerInfo = m_Timer.StartTimer(FWL_SCROLLBAR_Elapse, true);
}

void CFWL_ScrollBar::OnLButtonUp(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy) {
  m_pTimerInfo->StopTimer();
  m_bMouseDown = false;
  DoMouseUp(0, m_rtMinBtn, m_iMinButtonState, fx, fy);
  DoMouseUp(1, m_rtThumb, m_iThumbButtonState, fx, fy);
  DoMouseUp(2, m_rtMaxBtn, m_iMaxButtonState, fx, fy);
  DoMouseUp(3, m_rtMinTrack, m_iMinTrackState, fx, fy);
  DoMouseUp(4, m_rtMaxTrack, m_iMaxTrackState, fx, fy);
  SetGrab(false);
}

void CFWL_ScrollBar::OnMouseMove(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy) {
  DoMouseMove(0, m_rtMinBtn, m_iMinButtonState, fx, fy);
  DoMouseMove(1, m_rtThumb, m_iThumbButtonState, fx, fy);
  DoMouseMove(2, m_rtMaxBtn, m_iMaxButtonState, fx, fy);
  DoMouseMove(3, m_rtMinTrack, m_iMinTrackState, fx, fy);
  DoMouseMove(4, m_rtMaxTrack, m_iMaxTrackState, fx, fy);
}

void CFWL_ScrollBar::OnMouseLeave() {
  DoMouseLeave(0, m_rtMinBtn, m_iMinButtonState);
  DoMouseLeave(1, m_rtThumb, m_iThumbButtonState);
  DoMouseLeave(2, m_rtMaxBtn, m_iMaxButtonState);
  DoMouseLeave(3, m_rtMinTrack, m_iMinTrackState);
  DoMouseLeave(4, m_rtMaxTrack, m_iMaxTrackState);
}

void CFWL_ScrollBar::OnMouseWheel(FX_FLOAT fx,
                                  FX_FLOAT fy,
                                  uint32_t dwFlags,
                                  FX_FLOAT fDeltaX,
                                  FX_FLOAT fDeltaY) {
  m_iMouseWheel = (int32_t)fDeltaX;
  SendEvent();
  m_iMouseWheel = 0;
}

void CFWL_ScrollBar::DoMouseDown(int32_t iItem,
                                 const CFX_RectF& rtItem,
                                 int32_t& iState,
                                 FX_FLOAT fx,
                                 FX_FLOAT fy) {
  if (!rtItem.Contains(fx, fy))
    return;
  if (iState == CFWL_PartState_Pressed)
    return;

  iState = CFWL_PartState_Pressed;
  RepaintRect(rtItem);
}

void CFWL_ScrollBar::DoMouseUp(int32_t iItem,
                               const CFX_RectF& rtItem,
                               int32_t& iState,
                               FX_FLOAT fx,
                               FX_FLOAT fy) {
  int32_t iNewState =
      rtItem.Contains(fx, fy) ? CFWL_PartState_Hovered : CFWL_PartState_Normal;
  if (iState == iNewState)
    return;

  iState = iNewState;
  RepaintRect(rtItem);
  OnScroll(CFWL_EventScroll::Code::EndScroll, m_fTrackPos);
}

void CFWL_ScrollBar::DoMouseMove(int32_t iItem,
                                 const CFX_RectF& rtItem,
                                 int32_t& iState,
                                 FX_FLOAT fx,
                                 FX_FLOAT fy) {
  if (!m_bMouseDown) {
    int32_t iNewState = rtItem.Contains(fx, fy) ? CFWL_PartState_Hovered
                                                : CFWL_PartState_Normal;
    if (iState == iNewState)
      return;

    iState = iNewState;
    RepaintRect(rtItem);
  } else if ((2 == iItem) && (m_iThumbButtonState == CFWL_PartState_Pressed)) {
    FX_FLOAT fPos = GetTrackPointPos(fx, fy);
    m_fTrackPos = fPos;
    OnScroll(CFWL_EventScroll::Code::TrackPos, fPos);
  }
}

void CFWL_ScrollBar::DoMouseLeave(int32_t iItem,
                                  const CFX_RectF& rtItem,
                                  int32_t& iState) {
  if (iState == CFWL_PartState_Normal)
    return;

  iState = CFWL_PartState_Normal;
  RepaintRect(rtItem);
}

void CFWL_ScrollBar::DoMouseHover(int32_t iItem,
                                  const CFX_RectF& rtItem,
                                  int32_t& iState) {
  if (iState == CFWL_PartState_Hovered)
    return;

  iState = CFWL_PartState_Hovered;
  RepaintRect(rtItem);
}

CFWL_ScrollBar::Timer::Timer(CFWL_ScrollBar* pToolTip) : CFWL_Timer(pToolTip) {}

void CFWL_ScrollBar::Timer::Run(CFWL_TimerInfo* pTimerInfo) {
  CFWL_ScrollBar* pButton = static_cast<CFWL_ScrollBar*>(m_pWidget);

  if (pButton->m_pTimerInfo)
    pButton->m_pTimerInfo->StopTimer();

  if (!pButton->SendEvent())
    pButton->m_pTimerInfo = StartTimer(0, true);
}
