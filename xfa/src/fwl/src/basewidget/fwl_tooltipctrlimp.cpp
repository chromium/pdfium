// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_noteimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_panelimp.h"
#include "xfa/src/fwl/src/core/include/fwl_formimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_tooltipctrlimp.h"

// static
IFWL_ToolTip* IFWL_ToolTip::Create(const CFWL_WidgetImpProperties& properties,
                                   IFWL_Widget* pOuter) {
  IFWL_ToolTip* pToolTip = new IFWL_ToolTip;
  CFWL_ToolTipImp* pToolTipImpl = new CFWL_ToolTipImp(properties, pOuter);
  pToolTip->SetImpl(pToolTipImpl);
  pToolTipImpl->SetInterface(pToolTip);
  return pToolTip;
}
FWL_ERR IFWL_ToolTip::SetAnchor(const CFX_RectF& rtAnchor) {
  return static_cast<CFWL_ToolTipImp*>(GetImpl())->SetAnchor(rtAnchor);
}
FWL_ERR IFWL_ToolTip::Show() {
  return static_cast<CFWL_ToolTipImp*>(GetImpl())->Show();
}
FWL_ERR IFWL_ToolTip::Hide() {
  return static_cast<CFWL_ToolTipImp*>(GetImpl())->Hide();
}
IFWL_ToolTip::IFWL_ToolTip() {
}
CFWL_ToolTipImp::CFWL_ToolTipImp(const CFWL_WidgetImpProperties& properties,
                                 IFWL_Widget* pOuter)
    : CFWL_FormImp(properties, pOuter),
      m_bBtnDown(FALSE),
      m_dwTTOStyles(FDE_TTOSTYLE_SingleLine),
      m_iTTOAlign(FDE_TTOALIGNMENT_Center),
      m_hTimerShow(NULL),
      m_hTimerHide(NULL),
      m_pTimer(NULL) {
  m_rtClient.Set(0, 0, 0, 0);
  m_rtCaption.Set(0, 0, 0, 0);
  m_rtAnchor.Set(0, 0, 0, 0);
  m_TimerShow.m_pToolTip = this;
  m_TimerHide.m_pToolTip = this;
}
CFWL_ToolTipImp::~CFWL_ToolTipImp() {
  if (m_pTimer) {
    delete m_pTimer;
    m_pTimer = NULL;
  }
}
FWL_ERR CFWL_ToolTipImp::GetClassName(CFX_WideString& wsClass) const {
  wsClass = FWL_CLASS_ToolTip;
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_ToolTipImp::GetClassID() const {
  return FWL_CLASSHASH_ToolTip;
}
FWL_ERR CFWL_ToolTipImp::Initialize() {
  m_pProperties->m_dwStyles |= FWL_WGTSTYLE_Popup;
  m_pProperties->m_dwStyles &= ~FWL_WGTSTYLE_Child;
  if (CFWL_WidgetImp::Initialize() != FWL_ERR_Succeeded)
    return FWL_ERR_Indefinite;
  m_pDelegate = new CFWL_ToolTipImpDelegate(this);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ToolTipImp::Finalize() {
  delete m_pDelegate;
  m_pDelegate = nullptr;
  return CFWL_WidgetImp::Finalize();
}
FWL_ERR CFWL_ToolTipImp::GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize) {
  if (bAutoSize) {
    rect.Set(0, 0, 0, 0);
    if (m_pProperties->m_pThemeProvider == NULL) {
      m_pProperties->m_pThemeProvider = GetAvailableTheme();
    }
    CFX_WideString wsCaption;
    IFWL_ToolTipDP* pData =
        static_cast<IFWL_ToolTipDP*>(m_pProperties->m_pDataProvider);
    if (pData) {
      pData->GetCaption(m_pInterface, wsCaption);
    }
    int32_t iLen = wsCaption.GetLength();
    if (iLen > 0) {
      CFX_SizeF sz = CalcTextSize(wsCaption, m_pProperties->m_pThemeProvider);
      rect.Set(0, 0, sz.x, sz.y);
      rect.width += FWL_WGTCAPACITY_CXBorder * 25;
      rect.height += FWL_WGTCAPACITY_CYBorder * 8;
    }
    CFWL_WidgetImp::GetWidgetRect(rect, TRUE);
  } else {
    rect = m_pProperties->m_rtWidget;
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ToolTipImp::Update() {
  if (IsLocked()) {
    return FWL_ERR_Indefinite;
  }
  if (!m_pProperties->m_pThemeProvider) {
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  }
  UpdateTextOutStyles();
  GetClientRect(m_rtClient);
  m_rtCaption = m_rtClient;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ToolTipImp::GetClientRect(CFX_RectF& rect) {
  FX_FLOAT x = 0;
  FX_FLOAT y = 0;
  FX_FLOAT t = 0;
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  if (pTheme) {
    CFWL_ThemePart part;
    part.m_pWidget = m_pInterface;
    x = *static_cast<FX_FLOAT*>(
        pTheme->GetCapacity(&part, FWL_WGTCAPACITY_CXBorder));
    y = *static_cast<FX_FLOAT*>(
        pTheme->GetCapacity(&part, FWL_WGTCAPACITY_CYBorder));
  }
  rect = m_pProperties->m_rtWidget;
  rect.Offset(-rect.left, -rect.top);
  rect.Deflate(x, t, x, y);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ToolTipImp::DrawWidget(CFX_Graphics* pGraphics,
                                    const CFX_Matrix* pMatrix) {
  IFWL_ToolTipTarget* toolTipTarget =
      CFWL_ToolTipContainer::getInstance()->GetCurrentToolTipTarget();
  if (toolTipTarget && !toolTipTarget->UseDefaultTheme()) {
    return toolTipTarget->DrawToolTip(pGraphics, pMatrix, m_pInterface);
  }
  if (!pGraphics)
    return FWL_ERR_Indefinite;
  if (!m_pProperties->m_pThemeProvider)
    return FWL_ERR_Indefinite;
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  DrawBkground(pGraphics, pTheme, pMatrix);
  DrawText(pGraphics, pTheme, pMatrix);
  return FWL_ERR_Succeeded;
}
void CFWL_ToolTipImp::DrawBkground(CFX_Graphics* pGraphics,
                                   IFWL_ThemeProvider* pTheme,
                                   const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground param;
  param.m_pWidget = m_pInterface;
  param.m_iPart = FWL_PART_TTP_Background;
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
void CFWL_ToolTipImp::DrawText(CFX_Graphics* pGraphics,
                               IFWL_ThemeProvider* pTheme,
                               const CFX_Matrix* pMatrix) {
  if (!m_pProperties->m_pDataProvider)
    return;
  CFX_WideString wsCaption;
  m_pProperties->m_pDataProvider->GetCaption(m_pInterface, wsCaption);
  if (wsCaption.IsEmpty()) {
    return;
  }
  CFWL_ThemeText param;
  param.m_pWidget = m_pInterface;
  param.m_iPart = FWL_PART_TTP_Caption;
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
void CFWL_ToolTipImp::UpdateTextOutStyles() {
  m_iTTOAlign = FDE_TTOALIGNMENT_Center;
  m_dwTTOStyles = FDE_TTOSTYLE_SingleLine;
  if (m_pProperties->m_dwStyleExes & FWL_WGTSTYLE_RTLReading) {
    m_dwTTOStyles |= FDE_TTOSTYLE_RTL;
  }
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_TTP_Multiline) {
    m_dwTTOStyles &= ~FDE_TTOSTYLE_SingleLine;
  }
}
FWL_ERR CFWL_ToolTipImp::SetAnchor(const CFX_RectF& rtAnchor) {
  m_rtAnchor = rtAnchor;
  return TRUE;
}
FWL_ERR CFWL_ToolTipImp::Show() {
  IFWL_ToolTipDP* pData =
      static_cast<IFWL_ToolTipDP*>(m_pProperties->m_pDataProvider);
  int32_t nInitDelay = pData->GetInitialDelay(m_pInterface);
  if ((m_pProperties->m_dwStates & FWL_WGTSTATE_Invisible)) {
    m_hTimerShow = FWL_StartTimer(&m_TimerShow, nInitDelay, FALSE);
  }
  return TRUE;
}
FWL_ERR CFWL_ToolTipImp::Hide() {
  SetStates(FWL_WGTSTATE_Invisible, TRUE);
  if (m_hTimerHide) {
    FWL_StopTimer(m_hTimerHide);
    m_hTimerHide = NULL;
  }
  if (m_hTimerShow) {
    FWL_StopTimer(m_hTimerShow);
    m_hTimerShow = NULL;
  }
  return TRUE;
}
FWL_ERR CFWL_ToolTipImp::SetStates(FX_DWORD dwStates, FX_BOOL bSet) {
  if ((dwStates & FWL_WGTSTATE_Invisible) && !bSet) {
    IFWL_ToolTipDP* pData =
        static_cast<IFWL_ToolTipDP*>(m_pProperties->m_pDataProvider);
    int32_t nAutoPopDelay = pData->GetAutoPopDelay(m_pInterface);
    m_hTimerHide = FWL_StartTimer(&m_TimerHide, nAutoPopDelay, FALSE);
  }
  return CFWL_WidgetImp::SetStates(dwStates, bSet);
}
void CFWL_ToolTipImp::RefreshToolTipPos() {
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
CFWL_ToolTipImp::CFWL_ToolTipTimer::CFWL_ToolTipTimer(CFWL_ToolTipImp* pToolTip)
    : m_pToolTip(pToolTip) {}
int32_t CFWL_ToolTipImp::CFWL_ToolTipTimer::Run(FWL_HTIMER hTimer) {
  if (m_pToolTip->m_hTimerShow == hTimer && m_pToolTip->m_hTimerShow) {
    if (m_pToolTip->GetStates() & FWL_WGTSTATE_Invisible) {
      m_pToolTip->SetStates(FWL_WGTSTATE_Invisible, FALSE);
      m_pToolTip->RefreshToolTipPos();
      FWL_StopTimer(m_pToolTip->m_hTimerShow);
      m_pToolTip->m_hTimerShow = NULL;
      return TRUE;
    }
  }
  if (m_pToolTip->m_hTimerHide == hTimer && m_pToolTip->m_hTimerHide) {
    m_pToolTip->SetStates(FWL_WGTSTATE_Invisible, TRUE);
    FWL_StopTimer(m_pToolTip->m_hTimerHide);
    m_pToolTip->m_hTimerHide = NULL;
    return TRUE;
  }
  return TRUE;
}
CFWL_ToolTipImpDelegate::CFWL_ToolTipImpDelegate(CFWL_ToolTipImp* pOwner)
    : m_pOwner(pOwner) {}
int32_t CFWL_ToolTipImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
  return CFWL_WidgetImpDelegate::OnProcessMessage(pMessage);
}
FWL_ERR CFWL_ToolTipImpDelegate::OnProcessEvent(CFWL_Event* pEvent) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ToolTipImpDelegate::OnDrawWidget(CFX_Graphics* pGraphics,
                                              const CFX_Matrix* pMatrix) {
  return m_pOwner->DrawWidget(pGraphics, pMatrix);
}
