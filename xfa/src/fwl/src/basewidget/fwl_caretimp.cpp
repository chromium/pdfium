// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_noteimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_caretimp.h"

// static
IFWL_Caret* IFWL_Caret::Create(const CFWL_WidgetImpProperties& properties,
                               IFWL_Widget* pOuter) {
  IFWL_Caret* pCaret = new IFWL_Caret;
  CFWL_CaretImp* pCaretImpl = new CFWL_CaretImp(properties, pOuter);
  pCaret->SetImpl(pCaretImpl);
  pCaretImpl->SetInterface(pCaret);
  return pCaret;
}
IFWL_Caret::IFWL_Caret() {}
FWL_ERR IFWL_Caret::ShowCaret(FX_BOOL bFlag) {
  return static_cast<CFWL_CaretImp*>(GetImpl())->ShowCaret(bFlag);
}
FWL_ERR IFWL_Caret::GetFrequency(FX_DWORD& elapse) {
  return static_cast<CFWL_CaretImp*>(GetImpl())->GetFrequency(elapse);
}
FWL_ERR IFWL_Caret::SetFrequency(FX_DWORD elapse) {
  return static_cast<CFWL_CaretImp*>(GetImpl())->SetFrequency(elapse);
}
FWL_ERR IFWL_Caret::SetColor(CFX_Color crFill) {
  return static_cast<CFWL_CaretImp*>(GetImpl())->SetColor(crFill);
}

CFWL_CaretImp::CFWL_CaretImp(const CFWL_WidgetImpProperties& properties,
                             IFWL_Widget* pOuter)
    : CFWL_WidgetImp(properties, pOuter),
      m_hTimer(nullptr),
      m_dwElapse(400),
      m_bSetColor(FALSE) {
  m_pTimer = new CFWL_CaretTimer(this);
  SetStates(FWL_STATE_CAT_HightLight);
}
CFWL_CaretImp::~CFWL_CaretImp() {
  if (m_pTimer) {
    delete m_pTimer;
    m_pTimer = NULL;
  }
}
FWL_ERR CFWL_CaretImp::GetClassName(CFX_WideString& wsClass) const {
  wsClass = FWL_CLASS_Caret;
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_CaretImp::GetClassID() const {
  return FWL_CLASSHASH_Caret;
}
FWL_ERR CFWL_CaretImp::Initialize() {
  if (CFWL_WidgetImp::Initialize() != FWL_ERR_Succeeded)
    return FWL_ERR_Indefinite;
  m_pDelegate = new CFWL_CaretImpDelegate(this);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_CaretImp::Finalize() {
  if (m_hTimer) {
    FWL_StopTimer(m_hTimer);
    m_hTimer = NULL;
  }
  delete m_pDelegate;
  m_pDelegate = nullptr;
  return CFWL_WidgetImp::Finalize();
}
FWL_ERR CFWL_CaretImp::DrawWidget(CFX_Graphics* pGraphics,
                                  const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return FWL_ERR_Indefinite;
  if (!m_pProperties->m_pThemeProvider)
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  if (!m_pProperties->m_pThemeProvider)
    return FWL_ERR_Indefinite;
  DrawCaretBK(pGraphics, m_pProperties->m_pThemeProvider, pMatrix);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_CaretImp::ShowCaret(FX_BOOL bFlag) {
  if (m_hTimer) {
    FWL_StopTimer(m_hTimer);
    m_hTimer = NULL;
  }
  if (bFlag) {
    m_hTimer = FWL_StartTimer(m_pTimer, m_dwElapse);
  }
  return SetStates(FWL_WGTSTATE_Invisible, !bFlag);
}
FWL_ERR CFWL_CaretImp::GetFrequency(FX_DWORD& elapse) {
  elapse = m_dwElapse;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_CaretImp::SetFrequency(FX_DWORD elapse) {
  m_dwElapse = elapse;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_CaretImp::SetColor(CFX_Color crFill) {
  m_bSetColor = TRUE;
  m_crFill = crFill;
  return FWL_ERR_Succeeded;
}
FX_BOOL CFWL_CaretImp::DrawCaretBK(CFX_Graphics* pGraphics,
                                   IFWL_ThemeProvider* pTheme,
                                   const CFX_Matrix* pMatrix) {
  CFX_RectF rect;
  GetWidgetRect(rect);
  rect.Set(0, 0, rect.width, rect.height);
  CFWL_ThemeBackground param;
  param.m_pWidget = m_pInterface;
  param.m_pGraphics = pGraphics;
  param.m_rtPart = rect;
  if (m_bSetColor) {
    param.m_pData = &m_crFill;
  }
  if (!(m_pProperties->m_dwStates & FWL_STATE_CAT_HightLight)) {
    return FWL_ERR_Succeeded;
  }
  param.m_iPart = FWL_PART_CAT_Background;
  param.m_dwStates = FWL_PARTSTATE_CAT_HightLight;
  if (pMatrix) {
    param.m_matrix.Concat(*pMatrix);
  }
  pTheme->DrawBackground(&param);
  return FWL_ERR_Succeeded;
}
CFWL_CaretImp::CFWL_CaretTimer::CFWL_CaretTimer(CFWL_CaretImp* m_pCaret) {
  this->m_pCaret = m_pCaret;
}
int32_t CFWL_CaretImp::CFWL_CaretTimer::Run(FWL_HTIMER hTimer) {
  if (m_pCaret->GetStates() & FWL_STATE_CAT_HightLight) {
    m_pCaret->SetStates(FWL_STATE_CAT_HightLight, FALSE);
  } else {
    m_pCaret->SetStates(FWL_STATE_CAT_HightLight);
  }
  CFX_RectF rt;
  m_pCaret->GetWidgetRect(rt);
  rt.Set(0, 0, rt.width + 1, rt.height);
  m_pCaret->Repaint(&rt);
  return 1;
}
CFWL_CaretImpDelegate::CFWL_CaretImpDelegate(CFWL_CaretImp* pOwner)
    : m_pOwner(pOwner) {}
int32_t CFWL_CaretImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
  return 1;
}
FWL_ERR CFWL_CaretImpDelegate::OnDrawWidget(CFX_Graphics* pGraphics,
                                            const CFX_Matrix* pMatrix) {
  return m_pOwner->DrawWidget(pGraphics, pMatrix);
}
