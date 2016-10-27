// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/ifwl_scrollbar.h"

#include "xfa/fwl/core/cfwl_message.h"
#include "xfa/fwl/core/cfwl_themebackground.h"
#include "xfa/fwl/core/cfwl_themepart.h"
#include "xfa/fwl/core/fwl_noteimp.h"
#include "xfa/fwl/core/ifwl_scrollbar.h"
#include "xfa/fwl/core/ifwl_themeprovider.h"

#define FWL_SCROLLBAR_Elapse 500
#define FWL_SCROLLBAR_MinThumb 5

IFWL_ScrollBar::IFWL_ScrollBar(const CFWL_WidgetImpProperties& properties,
                               IFWL_Widget* pOuter)
    : IFWL_Widget(properties, pOuter),
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
      m_bTrackMouseLeave(FALSE),
      m_bMouseHover(FALSE),
      m_bMouseDown(FALSE),
      m_bRepaintThumb(FALSE),
      m_fButtonLen(0),
      m_bMinSize(FALSE),
      m_bCustomLayout(false),
      m_fMinThumb(FWL_SCROLLBAR_MinThumb) {
  m_rtClient.Reset();
  m_rtThumb.Reset();
  m_rtMinBtn.Reset();
  m_rtMaxBtn.Reset();
  m_rtMinTrack.Reset();
  m_rtMaxTrack.Reset();
}

IFWL_ScrollBar::~IFWL_ScrollBar() {}

FWL_Type IFWL_ScrollBar::GetClassID() const {
  return FWL_Type::ScrollBar;
}

FWL_Error IFWL_ScrollBar::Initialize() {
  if (IFWL_Widget::Initialize() != FWL_Error::Succeeded)
    return FWL_Error::Indefinite;

  m_pDelegate = new CFWL_ScrollBarImpDelegate(this);
  return FWL_Error::Succeeded;
}

void IFWL_ScrollBar::Finalize() {
  delete m_pDelegate;
  m_pDelegate = nullptr;
  IFWL_Widget::Finalize();
}

