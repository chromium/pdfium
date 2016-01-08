// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_noteimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_spinbuttonimp.h"
#define FWL_SPN_MinWidth 18
#define FWL_SPN_MinHeight 32
#define FWL_SPIN_Elapse 200

// static
IFWL_SpinButton* IFWL_SpinButton::Create(
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter) {
  IFWL_SpinButton* pSpinButton = new IFWL_SpinButton;
  CFWL_SpinButtonImp* pSpinButtonImpl =
      new CFWL_SpinButtonImp(properties, nullptr);
  pSpinButton->SetImpl(pSpinButtonImpl);
  pSpinButtonImpl->SetInterface(pSpinButton);
  return pSpinButton;
}
IFWL_SpinButton::IFWL_SpinButton() {}
FWL_ERR IFWL_SpinButton::EnableButton(FX_BOOL bEnable, FX_BOOL bUp) {
  return static_cast<CFWL_SpinButtonImp*>(GetImpl())
      ->EnableButton(bEnable, bUp);
}
FX_BOOL IFWL_SpinButton::IsButtonEnable(FX_BOOL bUp) {
  return static_cast<CFWL_SpinButtonImp*>(GetImpl())->IsButtonEnable(bUp);
}

CFWL_SpinButtonImp::CFWL_SpinButtonImp(
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter)
    : CFWL_WidgetImp(properties, pOuter),
      m_dwUpState(FWL_PARTSTATE_SPB_Normal),
      m_dwDnState(FWL_PARTSTATE_SPB_Normal),
      m_iButtonIndex(0),
      m_bLButtonDwn(FALSE),
      m_hTimer(NULL) {
  m_rtClient.Reset();
  m_rtUpButton.Reset();
  m_rtDnButton.Reset();
  m_pProperties->m_dwStyleExes |= FWL_STYLEEXE_SPB_Vert;
}
CFWL_SpinButtonImp::~CFWL_SpinButtonImp() {}
FWL_ERR CFWL_SpinButtonImp::GetClassName(CFX_WideString& wsClass) const {
  wsClass = FWL_CLASS_SpinButton;
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_SpinButtonImp::GetClassID() const {
  return FWL_CLASSHASH_SpinButton;
}
FWL_ERR CFWL_SpinButtonImp::Initialize() {
  if (CFWL_WidgetImp::Initialize() != FWL_ERR_Succeeded)
    return FWL_ERR_Indefinite;
  m_pDelegate = new CFWL_SpinButtonImpDelegate(this);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SpinButtonImp::Finalize() {
  delete m_pDelegate;
  m_pDelegate = nullptr;
  return CFWL_WidgetImp::Finalize();
}
FWL_ERR CFWL_SpinButtonImp::GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize) {
  if (bAutoSize) {
    rect.Set(0, 0, FWL_SPN_MinWidth, FWL_SPN_MinHeight);
    CFWL_WidgetImp::GetWidgetRect(rect, TRUE);
  } else {
    rect = m_pProperties->m_rtWidget;
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SpinButtonImp::Update() {
  if (IsLocked()) {
    return FWL_ERR_Indefinite;
  }
  GetClientRect(m_rtClient);
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXE_SPB_Vert) {
    m_rtUpButton.Set(m_rtClient.top, m_rtClient.left, m_rtClient.width,
                     m_rtClient.height / 2);
    m_rtDnButton.Set(m_rtClient.left, m_rtClient.top + m_rtClient.height / 2,
                     m_rtClient.width, m_rtClient.height / 2);
  } else {
    m_rtUpButton.Set(m_rtClient.left, m_rtClient.top, m_rtClient.width / 2,
                     m_rtClient.height);
    m_rtDnButton.Set(m_rtClient.left + m_rtClient.width / 2, m_rtClient.top,
                     m_rtClient.width / 2, m_rtClient.height);
  }
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_SpinButtonImp::HitTest(FX_FLOAT fx, FX_FLOAT fy) {
  if (m_rtClient.Contains(fx, fy)) {
    return FWL_WGTHITTEST_Client;
  }
  if (HasBorder() && (m_rtClient.Contains(fx, fy))) {
    return FWL_WGTHITTEST_Border;
  }
  if (HasEdge()) {
    CFX_RectF rtEdge;
    GetEdgeRect(rtEdge);
    if (rtEdge.Contains(fx, fy)) {
      return FWL_PART_SPB_Edge;
    }
  }
  if (m_rtUpButton.Contains(fx, fy)) {
    return FWL_WGTHITTEST_SPB_UpButton;
  }
  if (m_rtDnButton.Contains(fx, fy)) {
    return FWL_WGTHITTEST_SPB_DownButton;
  }
  return FWL_WGTHITTEST_Unknown;
}
FWL_ERR CFWL_SpinButtonImp::DrawWidget(CFX_Graphics* pGraphics,
                                       const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return FWL_ERR_Indefinite;
  CFX_RectF rtClip(m_rtClient);
  if (pMatrix != NULL) {
    pMatrix->TransformRect(rtClip);
  }
  IFWL_ThemeProvider* pTheme = GetAvailableTheme();
  if (HasBorder()) {
    DrawBorder(pGraphics, FWL_PART_SPB_Border, pTheme, pMatrix);
  }
  if (HasEdge()) {
    DrawEdge(pGraphics, FWL_PART_SPB_Edge, pTheme, pMatrix);
  }
  DrawUpButton(pGraphics, pTheme, pMatrix);
  DrawDownButton(pGraphics, pTheme, pMatrix);
  return FWL_ERR_Succeeded;
}
int32_t CFWL_SpinButtonImp::Run(FWL_HTIMER hTimer) {
  if (m_hTimer) {
    CFWL_EvtSpbClick wmPosChanged;
    wmPosChanged.m_pSrcTarget = m_pInterface;
    wmPosChanged.m_bUp = m_iButtonIndex == 0;
    DispatchEvent(&wmPosChanged);
  }
  return 1;
}
FWL_ERR CFWL_SpinButtonImp::EnableButton(FX_BOOL bEnable, FX_BOOL bUp) {
  if (bUp) {
    if (bEnable) {
      m_dwUpState = FWL_PARTSTATE_SPB_Normal;
    } else {
      m_dwUpState = FWL_PARTSTATE_SPB_Disabled;
    }
  } else {
    if (bEnable) {
      m_dwDnState = FWL_PARTSTATE_SPB_Normal;
    } else {
      m_dwDnState = FWL_PARTSTATE_SPB_Disabled;
    }
  }
  return FWL_ERR_Succeeded;
}
FX_BOOL CFWL_SpinButtonImp::IsButtonEnable(FX_BOOL bUp) {
  if (bUp) {
    return (m_dwUpState != FWL_PARTSTATE_SPB_Disabled);
  }
  return (m_dwDnState != FWL_PARTSTATE_SPB_Disabled);
}
void CFWL_SpinButtonImp::DrawUpButton(CFX_Graphics* pGraphics,
                                      IFWL_ThemeProvider* pTheme,
                                      const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground params;
  params.m_pWidget = m_pInterface;
  params.m_iPart = FWL_PART_SPB_UpButton;
  params.m_pGraphics = pGraphics;
  params.m_dwStates = m_dwUpState + 1;
  if (pMatrix) {
    params.m_matrix.Concat(*pMatrix);
  }
  params.m_rtPart = m_rtUpButton;
  pTheme->DrawBackground(&params);
}
void CFWL_SpinButtonImp::DrawDownButton(CFX_Graphics* pGraphics,
                                        IFWL_ThemeProvider* pTheme,
                                        const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground params;
  params.m_pWidget = m_pInterface;
  params.m_iPart = FWL_PART_SPB_DownButton;
  params.m_pGraphics = pGraphics;
  params.m_dwStates = m_dwDnState + 1;
  if (pMatrix) {
    params.m_matrix.Concat(*pMatrix);
  }
  params.m_rtPart = m_rtDnButton;
  pTheme->DrawBackground(&params);
}
CFWL_SpinButtonImpDelegate::CFWL_SpinButtonImpDelegate(
    CFWL_SpinButtonImp* pOwner)
    : m_pOwner(pOwner) {}
int32_t CFWL_SpinButtonImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return 0;
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
FWL_ERR CFWL_SpinButtonImpDelegate::OnProcessEvent(CFWL_Event* pEvent) {
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_SpinButtonImpDelegate::OnDrawWidget(CFX_Graphics* pGraphics,
                                                 const CFX_Matrix* pMatrix) {
  return m_pOwner->DrawWidget(pGraphics, pMatrix);
}
void CFWL_SpinButtonImpDelegate::OnFocusChanged(CFWL_Message* pMsg,
                                                FX_BOOL bSet) {
  if (bSet) {
    m_pOwner->m_pProperties->m_dwStates |= (FWL_WGTSTATE_Focused);
  } else {
    m_pOwner->m_pProperties->m_dwStates &= ~(FWL_WGTSTATE_Focused);
  }
  m_pOwner->Repaint(&m_pOwner->m_rtClient);
}
void CFWL_SpinButtonImpDelegate::OnLButtonDown(CFWL_MsgMouse* pMsg) {
  m_pOwner->m_bLButtonDwn = TRUE;
  m_pOwner->SetGrab(TRUE);
  m_pOwner->SetFocus(TRUE);
  if (!m_pOwner->m_pProperties->m_pDataProvider)
    return;
  FX_BOOL bUpPress = (m_pOwner->m_rtUpButton.Contains(pMsg->m_fx, pMsg->m_fy) &&
                      m_pOwner->IsButtonEnable(TRUE));
  FX_BOOL bDnPress = (m_pOwner->m_rtDnButton.Contains(pMsg->m_fx, pMsg->m_fy) &&
                      m_pOwner->IsButtonEnable(FALSE));
  if (!bUpPress && !bDnPress) {
    return;
  }
  if (bUpPress) {
    m_pOwner->m_iButtonIndex = 0;
    m_pOwner->m_dwUpState = FWL_PARTSTATE_SPB_Pressed;
  }
  if (bDnPress) {
    m_pOwner->m_iButtonIndex = 1;
    m_pOwner->m_dwDnState = FWL_PARTSTATE_SPB_Pressed;
  }
  CFWL_EvtSpbClick wmPosChanged;
  wmPosChanged.m_pSrcTarget = m_pOwner->m_pInterface;
  wmPosChanged.m_bUp = bUpPress;
  m_pOwner->DispatchEvent(&wmPosChanged);
  m_pOwner->Repaint(bUpPress ? &m_pOwner->m_rtUpButton
                             : &m_pOwner->m_rtDnButton);
  m_pOwner->m_hTimer = FWL_StartTimer(m_pOwner, FWL_SPIN_Elapse);
}
void CFWL_SpinButtonImpDelegate::OnLButtonUp(CFWL_MsgMouse* pMsg) {
  if (m_pOwner->m_pProperties->m_dwStates & FWL_PARTSTATE_SPB_Disabled) {
    return;
  }
  m_pOwner->m_bLButtonDwn = FALSE;
  m_pOwner->SetGrab(FALSE);
  m_pOwner->SetFocus(FALSE);
  if (m_pOwner->m_hTimer) {
    FWL_StopTimer(m_pOwner->m_hTimer);
    m_pOwner->m_hTimer = NULL;
  }
  FX_BOOL bRepaint = FALSE;
  CFX_RectF rtInvalidate;
  if (m_pOwner->m_dwUpState == FWL_PARTSTATE_SPB_Pressed &&
      m_pOwner->IsButtonEnable(TRUE)) {
    m_pOwner->m_dwUpState = FWL_PARTSTATE_SPB_Normal;
    bRepaint = TRUE;
    rtInvalidate = m_pOwner->m_rtUpButton;
  } else if (m_pOwner->m_dwDnState == FWL_PARTSTATE_SPB_Pressed &&
             m_pOwner->IsButtonEnable(FALSE)) {
    m_pOwner->m_dwDnState = FWL_PARTSTATE_SPB_Normal;
    bRepaint = TRUE;
    rtInvalidate = m_pOwner->m_rtDnButton;
  }
  if (bRepaint) {
    m_pOwner->Repaint(&rtInvalidate);
  }
}
void CFWL_SpinButtonImpDelegate::OnMouseMove(CFWL_MsgMouse* pMsg) {
  if (!m_pOwner->m_pProperties->m_pDataProvider)
    return;
  if (m_pOwner->m_bLButtonDwn) {
    return;
  }
  FX_BOOL bRepaint = FALSE;
  CFX_RectF rtInvlidate;
  rtInvlidate.Reset();
  if (m_pOwner->m_rtUpButton.Contains(pMsg->m_fx, pMsg->m_fy)) {
    if (m_pOwner->IsButtonEnable(TRUE)) {
      if (m_pOwner->m_dwUpState == FWL_PARTSTATE_SPB_Hovered) {
        m_pOwner->m_dwUpState = FWL_PARTSTATE_SPB_Hovered;
        bRepaint = TRUE;
        rtInvlidate = m_pOwner->m_rtUpButton;
      }
      if (m_pOwner->m_dwDnState != FWL_PARTSTATE_SPB_Normal &&
          m_pOwner->IsButtonEnable(FALSE)) {
        m_pOwner->m_dwDnState = FWL_PARTSTATE_SPB_Normal;
        if (bRepaint) {
          rtInvlidate.Union(m_pOwner->m_rtDnButton);
        } else {
          rtInvlidate = m_pOwner->m_rtDnButton;
        }
        bRepaint = TRUE;
      }
    }
    if (!m_pOwner->IsButtonEnable(FALSE)) {
      m_pOwner->EnableButton(FALSE, FALSE);
    }
  } else if (m_pOwner->m_rtDnButton.Contains(pMsg->m_fx, pMsg->m_fy)) {
    if (m_pOwner->IsButtonEnable(FALSE)) {
      if (m_pOwner->m_dwDnState != FWL_PARTSTATE_SPB_Hovered) {
        m_pOwner->m_dwDnState = FWL_PARTSTATE_SPB_Hovered;
        bRepaint = TRUE;
        rtInvlidate = m_pOwner->m_rtDnButton;
      }
      if (m_pOwner->m_dwUpState != FWL_PARTSTATE_SPB_Normal &&
          m_pOwner->IsButtonEnable(TRUE)) {
        m_pOwner->m_dwUpState = FWL_PARTSTATE_SPB_Normal;
        if (bRepaint) {
          rtInvlidate.Union(m_pOwner->m_rtUpButton);
        } else {
          rtInvlidate = m_pOwner->m_rtUpButton;
        }
        bRepaint = TRUE;
      }
    }
  } else if (m_pOwner->m_dwUpState != FWL_PARTSTATE_SPB_Normal ||
             m_pOwner->m_dwDnState != FWL_PARTSTATE_SPB_Normal) {
    if (m_pOwner->m_dwUpState != FWL_PARTSTATE_SPB_Normal) {
      m_pOwner->m_dwUpState = FWL_PARTSTATE_SPB_Normal;
      bRepaint = TRUE;
      rtInvlidate = m_pOwner->m_rtUpButton;
    }
    if (m_pOwner->m_dwDnState != FWL_PARTSTATE_SPB_Normal) {
      m_pOwner->m_dwDnState = FWL_PARTSTATE_SPB_Normal;
      if (bRepaint) {
        rtInvlidate.Union(m_pOwner->m_rtDnButton);
      } else {
        rtInvlidate = m_pOwner->m_rtDnButton;
      }
      bRepaint = TRUE;
    }
  }
  if (bRepaint) {
    m_pOwner->Repaint(&rtInvlidate);
  }
}
void CFWL_SpinButtonImpDelegate::OnMouseLeave(CFWL_MsgMouse* pMsg) {
  if (!pMsg)
    return;
  if (m_pOwner->m_dwUpState != FWL_PARTSTATE_SPB_Normal &&
      m_pOwner->IsButtonEnable(TRUE)) {
    m_pOwner->m_dwUpState = FWL_PARTSTATE_SPB_Normal;
  }
  if (m_pOwner->m_dwDnState != FWL_PARTSTATE_SPB_Normal &&
      m_pOwner->IsButtonEnable(FALSE)) {
    m_pOwner->m_dwDnState = FWL_PARTSTATE_SPB_Normal;
  }
  m_pOwner->Repaint(&m_pOwner->m_rtClient);
}
void CFWL_SpinButtonImpDelegate::OnKeyDown(CFWL_MsgKey* pMsg) {
  if (!m_pOwner->m_pProperties->m_pDataProvider)
    return;
  FX_BOOL bUp =
      pMsg->m_dwKeyCode == FWL_VKEY_Up || pMsg->m_dwKeyCode == FWL_VKEY_Left;
  FX_BOOL bDown =
      pMsg->m_dwKeyCode == FWL_VKEY_Down || pMsg->m_dwKeyCode == FWL_VKEY_Right;
  if (!bUp && !bDown) {
    return;
  }
  FX_BOOL bUpEnable = m_pOwner->IsButtonEnable(TRUE);
  FX_BOOL bDownEnable = m_pOwner->IsButtonEnable(FALSE);
  if (!bUpEnable && !bDownEnable) {
    return;
  }
  CFWL_EvtSpbClick wmPosChanged;
  wmPosChanged.m_pSrcTarget = m_pOwner->m_pInterface;
  wmPosChanged.m_bUp = bUpEnable;
  m_pOwner->DispatchEvent(&wmPosChanged);
  m_pOwner->Repaint(bUpEnable ? &m_pOwner->m_rtUpButton
                              : &m_pOwner->m_rtDnButton);
}
