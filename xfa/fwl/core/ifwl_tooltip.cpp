// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/ifwl_tooltip.h"

#include "xfa/fde/tto/fde_textout.h"
#include "xfa/fwl/core/cfwl_themebackground.h"
#include "xfa/fwl/core/cfwl_themepart.h"
#include "xfa/fwl/core/cfwl_themetext.h"
#include "xfa/fwl/core/fwl_noteimp.h"
#include "xfa/fwl/core/ifwl_themeprovider.h"
#include "xfa/fwl/core/ifwl_tooltip.h"
#include "xfa/fwl/theme/cfwl_widgettp.h"

IFWL_ToolTip::IFWL_ToolTip(const CFWL_WidgetImpProperties& properties,
                           IFWL_Widget* pOuter)
    : IFWL_Form(properties, pOuter),
      m_bBtnDown(FALSE),
      m_dwTTOStyles(FDE_TTOSTYLE_SingleLine),
      m_iTTOAlign(FDE_TTOALIGNMENT_Center),
      m_pTimerInfoShow(nullptr),
      m_pTimerInfoHide(nullptr) {
  m_rtClient.Set(0, 0, 0, 0);
  m_rtCaption.Set(0, 0, 0, 0);
  m_rtAnchor.Set(0, 0, 0, 0);
  m_TimerShow.m_pToolTip = this;
  m_TimerHide.m_pToolTip = this;
}

IFWL_ToolTip::~IFWL_ToolTip() {}

FWL_Type IFWL_ToolTip::GetClassID() const {
  return FWL_Type::ToolTip;
}

FWL_Error IFWL_ToolTip::Initialize() {
  m_pProperties->m_dwStyles |= FWL_WGTSTYLE_Popup;
  m_pProperties->m_dwStyles &= ~FWL_WGTSTYLE_Child;
  if (IFWL_Widget::Initialize() != FWL_Error::Succeeded)
    return FWL_Error::Indefinite;

  m_pDelegate = new CFWL_ToolTipImpDelegate(this);
  return FWL_Error::Succeeded;
}

void IFWL_ToolTip::Finalize() {
  delete m_pDelegate;
  m_pDelegate = nullptr;
  IFWL_Widget::Finalize();
}

