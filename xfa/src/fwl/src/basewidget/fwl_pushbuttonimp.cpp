// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_noteimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_pushbuttonimp.h"

// static
IFWL_PushButton* IFWL_PushButton::Create(
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter) {
  IFWL_PushButton* pPushButton = new IFWL_PushButton;
  CFWL_PushButtonImp* pPushButtonImpl =
      new CFWL_PushButtonImp(properties, pOuter);
  pPushButton->SetImpl(pPushButtonImpl);
  pPushButtonImpl->SetInterface(pPushButton);
  return pPushButton;
}
IFWL_PushButton::IFWL_PushButton() {
}

CFWL_PushButtonImp::CFWL_PushButtonImp(
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter)
    : CFWL_WidgetImp(properties, pOuter),
      m_bBtnDown(FALSE),
      m_dwTTOStyles(FDE_TTOSTYLE_SingleLine),
      m_iTTOAlign(FDE_TTOALIGNMENT_Center) {
  m_rtClient.Set(0, 0, 0, 0);
  m_rtCaption.Set(0, 0, 0, 0);
}
CFWL_PushButtonImp::~CFWL_PushButtonImp() {}
FWL_ERR CFWL_PushButtonImp::GetClassName(CFX_WideString& wsClass) const {
  wsClass = FWL_CLASS_PushButton;
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_PushButtonImp::GetClassID() const {
  return FWL_CLASSHASH_PushButton;
}
FWL_ERR CFWL_PushButtonImp::Initialize() {
  if (CFWL_WidgetImp::Initialize() != FWL_ERR_Succeeded)
    return FWL_ERR_Indefinite;
  m_pDelegate = new CFWL_PushButtonImpDelegate(this);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PushButtonImp::Finalize() {
  delete m_pDelegate;
  m_pDelegate = nullptr;
  return CFWL_WidgetImp::Finalize();
}
FWL_ERR CFWL_PushButtonImp::GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize) {
  if (bAutoSize) {
    rect.Set(0, 0, 0, 0);
    if (m_pProperties->m_pThemeProvider == NULL) {
      m_pProperties->m_pThemeProvider = GetAvailableTheme();
    }
    CFX_WideString wsCaption;
    IFWL_PushButtonDP* pData =
        static_cast<IFWL_PushButtonDP*>(m_pProperties->m_pDataProvider);
    if (pData) {
      pData->GetCaption(m_pInterface, wsCaption);
    }
    int32_t iLen = wsCaption.GetLength();
    if (iLen > 0) {
      CFX_SizeF sz = CalcTextSize(wsCaption, m_pProperties->m_pThemeProvider);
      rect.Set(0, 0, sz.x, sz.y);
    }
    FX_FLOAT* fcaption =
        static_cast<FX_FLOAT*>(GetThemeCapacity(FWL_WGTCAPACITY_PSB_Margin));
    rect.Inflate(*fcaption, *fcaption);
    CFWL_WidgetImp::GetWidgetRect(rect, TRUE);
  } else {
    rect = m_pProperties->m_rtWidget;
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PushButtonImp::SetStates(FX_DWORD dwStates, FX_BOOL bSet) {
  if ((dwStates & FWL_WGTSTATE_Disabled) && bSet) {
    m_pProperties->m_dwStates = FWL_WGTSTATE_Disabled;
    return FWL_ERR_Succeeded;
  }
  return CFWL_WidgetImp::SetStates(dwStates, bSet);
}
FWL_ERR CFWL_PushButtonImp::Update() {
  if (IsLocked()) {
    return FWL_ERR_Indefinite;
  }
  if (!m_pProperties->m_pThemeProvider) {
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  }
  UpdateTextOutStyles();
  GetClientRect(m_rtClient);
  m_rtCaption = m_rtClient;
  FX_FLOAT* fcaption =
      static_cast<FX_FLOAT*>(GetThemeCapacity(FWL_WGTCAPACITY_PSB_Margin));
  m_rtCaption.Inflate(-*fcaption, -*fcaption);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PushButtonImp::DrawWidget(CFX_Graphics* pGraphics,
                                       const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return FWL_ERR_Indefinite;
  if (!m_pProperties->m_pThemeProvider)
    return FWL_ERR_Indefinite;
  IFWL_PushButtonDP* pData =
      static_cast<IFWL_PushButtonDP*>(m_pProperties->m_pDataProvider);
  CFX_DIBitmap* pPicture = NULL;
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  if (HasBorder()) {
    DrawBorder(pGraphics, FWL_PART_PSB_Border, m_pProperties->m_pThemeProvider,
               pMatrix);
  }
  if (HasEdge()) {
    DrawEdge(pGraphics, FWL_PART_PSB_Edge, m_pProperties->m_pThemeProvider,
             pMatrix);
  }
  DrawBkground(pGraphics, m_pProperties->m_pThemeProvider, pMatrix);
  CFX_Matrix matrix;
  matrix.Concat(*pMatrix);
  FX_FLOAT iPicwidth = 0;
  FX_FLOAT ipicheight = 0;
  CFX_WideString wsCaption;
  if (pData) {
    pData->GetCaption(m_pInterface, wsCaption);
  }
  CFX_RectF rtText;
  rtText.Set(0, 0, 0, 0);
  if (!wsCaption.IsEmpty()) {
    CalcTextRect(wsCaption, pTheme, 0, m_iTTOAlign, rtText);
  }
  switch (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_PSB_ModeMask) {
    case FWL_STYLEEXT_PSB_TextOnly:
      DrawText(pGraphics, m_pProperties->m_pThemeProvider, &matrix);
      break;
    case FWL_STYLEEXT_PSB_IconOnly:
      if (pData) {
        pPicture = pData->GetPicture(m_pInterface);
      }
      if (pPicture) {
        CFX_PointF point;
        switch (m_iTTOAlign) {
          case 0: {
            point.x = m_rtClient.left;
            point.y = m_rtClient.top;
            break;
          }
          case 1: {
            point.x = m_rtClient.left +
                      (m_rtClient.width / 2 - pPicture->GetWidth() / 2);
            point.y = m_rtClient.top;
            break;
          }
          case 2:
            point.x = m_rtClient.left + m_rtClient.width - pPicture->GetWidth();
            point.y = m_rtClient.top;
            break;
          case 4:
            point.x = m_rtClient.left;
            point.y = m_rtClient.top + m_rtClient.height / 2 -
                      pPicture->GetHeight() / 2;
            break;
          case 5:
            point.x = m_rtClient.left +
                      (m_rtClient.width / 2 - pPicture->GetWidth() / 2);
            point.y = m_rtClient.top + m_rtClient.height / 2 -
                      pPicture->GetHeight() / 2;
            break;
          case 6:
            point.x = m_rtClient.left + m_rtClient.width - pPicture->GetWidth();
            point.y = m_rtClient.top + m_rtClient.height / 2 -
                      pPicture->GetHeight() / 2;
            break;
          case 8:
            point.x = m_rtClient.left;
            point.y =
                m_rtClient.top + m_rtClient.height - pPicture->GetHeight();
            break;
          case 9:
            point.x = m_rtClient.left +
                      (m_rtClient.width / 2 - pPicture->GetWidth() / 2);
            point.y =
                m_rtClient.top + m_rtClient.height - pPicture->GetHeight();
            break;
          case 10:
            point.x = m_rtClient.left + m_rtClient.width - pPicture->GetWidth();
            point.y =
                m_rtClient.top + m_rtClient.height - pPicture->GetHeight();
            break;
        }
        pGraphics->DrawImage(pPicture, point, &matrix);
      }
      break;
    case FWL_STYLEEXT_PSB_TextIcon:
      if (pPicture) {
        CFX_PointF point;
        switch (m_iTTOAlign) {
          case 0: {
            point.x = m_rtClient.left;
            point.y = m_rtClient.top;
            iPicwidth = (FX_FLOAT)(pPicture->GetWidth() - 7);
            ipicheight =
                pPicture->GetHeight() / 2 - m_rtCaption.top - rtText.height / 2;
            break;
          }
          case 1: {
            point.x =
                m_rtClient.left + (m_rtClient.width / 2 -
                                   (pPicture->GetWidth() + rtText.width) / 2);
            point.y = m_rtClient.top;
            iPicwidth = pPicture->GetWidth() -
                        ((m_rtClient.width) / 2 - rtText.width / 2 - point.x) +
                        rtText.width / 2 - 7;
            ipicheight =
                pPicture->GetHeight() / 2 - m_rtCaption.top - rtText.height / 2;
            break;
          }
          case 2:
            point.x = m_rtClient.left + m_rtClient.width -
                      pPicture->GetWidth() - rtText.width;
            point.y = m_rtClient.top;
            iPicwidth = m_rtClient.left + m_rtClient.width - point.x -
                        pPicture->GetWidth() - rtText.width + 7;
            ipicheight =
                pPicture->GetHeight() / 2 - m_rtCaption.top - rtText.height / 2;
            break;
          case 4:
            point.x = m_rtClient.left;
            point.y = m_rtClient.top + m_rtClient.height / 2 -
                      pPicture->GetHeight() / 2;
            iPicwidth = m_rtClient.left + pPicture->GetWidth() - 7;
            break;
          case 5:
            point.x =
                m_rtClient.left + (m_rtClient.width / 2 -
                                   (pPicture->GetWidth() + rtText.width) / 2);
            point.y = m_rtClient.top + m_rtClient.height / 2 -
                      pPicture->GetHeight() / 2;
            iPicwidth = pPicture->GetWidth() -
                        ((m_rtClient.width) / 2 - rtText.width / 2 - point.x) +
                        rtText.width / 2 - 7;
            break;
          case 6:
            point.x = m_rtClient.left + m_rtClient.width -
                      pPicture->GetWidth() - rtText.width;
            point.y = m_rtClient.top + m_rtClient.height / 2 -
                      pPicture->GetHeight() / 2;
            iPicwidth = m_rtClient.left + m_rtClient.width - point.x -
                        pPicture->GetWidth() - rtText.width + 7;
            break;
          case 8:
            point.x = m_rtClient.left;
            point.y =
                m_rtClient.top + m_rtClient.height - pPicture->GetHeight();
            iPicwidth = (FX_FLOAT)(pPicture->GetWidth() - 7);
            ipicheight -= rtText.height / 2;
            break;
          case 9:
            point.x =
                m_rtClient.left + (m_rtClient.width / 2 -
                                   (pPicture->GetWidth() + rtText.width) / 2);
            point.y =
                m_rtClient.top + m_rtClient.height - pPicture->GetHeight();
            iPicwidth = pPicture->GetWidth() -
                        ((m_rtClient.width) / 2 - rtText.width / 2 - point.x) +
                        rtText.width / 2 - 7;
            ipicheight -= rtText.height / 2;
            break;
          case 10:
            point.x = m_rtClient.left + m_rtClient.width -
                      pPicture->GetWidth() - rtText.width;
            point.y =
                m_rtClient.top + m_rtClient.height - pPicture->GetHeight();
            iPicwidth = m_rtClient.left + m_rtClient.width - point.x -
                        pPicture->GetWidth() - rtText.width + 7;
            ipicheight -= rtText.height / 2;
            break;
        }
        pGraphics->DrawImage(pPicture, point, &matrix);
      }
      matrix.e += m_rtClient.left + iPicwidth;
      matrix.f += m_rtClient.top + ipicheight;
      DrawText(pGraphics, m_pProperties->m_pThemeProvider, &matrix);
      break;
  }
  return FWL_ERR_Succeeded;
}
void CFWL_PushButtonImp::DrawBkground(CFX_Graphics* pGraphics,
                                      IFWL_ThemeProvider* pTheme,
                                      const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground param;
  param.m_pWidget = m_pInterface;
  param.m_iPart = FWL_PART_PSB_Background;
  param.m_dwStates = GetPartStates();
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
void CFWL_PushButtonImp::DrawText(CFX_Graphics* pGraphics,
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
  param.m_iPart = FWL_PART_PSB_Caption;
  param.m_dwStates = GetPartStates();
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
FX_DWORD CFWL_PushButtonImp::GetPartStates() {
  FX_DWORD dwStates = FWL_PARTSTATE_PSB_Normal;
  if (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) {
    dwStates |= FWL_PARTSTATE_PSB_Focused;
  }
  if (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled) {
    dwStates = FWL_PARTSTATE_PSB_Disabled;
  } else if (m_pProperties->m_dwStates & FWL_STATE_PSB_Pressed) {
    dwStates |= FWL_PARTSTATE_PSB_Pressed;
  } else if (m_pProperties->m_dwStates & FWL_STATE_PSB_Hovered) {
    dwStates |= FWL_PARTSTATE_PSB_Hovered;
  } else if (m_pProperties->m_dwStates & FWL_STATE_PSB_Default) {
    dwStates |= FWL_PARTSTATE_PSB_Default;
  }
  return dwStates;
}
void CFWL_PushButtonImp::UpdateTextOutStyles() {
  m_iTTOAlign = FDE_TTOALIGNMENT_Center;
  switch (m_pProperties->m_dwStyleExes &
          (FWL_STYLEEXT_PSB_HLayoutMask | FWL_STYLEEXT_PSB_VLayoutMask)) {
    case FWL_STYLEEXT_PSB_Left | FWL_STYLEEXT_PSB_Top: {
      m_iTTOAlign = FDE_TTOALIGNMENT_TopLeft;
      break;
    }
    case FWL_STYLEEXT_PSB_Center | FWL_STYLEEXT_PSB_Top: {
      m_iTTOAlign = FDE_TTOALIGNMENT_TopCenter;
      break;
    }
    case FWL_STYLEEXT_PSB_Right | FWL_STYLEEXT_PSB_Top: {
      m_iTTOAlign = FDE_TTOALIGNMENT_TopRight;
      break;
    }
    case FWL_STYLEEXT_PSB_Left | FWL_STYLEEXT_PSB_VCenter: {
      m_iTTOAlign = FDE_TTOALIGNMENT_CenterLeft;
      break;
    }
    case FWL_STYLEEXT_PSB_Center | FWL_STYLEEXT_PSB_VCenter: {
      m_iTTOAlign = FDE_TTOALIGNMENT_Center;
      break;
    }
    case FWL_STYLEEXT_PSB_Right | FWL_STYLEEXT_PSB_VCenter: {
      m_iTTOAlign = FDE_TTOALIGNMENT_CenterRight;
      break;
    }
    case FWL_STYLEEXT_PSB_Left | FWL_STYLEEXT_PSB_Bottom: {
      m_iTTOAlign = FDE_TTOALIGNMENT_BottomLeft;
      break;
    }
    case FWL_STYLEEXT_PSB_Center | FWL_STYLEEXT_PSB_Bottom: {
      m_iTTOAlign = FDE_TTOALIGNMENT_BottomCenter;
      break;
    }
    case FWL_STYLEEXT_PSB_Right | FWL_STYLEEXT_PSB_Bottom: {
      m_iTTOAlign = FDE_TTOALIGNMENT_BottomRight;
      break;
    }
    default: {}
  }
  m_dwTTOStyles = FDE_TTOSTYLE_SingleLine;
  if (m_pProperties->m_dwStyleExes & FWL_WGTSTYLE_RTLReading) {
    m_dwTTOStyles |= FDE_TTOSTYLE_RTL;
  }
}
CFWL_PushButtonImpDelegate::CFWL_PushButtonImpDelegate(
    CFWL_PushButtonImp* pOwner)
    : m_pOwner(pOwner) {}
int32_t CFWL_PushButtonImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return 0;
  if (!m_pOwner->IsEnabled()) {
    return 1;
  }
  int32_t iRet = 1;
  FX_DWORD dwMsgCode = pMessage->GetClassID();
  switch (dwMsgCode) {
    case FWL_MSGHASH_SetFocus:
    case FWL_MSGHASH_KillFocus: {
      OnFocusChanged(pMessage, dwMsgCode == FWL_MSGHASH_SetFocus);
      break;
    }
    case FWL_MSGHASH_Mouse: {
      CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
      FX_DWORD dwCmd = pMsg->m_dwCmd;
      switch (dwCmd) {
        case FWL_MSGMOUSECMD_LButtonDown: {
          OnLButtonDown(pMsg);
          break;
        }
        case FWL_MSGMOUSECMD_LButtonUp: {
          OnLButtonUp(pMsg);
          break;
        }
        case FWL_MSGMOUSECMD_MouseMove: {
          OnMouseMove(pMsg);
          break;
        }
        case FWL_MSGMOUSECMD_MouseLeave: {
          OnMouseLeave(pMsg);
          break;
        }
        default: {}
      }
      break;
    }
    case FWL_MSGHASH_Key: {
      CFWL_MsgKey* pKey = static_cast<CFWL_MsgKey*>(pMessage);
      if (pKey->m_dwCmd == FWL_MSGKEYCMD_KeyDown) {
        OnKeyDown(pKey);
      }
      break;
    }
    default: {
      iRet = 0;
      break;
    }
  }
  CFWL_WidgetImpDelegate::OnProcessMessage(pMessage);
  return iRet;
}
FWL_ERR CFWL_PushButtonImpDelegate::OnProcessEvent(CFWL_Event* pEvent) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PushButtonImpDelegate::OnDrawWidget(CFX_Graphics* pGraphics,
                                                 const CFX_Matrix* pMatrix) {
  return m_pOwner->DrawWidget(pGraphics, pMatrix);
}
void CFWL_PushButtonImpDelegate::OnFocusChanged(CFWL_Message* pMsg,
                                                FX_BOOL bSet) {
  if (bSet) {
    m_pOwner->m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;
  } else {
    m_pOwner->m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Focused;
  }
  m_pOwner->Repaint(&m_pOwner->m_rtClient);
}
void CFWL_PushButtonImpDelegate::OnLButtonDown(CFWL_MsgMouse* pMsg) {
  if ((m_pOwner->m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) == 0) {
    m_pOwner->SetFocus(TRUE);
  }
  m_pOwner->m_bBtnDown = TRUE;
  m_pOwner->m_pProperties->m_dwStates |= FWL_STATE_PSB_Hovered;
  m_pOwner->m_pProperties->m_dwStates |= FWL_STATE_PSB_Pressed;
  m_pOwner->Repaint(&m_pOwner->m_rtClient);
}
void CFWL_PushButtonImpDelegate::OnLButtonUp(CFWL_MsgMouse* pMsg) {
  m_pOwner->m_bBtnDown = FALSE;
  if (m_pOwner->m_rtClient.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_pOwner->m_pProperties->m_dwStates &= ~FWL_STATE_PSB_Pressed;
    m_pOwner->m_pProperties->m_dwStates |= FWL_STATE_PSB_Hovered;
  } else {
    m_pOwner->m_pProperties->m_dwStates &= ~FWL_STATE_PSB_Hovered;
    m_pOwner->m_pProperties->m_dwStates &= ~FWL_STATE_PSB_Pressed;
  }
  if (m_pOwner->m_rtClient.Contains(pMsg->m_fx, pMsg->m_fy)) {
    CFWL_EvtClick wmClick;
    wmClick.m_pSrcTarget = m_pOwner->m_pInterface;
    m_pOwner->DispatchEvent(&wmClick);
  }
  m_pOwner->Repaint(&m_pOwner->m_rtClient);
}
void CFWL_PushButtonImpDelegate::OnMouseMove(CFWL_MsgMouse* pMsg) {
  FX_BOOL bRepaint = FALSE;
  if (m_pOwner->m_bBtnDown) {
    if (m_pOwner->m_rtClient.Contains(pMsg->m_fx, pMsg->m_fy)) {
      if ((m_pOwner->m_pProperties->m_dwStates & FWL_STATE_PSB_Pressed) == 0) {
        m_pOwner->m_pProperties->m_dwStates |= FWL_STATE_PSB_Pressed;
        bRepaint = TRUE;
      }
      if (m_pOwner->m_pProperties->m_dwStates & FWL_STATE_PSB_Hovered) {
        m_pOwner->m_pProperties->m_dwStates &= ~FWL_STATE_PSB_Hovered;
        bRepaint = TRUE;
      }
    } else {
      if (m_pOwner->m_pProperties->m_dwStates & FWL_STATE_PSB_Pressed) {
        m_pOwner->m_pProperties->m_dwStates &= ~FWL_STATE_PSB_Pressed;
        bRepaint = TRUE;
      }
      if ((m_pOwner->m_pProperties->m_dwStates & FWL_STATE_PSB_Hovered) == 0) {
        m_pOwner->m_pProperties->m_dwStates |= FWL_STATE_PSB_Hovered;
        bRepaint = TRUE;
      }
    }
  } else {
    if (!m_pOwner->m_rtClient.Contains(pMsg->m_fx, pMsg->m_fy)) {
      return;
    }
    if ((m_pOwner->m_pProperties->m_dwStates & FWL_STATE_PSB_Hovered) == 0) {
      m_pOwner->m_pProperties->m_dwStates |= FWL_STATE_PSB_Hovered;
      bRepaint = TRUE;
    }
  }
  if (bRepaint) {
    m_pOwner->Repaint(&m_pOwner->m_rtClient);
  }
}
void CFWL_PushButtonImpDelegate::OnMouseLeave(CFWL_MsgMouse* pMsg) {
  m_pOwner->m_bBtnDown = FALSE;
  m_pOwner->m_pProperties->m_dwStates &= ~FWL_STATE_PSB_Hovered;
  m_pOwner->m_pProperties->m_dwStates &= ~FWL_STATE_PSB_Pressed;
  m_pOwner->Repaint(&m_pOwner->m_rtClient);
}
void CFWL_PushButtonImpDelegate::OnKeyDown(CFWL_MsgKey* pMsg) {
  if (pMsg->m_dwKeyCode == FWL_VKEY_Return) {
    CFWL_EvtMouse wmMouse;
    wmMouse.m_pSrcTarget = m_pOwner->m_pInterface;
    wmMouse.m_dwCmd = FWL_MSGMOUSECMD_LButtonUp;
    m_pOwner->DispatchEvent(&wmMouse);
    CFWL_EvtClick wmClick;
    wmClick.m_pSrcTarget = m_pOwner->m_pInterface;
    m_pOwner->DispatchEvent(&wmClick);
    return;
  }
  if (pMsg->m_dwKeyCode != FWL_VKEY_Tab) {
    return;
  }
  m_pOwner->DispatchKeyEvent(pMsg);
}