FWL_Error IFWL_ScrollBar::GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize) {
  if (bAutoSize) {
    rect.Set(0, 0, 0, 0);
    FX_FLOAT* pfMinWidth = static_cast<FX_FLOAT*>(
        GetThemeCapacity(CFWL_WidgetCapacity::ScrollBarWidth));
    if (!pfMinWidth)
      return FWL_Error::Indefinite;
    if (IsVertical()) {
      rect.Set(0, 0, (*pfMinWidth), (*pfMinWidth) * 3);
    } else {
      rect.Set(0, 0, (*pfMinWidth) * 3, (*pfMinWidth));
    }
    IFWL_Widget::GetWidgetRect(rect, TRUE);
  } else {
    rect = m_pProperties->m_rtWidget;
  }
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_ScrollBar::Update() {
  if (IsLocked()) {
    return FWL_Error::Indefinite;
  }
  if (!m_pProperties->m_pThemeProvider) {
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  }
  Layout();
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_ScrollBar::DrawWidget(CFX_Graphics* pGraphics,
                                     const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return FWL_Error::Indefinite;
  if (!m_pProperties->m_pThemeProvider)
    return FWL_Error::Indefinite;
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  if (HasBorder()) {
    DrawBorder(pGraphics, CFWL_Part::Border, pTheme, pMatrix);
  }
  if (HasEdge()) {
    DrawEdge(pGraphics, CFWL_Part::Edge, pTheme, pMatrix);
  }
  DrawTrack(pGraphics, pTheme, TRUE, pMatrix);
  DrawTrack(pGraphics, pTheme, FALSE, pMatrix);
  DrawArrowBtn(pGraphics, pTheme, TRUE, pMatrix);
  DrawArrowBtn(pGraphics, pTheme, FALSE, pMatrix);
  DrawThumb(pGraphics, pTheme, pMatrix);
  return FWL_Error::Succeeded;
}

inline FX_BOOL IFWL_ScrollBar::IsVertical() {
  return m_pProperties->m_dwStyleExes & FWL_STYLEEXT_SCB_Vert;
}

FWL_Error IFWL_ScrollBar::GetRange(FX_FLOAT& fMin, FX_FLOAT& fMax) {
  fMin = m_fRangeMin;
  fMax = m_fRangeMax;
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_ScrollBar::SetRange(FX_FLOAT fMin, FX_FLOAT fMax) {
  m_fRangeMin = fMin;
  m_fRangeMax = fMax;
  return FWL_Error::Succeeded;
}

FX_FLOAT IFWL_ScrollBar::GetPageSize() {
  return m_fPageSize;
}

FWL_Error IFWL_ScrollBar::SetPageSize(FX_FLOAT fPageSize) {
  m_fPageSize = fPageSize;
  return FWL_Error::Succeeded;
}

FX_FLOAT IFWL_ScrollBar::GetStepSize() {
  return m_fStepSize;
}

FWL_Error IFWL_ScrollBar::SetStepSize(FX_FLOAT fStepSize) {
  m_fStepSize = fStepSize;
  return FWL_Error::Succeeded;
}

FX_FLOAT IFWL_ScrollBar::GetPos() {
  return m_fPos;
}

FWL_Error IFWL_ScrollBar::SetPos(FX_FLOAT fPos) {
  m_fPos = fPos;
  return FWL_Error::Succeeded;
}

FX_FLOAT IFWL_ScrollBar::GetTrackPos() {
  return m_fTrackPos;
}

FWL_Error IFWL_ScrollBar::SetTrackPos(FX_FLOAT fTrackPos) {
  m_fTrackPos = fTrackPos;
  CalcThumbButtonRect(m_rtThumb);
  CalcMinTrackRect(m_rtMinTrack);
  CalcMaxTrackRect(m_rtMaxTrack);
  return FWL_Error::Succeeded;
}

FX_BOOL IFWL_ScrollBar::DoScroll(uint32_t dwCode, FX_FLOAT fPos) {
  switch (dwCode) {
    case FWL_SCBCODE_Min:
    case FWL_SCBCODE_Max:
    case FWL_SCBCODE_PageBackward:
    case FWL_SCBCODE_PageForward:
    case FWL_SCBCODE_StepBackward:
      break;
    case FWL_SCBCODE_StepForward:
      break;
    case FWL_SCBCODE_Pos:
    case FWL_SCBCODE_TrackPos:
    case FWL_SCBCODE_EndScroll:
      break;
    default: { return FALSE; }
  }
  return OnScroll(dwCode, fPos);
}

void IFWL_ScrollBar::Run(IFWL_TimerInfo* pTimerInfo) {
  if (m_pTimerInfo)
    m_pTimerInfo->StopTimer();

  if (!SendEvent())
    m_pTimerInfo = StartTimer(0, true);
}

FWL_Error IFWL_ScrollBar::SetOuter(IFWL_Widget* pOuter) {
  m_pOuter = pOuter;
  return FWL_Error::Succeeded;
}

void IFWL_ScrollBar::DrawTrack(CFX_Graphics* pGraphics,
                               IFWL_ThemeProvider* pTheme,
                               FX_BOOL bLower,
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

void IFWL_ScrollBar::DrawArrowBtn(CFX_Graphics* pGraphics,
                                  IFWL_ThemeProvider* pTheme,
                                  FX_BOOL bMinBtn,
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
  if (param.m_rtPart.height > 0 && param.m_rtPart.width > 0) {
    pTheme->DrawBackground(&param);
  }
}

void IFWL_ScrollBar::DrawThumb(CFX_Graphics* pGraphics,
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

void IFWL_ScrollBar::Layout() {
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  CFWL_ThemePart part;
  part.m_pWidget = this;
  m_fMinThumb = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, CFWL_WidgetCapacity::Size));
  m_bCustomLayout = pTheme->IsCustomizedLayout(this);
  GetClientRect(m_rtClient);
  CalcButtonLen();
  CalcMinButtonRect(m_rtMinBtn);
  CalcMaxButtonRect(m_rtMaxBtn);
  CalcThumbButtonRect(m_rtThumb);
  CalcMinTrackRect(m_rtMinTrack);
  CalcMaxTrackRect(m_rtMaxTrack);
}

void IFWL_ScrollBar::CalcButtonLen() {
  m_fButtonLen = IsVertical() ? m_rtClient.width : m_rtClient.height;
  FX_FLOAT fLength = IsVertical() ? m_rtClient.height : m_rtClient.width;
  if (fLength < m_fButtonLen * 2) {
    m_fButtonLen = fLength / 2;
    m_bMinSize = TRUE;
  } else {
    m_bMinSize = FALSE;
  }
}

void IFWL_ScrollBar::CalcMinButtonRect(CFX_RectF& rect) {
  if (m_bCustomLayout)
    return;

  rect.left = m_rtClient.left;
  rect.top = m_rtClient.top;
  rect.width = IsVertical() ? m_rtClient.width : m_fButtonLen;
  rect.height = IsVertical() ? m_fButtonLen : m_rtClient.height;
}

void IFWL_ScrollBar::CalcMaxButtonRect(CFX_RectF& rect) {
  if (m_bCustomLayout)
    return;

  rect.left =
      IsVertical() ? m_rtClient.left : m_rtClient.right() - m_fButtonLen;
  rect.top = IsVertical() ? m_rtClient.bottom() - m_fButtonLen : m_rtClient.top;
  rect.width = IsVertical() ? m_rtClient.width : m_fButtonLen;
  rect.height = IsVertical() ? m_fButtonLen : m_rtClient.height;
}

void IFWL_ScrollBar::CalcThumbButtonRect(CFX_RectF& rect) {
  if (!IsEnabled()) {
    m_rtThumb.Reset();
    return;
  }
  if (m_bMinSize) {
    m_rtThumb.Empty();
    return;
  }
  FX_FLOAT fRange = m_fRangeMax - m_fRangeMin;
  memset(&rect, 0, sizeof(CFX_Rect));
  if (fRange < 0) {
    if (IsVertical()) {
      rect.Set(m_rtClient.left, m_rtMaxBtn.bottom(), m_rtClient.width, 0);
    } else {
      rect.Set(m_rtMaxBtn.right(), m_rtClient.top, 0, m_rtClient.height);
    }
    return;
  }
  CFX_RectF rtClient = m_rtClient;
  FX_FLOAT fLength = IsVertical() ? rtClient.height : rtClient.width;
  FX_FLOAT fSize = m_fButtonLen;
  if (m_bCustomLayout) {
    if (IsVertical()) {
      fLength = fLength - m_rtMinBtn.height - m_rtMaxBtn.height;
      if (fLength < m_rtMinBtn.height || fLength < m_rtMaxBtn.height) {
        fLength = 0.0f;
      }
    } else {
      fLength = fLength - m_rtMinBtn.width - m_rtMaxBtn.width;
      if (fLength < m_rtMinBtn.width || fLength < m_rtMaxBtn.width) {
        fLength = 0.0f;
      }
    }
  } else {
    fLength -= fSize * 2.0f;
    if (fLength < fSize) {
      fLength = 0.0f;
    }
  }
  FX_FLOAT fThumbSize = fLength * fLength / (fRange + fLength);
  if (fThumbSize < m_fMinThumb) {
    fThumbSize = m_fMinThumb;
  }
  FX_FLOAT fDiff = fLength - fThumbSize;
  if (fDiff < 0.0f) {
    fDiff = 0.0f;
  }
  FX_FLOAT fTrackPos = m_fTrackPos;
  if (fTrackPos > m_fRangeMax) {
    fTrackPos = m_fRangeMax;
  }
  if (fTrackPos < m_fRangeMin) {
    fTrackPos = m_fRangeMin;
  }
  if (!fRange)
    return;
  if (m_bCustomLayout) {
    FX_FLOAT iPos = fDiff * (fTrackPos - m_fRangeMin) / fRange;
    rect.left = rtClient.left;
    if (!IsVertical()) {
      if ((m_rtMinBtn.right() == m_rtMaxBtn.left && m_rtMinBtn.width > 0 &&
           m_rtMaxBtn.width > 0) ||
          (0 == m_rtMinBtn.width && 0 == m_rtMaxBtn.width)) {
        rect.left += iPos;
      } else {
        rect.left += m_rtMinBtn.right() + iPos;
      }
    }
    rect.top = rtClient.top;
    if (IsVertical()) {
      if ((m_rtMinBtn.bottom() == m_rtMaxBtn.top && m_rtMinBtn.height > 0 &&
           m_rtMaxBtn.height > 0) ||
          (0 == m_rtMinBtn.height && 0 == m_rtMaxBtn.height)) {
        rect.top += iPos;
      } else {
        rect.top += m_rtMinBtn.bottom() + iPos;
      }
    }
    rect.width = IsVertical() ? rtClient.width : fThumbSize;
    rect.height = IsVertical() ? fThumbSize : rtClient.height;
  } else {
    FX_FLOAT iPos = fSize + fDiff * (fTrackPos - m_fRangeMin) / fRange;
    rect.left = rtClient.left;
    if (!IsVertical()) {
      rect.left += iPos;
    }
    rect.top = rtClient.top;
    if (IsVertical()) {
      rect.top += iPos;
    }
    rect.width = IsVertical() ? rtClient.width : fThumbSize;
    rect.height = IsVertical() ? fThumbSize : rtClient.height;
  }
}

void IFWL_ScrollBar::CalcMinTrackRect(CFX_RectF& rect) {
  if (m_bMinSize) {
    rect.Empty();
    return;
  }
  FX_FLOAT fBottom = m_rtThumb.bottom();
  FX_FLOAT fRight = m_rtThumb.right();
  FX_FLOAT ix = (m_rtThumb.left + fRight) / 2;
  FX_FLOAT iy = (m_rtThumb.top + fBottom) / 2;
  rect.left = m_rtClient.left;
  rect.top = m_rtClient.top;
  FX_BOOL bVertical = IsVertical();
  rect.width = bVertical ? m_rtClient.width : ix;
  rect.height = bVertical ? iy : m_rtClient.height;
  if (m_bCustomLayout) {
    if (bVertical) {
      if (0 == m_rtMinBtn.height && 0 == m_rtMaxBtn.height) {
        rect.top = m_rtClient.top;
      } else if (m_rtMinBtn.top < m_rtThumb.top) {
        rect.top = m_rtMinBtn.bottom();
        rect.height -= (m_rtMinBtn.bottom() - m_rtClient.top);
      }
    } else {
      if (0 == m_rtMinBtn.width && 0 == m_rtMaxBtn.width) {
        rect.left = m_rtClient.left;
      } else if (m_rtMinBtn.left < m_rtThumb.left) {
        rect.left = m_rtMinBtn.right();
        rect.width -= (m_rtMinBtn.right() - m_rtClient.left);
      }
    }
  }
}

void IFWL_ScrollBar::CalcMaxTrackRect(CFX_RectF& rect) {
  if (m_bMinSize) {
    rect.Empty();
    return;
  }
  FX_FLOAT ix = (m_rtThumb.left + m_rtThumb.right()) / 2;
  FX_FLOAT iy = (m_rtThumb.top + m_rtThumb.bottom()) / 2;
  FX_BOOL bVertical = IsVertical();
  rect.left = bVertical ? m_rtClient.left : ix;
  rect.top = bVertical ? iy : m_rtClient.top;
  rect.width = bVertical ? m_rtClient.width : m_rtClient.right() - ix;
  rect.height = bVertical ? m_rtClient.bottom() - iy : m_rtClient.height;
  if (m_bCustomLayout) {
    if (bVertical) {
      if (m_rtMinBtn.top > m_rtThumb.top && m_rtMinBtn.height > 0 &&
          m_rtMaxBtn.height > 0) {
        rect.height -= (m_rtClient.bottom() - m_rtMinBtn.top);
      } else if (m_rtMinBtn.height > 0 && m_rtMaxBtn.height > 0) {
        rect.height -= (m_rtClient.bottom() - m_rtMaxBtn.top);
      }
    } else {
      if (m_rtMinBtn.left > m_rtThumb.left && m_rtMinBtn.width > 0 &&
          m_rtMaxBtn.width > 0) {
        rect.width -= (m_rtClient.right() - m_rtMinBtn.left);
      } else if (m_rtMinBtn.width > 0 && m_rtMaxBtn.width > 0) {
        rect.width -= (m_rtClient.right() - m_rtMaxBtn.left);
      }
    }
  }
}

FX_FLOAT IFWL_ScrollBar::GetTrackPointPos(FX_FLOAT fx, FX_FLOAT fy) {
  FX_FLOAT fDiffX = fx - m_cpTrackPointX;
  FX_FLOAT fDiffY = fy - m_cpTrackPointY;
  FX_FLOAT fRange = m_fRangeMax - m_fRangeMin;
  FX_FLOAT fPos;
  if (m_bCustomLayout) {
    if (IsVertical()) {
      if (0 == m_rtMinBtn.height && 0 == m_rtMaxBtn.height) {
        fPos = fRange * fDiffY / (m_rtClient.height - m_rtThumb.height);
      } else if (m_rtMinBtn.bottom() == m_rtMaxBtn.top) {
        fPos = fRange * fDiffY /
               (m_rtMinBtn.top - m_rtClient.top - m_rtThumb.height);
      } else {
        fPos = fRange * fDiffY /
               (m_rtMaxBtn.top - m_rtMinBtn.bottom() - m_rtThumb.height);
      }
    } else {
      if (0 == m_rtMinBtn.width && 0 == m_rtMaxBtn.width) {
        fPos = fRange * fDiffX / (m_rtClient.width - m_rtThumb.width);
      } else if (m_rtMinBtn.right() == m_rtMaxBtn.left) {
        fPos = fRange * fDiffX /
               (m_rtMinBtn.left - m_rtClient.left - m_rtThumb.width);
      } else {
        fPos = fRange * fDiffX /
               (m_rtMaxBtn.left - m_rtMinBtn.right() - m_rtThumb.width);
      }
    }
  } else {
    if (IsVertical()) {
      fPos = fRange * fDiffY /
             (m_rtMaxBtn.top - m_rtMinBtn.bottom() - m_rtThumb.height);
    } else {
      fPos = fRange * fDiffX /
             (m_rtMaxBtn.left - m_rtMinBtn.right() - m_rtThumb.width);
    }
  }
  fPos += m_fLastTrackPos;
  if (fPos < m_fRangeMin) {
    fPos = m_fRangeMin;
  }
  if (fPos > m_fRangeMax) {
    fPos = m_fRangeMax;
  }
  return fPos;
}

void IFWL_ScrollBar::GetTrackRect(CFX_RectF& rect, FX_BOOL bLower) {
  bool bDisabled = !!(m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled);
  if (bDisabled || m_bCustomLayout) {
    rect = bLower ? m_rtMinTrack : m_rtMaxTrack;
  } else {
    FX_FLOAT fW = m_rtThumb.width / 2;
    FX_FLOAT fH = m_rtThumb.height / 2;
    FX_BOOL bVert = IsVertical();
    if (bLower) {
      if (bVert) {
        FX_FLOAT fMinTrackHeight = m_rtMinTrack.height - fH - m_rtMinBtn.height;
        fMinTrackHeight = (fMinTrackHeight >= 0.0f) ? fMinTrackHeight : 0.0f;
        rect.Set(m_rtMinTrack.left, m_rtMinTrack.top + m_rtMinBtn.height,
                 m_rtMinTrack.width, fMinTrackHeight);
      } else {
        FX_FLOAT fMinTrackWidth =
            m_rtMinTrack.width - fW - m_rtMinBtn.width + 2;
        fMinTrackWidth = (fMinTrackWidth >= 0.0f) ? fMinTrackWidth : 0.0f;
        rect.Set(m_rtMinTrack.left + m_rtMinBtn.width - 1, m_rtMinTrack.top,
                 fMinTrackWidth, m_rtMinTrack.height);
      }
    } else {
      if (bVert) {
        FX_FLOAT fMaxTrackHeight = m_rtMaxTrack.height - fH - m_rtMaxBtn.height;
        fMaxTrackHeight = (fMaxTrackHeight >= 0.0f) ? fMaxTrackHeight : 0.0f;
        rect.Set(m_rtMaxTrack.left, m_rtMaxTrack.top + fH, m_rtMaxTrack.width,
                 fMaxTrackHeight);
      } else {
        FX_FLOAT fMaxTrackWidth =
            m_rtMaxTrack.width - fW - m_rtMaxBtn.width + 2;
        fMaxTrackWidth = (fMaxTrackWidth >= 0.0f) ? fMaxTrackWidth : 0.0f;
        rect.Set(m_rtMaxTrack.left + fW, m_rtMaxTrack.top, fMaxTrackWidth,
                 m_rtMaxTrack.height);
      }
    }
  }
}

FX_BOOL IFWL_ScrollBar::SendEvent() {
  if (m_iMinButtonState == CFWL_PartState_Pressed) {
    DoScroll(FWL_SCBCODE_StepBackward, m_fTrackPos);
    return FALSE;
  }
  if (m_iMaxButtonState == CFWL_PartState_Pressed) {
    DoScroll(FWL_SCBCODE_StepForward, m_fTrackPos);
    return FALSE;
  }
  if (m_iMinTrackState == CFWL_PartState_Pressed) {
    DoScroll(FWL_SCBCODE_PageBackward, m_fTrackPos);
    return m_rtThumb.Contains(m_cpTrackPointX, m_cpTrackPointY);
  }
  if (m_iMaxTrackState == CFWL_PartState_Pressed) {
    DoScroll(FWL_SCBCODE_PageForward, m_fTrackPos);
    return m_rtThumb.Contains(m_cpTrackPointX, m_cpTrackPointY);
  }
  if (m_iMouseWheel) {
    uint16_t dwCode =
        m_iMouseWheel < 0 ? FWL_SCBCODE_StepForward : FWL_SCBCODE_StepBackward;
    DoScroll(dwCode, m_fTrackPos);
  }
  return TRUE;
}

FX_BOOL IFWL_ScrollBar::OnScroll(uint32_t dwCode, FX_FLOAT fPos) {
  FX_BOOL bRet = TRUE;
  CFWL_EvtScroll ev;
  ev.m_iScrollCode = dwCode;
  ev.m_pSrcTarget = this;
  ev.m_fPos = fPos;
  ev.m_pRet = &bRet;
  DispatchEvent(&ev);
  return bRet;
}

CFWL_ScrollBarImpDelegate::CFWL_ScrollBarImpDelegate(IFWL_ScrollBar* pOwner)
    : m_pOwner(pOwner) {}

void CFWL_ScrollBarImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return;

  CFWL_MessageType dwMsgCode = pMessage->GetClassID();
  if (dwMsgCode == CFWL_MessageType::Mouse) {
    CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
    switch (pMsg->m_dwCmd) {
      case FWL_MouseCommand::LeftButtonDown: {
        OnLButtonDown(pMsg->m_dwFlags, pMsg->m_fx, pMsg->m_fy);
        break;
      }
      case FWL_MouseCommand::LeftButtonUp: {
        OnLButtonUp(pMsg->m_dwFlags, pMsg->m_fx, pMsg->m_fy);
        break;
      }
      case FWL_MouseCommand::Move: {
        OnMouseMove(pMsg->m_dwFlags, pMsg->m_fx, pMsg->m_fy);
        break;
      }
      case FWL_MouseCommand::Leave: {
        OnMouseLeave();
        break;
      }
      default: { break; }
    }
  } else if (dwMsgCode == CFWL_MessageType::MouseWheel) {
    CFWL_MsgMouseWheel* pMsg = static_cast<CFWL_MsgMouseWheel*>(pMessage);
    OnMouseWheel(pMsg->m_fx, pMsg->m_fy, pMsg->m_dwFlags, pMsg->m_fDeltaX,
                 pMsg->m_fDeltaY);
  }
}

void CFWL_ScrollBarImpDelegate::OnDrawWidget(CFX_Graphics* pGraphics,
                                             const CFX_Matrix* pMatrix) {
  m_pOwner->DrawWidget(pGraphics, pMatrix);
}

void CFWL_ScrollBarImpDelegate::OnLButtonDown(uint32_t dwFlags,
                                              FX_FLOAT fx,
                                              FX_FLOAT fy) {
  if (!m_pOwner->IsEnabled()) {
    return;
  }
  m_pOwner->m_bMouseDown = TRUE;
  m_pOwner->SetGrab(TRUE);
  m_pOwner->m_cpTrackPointX = fx;
  m_pOwner->m_cpTrackPointY = fy;
  m_pOwner->m_fLastTrackPos = m_pOwner->m_fTrackPos;
  if (m_pOwner->m_rtMinBtn.Contains(fx, fy)) {
    DoMouseDown(0, m_pOwner->m_rtMinBtn, m_pOwner->m_iMinButtonState, fx, fy);
  } else {
    if (m_pOwner->m_rtThumb.Contains(fx, fy)) {
      DoMouseDown(1, m_pOwner->m_rtThumb, m_pOwner->m_iThumbButtonState, fx,
                  fy);
    } else {
      if (m_pOwner->m_rtMaxBtn.Contains(fx, fy)) {
        DoMouseDown(2, m_pOwner->m_rtMaxBtn, m_pOwner->m_iMaxButtonState, fx,
                    fy);
      } else {
        if (m_pOwner->m_rtMinTrack.Contains(fx, fy)) {
          DoMouseDown(3, m_pOwner->m_rtMinTrack, m_pOwner->m_iMinTrackState, fx,
                      fy);
        } else {
          DoMouseDown(4, m_pOwner->m_rtMaxTrack, m_pOwner->m_iMaxTrackState, fx,
                      fy);
        }
      }
    }
  }
  if (!m_pOwner->SendEvent())
    m_pOwner->m_pTimerInfo = m_pOwner->StartTimer(FWL_SCROLLBAR_Elapse, true);
}

void CFWL_ScrollBarImpDelegate::OnLButtonUp(uint32_t dwFlags,
                                            FX_FLOAT fx,
                                            FX_FLOAT fy) {
  m_pOwner->m_pTimerInfo->StopTimer();
  m_pOwner->m_bMouseDown = FALSE;
  DoMouseUp(0, m_pOwner->m_rtMinBtn, m_pOwner->m_iMinButtonState, fx, fy);
  DoMouseUp(1, m_pOwner->m_rtThumb, m_pOwner->m_iThumbButtonState, fx, fy);
  DoMouseUp(2, m_pOwner->m_rtMaxBtn, m_pOwner->m_iMaxButtonState, fx, fy);
  DoMouseUp(3, m_pOwner->m_rtMinTrack, m_pOwner->m_iMinTrackState, fx, fy);
  DoMouseUp(4, m_pOwner->m_rtMaxTrack, m_pOwner->m_iMaxTrackState, fx, fy);
  m_pOwner->SetGrab(FALSE);
}

void CFWL_ScrollBarImpDelegate::OnMouseMove(uint32_t dwFlags,
                                            FX_FLOAT fx,
                                            FX_FLOAT fy) {
  DoMouseMove(0, m_pOwner->m_rtMinBtn, m_pOwner->m_iMinButtonState, fx, fy);
  DoMouseMove(1, m_pOwner->m_rtThumb, m_pOwner->m_iThumbButtonState, fx, fy);
  DoMouseMove(2, m_pOwner->m_rtMaxBtn, m_pOwner->m_iMaxButtonState, fx, fy);
  DoMouseMove(3, m_pOwner->m_rtMinTrack, m_pOwner->m_iMinTrackState, fx, fy);
  DoMouseMove(4, m_pOwner->m_rtMaxTrack, m_pOwner->m_iMaxTrackState, fx, fy);
}

void CFWL_ScrollBarImpDelegate::OnMouseLeave() {
  DoMouseLeave(0, m_pOwner->m_rtMinBtn, m_pOwner->m_iMinButtonState);
  DoMouseLeave(1, m_pOwner->m_rtThumb, m_pOwner->m_iThumbButtonState);
  DoMouseLeave(2, m_pOwner->m_rtMaxBtn, m_pOwner->m_iMaxButtonState);
  DoMouseLeave(3, m_pOwner->m_rtMinTrack, m_pOwner->m_iMinTrackState);
  DoMouseLeave(4, m_pOwner->m_rtMaxTrack, m_pOwner->m_iMaxTrackState);
}

void CFWL_ScrollBarImpDelegate::OnMouseWheel(FX_FLOAT fx,
                                             FX_FLOAT fy,
                                             uint32_t dwFlags,
                                             FX_FLOAT fDeltaX,
                                             FX_FLOAT fDeltaY) {
  m_pOwner->m_iMouseWheel = (int32_t)fDeltaX;
  m_pOwner->SendEvent();
  m_pOwner->m_iMouseWheel = 0;
}

void CFWL_ScrollBarImpDelegate::DoMouseDown(int32_t iItem,
                                            const CFX_RectF& rtItem,
                                            int32_t& iState,
                                            FX_FLOAT fx,
                                            FX_FLOAT fy) {
  if (!rtItem.Contains(fx, fy)) {
    return;
  }
  if (iState == CFWL_PartState_Pressed) {
    return;
  }
  iState = CFWL_PartState_Pressed;
  m_pOwner->Repaint(&rtItem);
}

void CFWL_ScrollBarImpDelegate::DoMouseUp(int32_t iItem,
                                          const CFX_RectF& rtItem,
                                          int32_t& iState,
                                          FX_FLOAT fx,
                                          FX_FLOAT fy) {
  int32_t iNewState =
      rtItem.Contains(fx, fy) ? CFWL_PartState_Hovered : CFWL_PartState_Normal;
  if (iState == iNewState) {
    return;
  }
  iState = iNewState;
  m_pOwner->Repaint(&rtItem);
  m_pOwner->OnScroll(FWL_SCBCODE_EndScroll, m_pOwner->m_fTrackPos);
}

void CFWL_ScrollBarImpDelegate::DoMouseMove(int32_t iItem,
                                            const CFX_RectF& rtItem,
                                            int32_t& iState,
                                            FX_FLOAT fx,
                                            FX_FLOAT fy) {
  if (!m_pOwner->m_bMouseDown) {
    int32_t iNewState = rtItem.Contains(fx, fy) ? CFWL_PartState_Hovered
                                                : CFWL_PartState_Normal;
    if (iState == iNewState) {
      return;
    }
    iState = iNewState;
    m_pOwner->Repaint(&rtItem);
  } else if ((2 == iItem) &&
             (m_pOwner->m_iThumbButtonState == CFWL_PartState_Pressed)) {
    FX_FLOAT fPos = m_pOwner->GetTrackPointPos(fx, fy);
    m_pOwner->m_fTrackPos = fPos;
    m_pOwner->OnScroll(FWL_SCBCODE_TrackPos, fPos);
  }
}

void CFWL_ScrollBarImpDelegate::DoMouseLeave(int32_t iItem,
                                             const CFX_RectF& rtItem,
                                             int32_t& iState) {
  if (iState == CFWL_PartState_Normal) {
    return;
  }
  iState = CFWL_PartState_Normal;
  m_pOwner->Repaint(&rtItem);
}

void CFWL_ScrollBarImpDelegate::DoMouseHover(int32_t iItem,
                                             const CFX_RectF& rtItem,
                                             int32_t& iState) {
  if (iState == CFWL_PartState_Hovered) {
    return;
  }
  iState = CFWL_PartState_Hovered;
  m_pOwner->Repaint(&rtItem);
}