FWL_Error IFWL_ToolTip::GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize) {
  if (bAutoSize) {
    rect.Set(0, 0, 0, 0);
    if (!m_pProperties->m_pThemeProvider) {
      m_pProperties->m_pThemeProvider = GetAvailableTheme();
    }
    CFX_WideString wsCaption;
    IFWL_ToolTipDP* pData =
        static_cast<IFWL_ToolTipDP*>(m_pProperties->m_pDataProvider);
    if (pData) {
      pData->GetCaption(this, wsCaption);
    }
    int32_t iLen = wsCaption.GetLength();
    if (iLen > 0) {
      CFX_SizeF sz = CalcTextSize(wsCaption, m_pProperties->m_pThemeProvider);
      rect.Set(0, 0, sz.x, sz.y);
      rect.width += 25;
      rect.height += 16;
    }
    IFWL_Widget::GetWidgetRect(rect, TRUE);
  } else {
    rect = m_pProperties->m_rtWidget;
  }
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_ToolTip::Update() {
  if (IsLocked()) {
    return FWL_Error::Indefinite;
  }
  if (!m_pProperties->m_pThemeProvider) {
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  }
  UpdateTextOutStyles();
  GetClientRect(m_rtClient);
  m_rtCaption = m_rtClient;
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_ToolTip::GetClientRect(CFX_RectF& rect) {
  FX_FLOAT x = 0;
  FX_FLOAT y = 0;
  FX_FLOAT t = 0;
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  if (pTheme) {
    CFWL_ThemePart part;
    part.m_pWidget = this;
    x = *static_cast<FX_FLOAT*>(
        pTheme->GetCapacity(&part, CFWL_WidgetCapacity::CXBorder));
    y = *static_cast<FX_FLOAT*>(
        pTheme->GetCapacity(&part, CFWL_WidgetCapacity::CYBorder));
  }
  rect = m_pProperties->m_rtWidget;
  rect.Offset(-rect.left, -rect.top);
  rect.Deflate(x, t, x, y);
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_ToolTip::DrawWidget(CFX_Graphics* pGraphics,
                                   const CFX_Matrix* pMatrix) {
  if (!pGraphics || !m_pProperties->m_pThemeProvider)
    return FWL_Error::Indefinite;

  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  DrawBkground(pGraphics, pTheme, pMatrix);
  DrawText(pGraphics, pTheme, pMatrix);
  return FWL_Error::Succeeded;
}

void IFWL_ToolTip::DrawBkground(CFX_Graphics* pGraphics,
                                IFWL_ThemeProvider* pTheme,
                                const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground param;
  param.m_pWidget = this;
  param.m_iPart = CFWL_Part::Background;
  param.m_dwStates = m_pProperties->m_dwStates;
  param.m_pGraphics = pGraphics;
  if (pMatrix) {
    param.m_matrix.Concat(*pMatrix);
  }
  param.m_rtPart = m_rtClient;
  if (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) {
    param.m_pData = &m_rtCaption;
  }
  pTheme->DrawBackground(&param);
}

void IFWL_ToolTip::DrawText(CFX_Graphics* pGraphics,
                            IFWL_ThemeProvider* pTheme,
                            const CFX_Matrix* pMatrix) {
  if (!m_pProperties->m_pDataProvider)
    return;
  CFX_WideString wsCaption;
  m_pProperties->m_pDataProvider->GetCaption(this, wsCaption);
  if (wsCaption.IsEmpty()) {
    return;
  }
  CFWL_ThemeText param;
  param.m_pWidget = this;
  param.m_iPart = CFWL_Part::Caption;
  param.m_dwStates = m_pProperties->m_dwStates;
  param.m_pGraphics = pGraphics;
  if (pMatrix) {
    param.m_matrix.Concat(*pMatrix);
  }
  param.m_rtPart = m_rtCaption;
  param.m_wsText = wsCaption;
  param.m_dwTTOStyles = m_dwTTOStyles;
  param.m_iTTOAlign = m_iTTOAlign;
  pTheme->DrawText(&param);
}

void IFWL_ToolTip::UpdateTextOutStyles() {
  m_iTTOAlign = FDE_TTOALIGNMENT_Center;
  m_dwTTOStyles = FDE_TTOSTYLE_SingleLine;
  if (m_pProperties->m_dwStyleExes & FWL_WGTSTYLE_RTLReading) {
    m_dwTTOStyles |= FDE_TTOSTYLE_RTL;
  }
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_TTP_Multiline) {
    m_dwTTOStyles &= ~FDE_TTOSTYLE_SingleLine;
  }
}

void IFWL_ToolTip::SetAnchor(const CFX_RectF& rtAnchor) {
  m_rtAnchor = rtAnchor;
}

void IFWL_ToolTip::Show() {
  IFWL_ToolTipDP* pData =
      static_cast<IFWL_ToolTipDP*>(m_pProperties->m_pDataProvider);
  int32_t nInitDelay = pData->GetInitialDelay(this);
  if ((m_pProperties->m_dwStates & FWL_WGTSTATE_Invisible))
    m_pTimerInfoShow = m_TimerShow.StartTimer(nInitDelay, false);
}

void IFWL_ToolTip::Hide() {
  SetStates(FWL_WGTSTATE_Invisible, TRUE);
  if (m_pTimerInfoHide) {
    m_pTimerInfoHide->StopTimer();
    m_pTimerInfoHide = nullptr;
  }
  if (m_pTimerInfoShow) {
    m_pTimerInfoShow->StopTimer();
    m_pTimerInfoShow = nullptr;
  }
}

void IFWL_ToolTip::SetStates(uint32_t dwStates, FX_BOOL bSet) {
  if ((dwStates & FWL_WGTSTATE_Invisible) && !bSet) {
    IFWL_ToolTipDP* pData =
        static_cast<IFWL_ToolTipDP*>(m_pProperties->m_pDataProvider);
    int32_t nAutoPopDelay = pData->GetAutoPopDelay(this);
    m_pTimerInfoHide = m_TimerHide.StartTimer(nAutoPopDelay, false);
  }
  IFWL_Widget::SetStates(dwStates, bSet);
}

void IFWL_ToolTip::RefreshToolTipPos() {
  if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_TTP_NoAnchor) == 0) {
    CFX_RectF rtPopup;
    CFX_RectF rtWidget(m_pProperties->m_rtWidget);
    CFX_RectF rtAnchor(m_rtAnchor);
    rtPopup.Set(0, 0, 0, 0);
    FX_FLOAT fx = rtAnchor.Center().x + 20;
    FX_FLOAT fy = rtAnchor.Center().y + 20;
    rtPopup.Set(fx, fy, rtWidget.Width(), rtWidget.Height());
    FX_FLOAT fScreenWidth = 0;
    FX_FLOAT fScreenHeight = 0;
    GetScreenSize(fScreenWidth, fScreenHeight);
    if (rtPopup.bottom() > fScreenHeight) {
      rtPopup.Offset(0, fScreenHeight - rtPopup.bottom());
    }
    if (rtPopup.right() > fScreenWidth) {
      rtPopup.Offset(fScreenWidth - rtPopup.right(), 0);
    }
    if (rtPopup.left < 0) {
      rtPopup.Offset(0 - rtPopup.left, 0);
    }
    if (rtPopup.top < 0) {
      rtPopup.Offset(0, 0 - rtPopup.top);
    }
    SetWidgetRect(rtPopup);
    Update();
  }
}

IFWL_ToolTip::CFWL_ToolTipTimer::CFWL_ToolTipTimer(IFWL_ToolTip* pToolTip)
    : m_pToolTip(pToolTip) {}

void IFWL_ToolTip::CFWL_ToolTipTimer::Run(IFWL_TimerInfo* pTimerInfo) {
  if (m_pToolTip->m_pTimerInfoShow == pTimerInfo &&
      m_pToolTip->m_pTimerInfoShow) {
    if (m_pToolTip->GetStates() & FWL_WGTSTATE_Invisible) {
      m_pToolTip->SetStates(FWL_WGTSTATE_Invisible, FALSE);
      m_pToolTip->RefreshToolTipPos();
      m_pToolTip->m_pTimerInfoShow->StopTimer();
      m_pToolTip->m_pTimerInfoShow = nullptr;
      return;
    }
  }
  if (m_pToolTip->m_pTimerInfoHide == pTimerInfo &&
      m_pToolTip->m_pTimerInfoHide) {
    m_pToolTip->SetStates(FWL_WGTSTATE_Invisible, TRUE);
    m_pToolTip->m_pTimerInfoHide->StopTimer();
    m_pToolTip->m_pTimerInfoHide = nullptr;
  }
}

CFWL_ToolTipImpDelegate::CFWL_ToolTipImpDelegate(IFWL_ToolTip* pOwner)
    : m_pOwner(pOwner) {}

void CFWL_ToolTipImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
  CFWL_WidgetImpDelegate::OnProcessMessage(pMessage);
}

void CFWL_ToolTipImpDelegate::OnProcessEvent(CFWL_Event* pEvent) {}

void CFWL_ToolTipImpDelegate::OnDrawWidget(CFX_Graphics* pGraphics,
                                           const CFX_Matrix* pMatrix) {
  m_pOwner->DrawWidget(pGraphics, pMatrix);
}
