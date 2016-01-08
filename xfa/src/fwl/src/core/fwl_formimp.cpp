// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_threadimp.h"
#include "xfa/src/fwl/src/core/include/fwl_noteimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_panelimp.h"
#include "xfa/src/fwl/src/core/include/fwl_formimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetmgrimp.h"
#include "xfa/src/fwl/src/core/include/fwl_appimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_formproxyimp.h"

#define FWL_SYSBTNSIZE 21
#define FWL_SYSBTNMARGIN 5
#define FWL_SYSBTNSPAN 2
#define FWL_CornerEnlarge 10

// static
IFWL_Form* IFWL_Form::CreateFormProxy(CFWL_WidgetImpProperties& properties,
                                      CFX_WideString* classname,
                                      IFWL_Widget* pOuter) {
  IFWL_Form* pForm = new IFWL_Form;
  CFWL_FormProxyImp* pFormProxyImpl = new CFWL_FormProxyImp(properties, pOuter);
  pForm->SetImpl(pFormProxyImpl);
  pFormProxyImpl->SetInterface(pForm);
  return pForm;
}
IFWL_Form::IFWL_Form() {}
FWL_FORMSIZE IFWL_Form::GetFormSize() {
  return static_cast<CFWL_FormImp*>(GetImpl())->GetFormSize();
}
FWL_ERR IFWL_Form::SetFormSize(FWL_FORMSIZE eFormSize) {
  return static_cast<CFWL_FormImp*>(GetImpl())->SetFormSize(eFormSize);
}
IFWL_Widget* IFWL_Form::DoModal() {
  return static_cast<CFWL_FormImp*>(GetImpl())->DoModal();
}
IFWL_Widget* IFWL_Form::DoModal(FX_DWORD& dwCommandID) {
  return static_cast<CFWL_FormImp*>(GetImpl())->DoModal(dwCommandID);
}
FWL_ERR IFWL_Form::EndDoModal() {
  return static_cast<CFWL_FormImp*>(GetImpl())->EndDoModal();
}
FWL_ERR IFWL_Form::SetBorderRegion(CFX_Path* pPath) {
  return static_cast<CFWL_FormImp*>(GetImpl())->SetBorderRegion(pPath);
}

CFWL_FormImp::CFWL_FormImp(const CFWL_WidgetImpProperties& properties,
                           IFWL_Widget* pOuter)
    : CFWL_PanelImp(properties, pOuter),
      m_pCloseBox(NULL),
      m_pMinBox(NULL),
      m_pMaxBox(NULL),
      m_pCaptionBox(NULL),
      m_pNoteLoop(NULL),
      m_pSubFocus(NULL),
      m_fCXBorder(0),
      m_fCYBorder(0),
      m_iCaptureBtn(-1),
      m_iSysBox(0),
      m_eResizeType(FORM_RESIZETYPE_None),
      m_bLButtonDown(FALSE),
      m_bMaximized(FALSE),
      m_bSetMaximize(FALSE),
      m_bCustomizeLayout(FALSE),
      m_eFormSize(FWL_FORMSIZE_Manual),
      m_bDoModalFlag(FALSE),
      m_pBigIcon(NULL),
      m_pSmallIcon(NULL),
      m_bMouseIn(FALSE) {
  m_rtRelative.Reset();
  m_rtCaption.Reset();
  m_rtRestore.Reset();
  m_rtCaptionText.Reset();
  m_rtIcon.Reset();
  m_InfoStart.m_ptStart.Reset();
  m_InfoStart.m_szStart.Reset();
}
CFWL_FormImp::~CFWL_FormImp() {
  RemoveSysButtons();
  if (m_pNoteLoop) {
    delete m_pNoteLoop;
    m_pNoteLoop = NULL;
  }
}
FWL_ERR CFWL_FormImp::GetClassName(CFX_WideString& wsClass) const {
  wsClass = FWL_CLASS_Form;
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_FormImp::GetClassID() const {
  return FWL_CLASSHASH_Form;
}
FX_BOOL CFWL_FormImp::IsInstance(const CFX_WideStringC& wsClass) const {
  if (wsClass == CFX_WideStringC(FWL_CLASS_Form)) {
    return TRUE;
  }
  return CFWL_PanelImp::IsInstance(wsClass);
}
FWL_ERR CFWL_FormImp::Initialize() {
  if (CFWL_WidgetImp::Initialize() != FWL_ERR_Succeeded)
    return FWL_ERR_Indefinite;
  RegisterForm();
  RegisterEventTarget();
  m_pDelegate = new CFWL_FormImpDelegate(this);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_FormImp::Finalize() {
  delete m_pDelegate;
  m_pDelegate = nullptr;
  UnregisterEventTarget();
  UnRegisterForm();
  return CFWL_WidgetImp::Finalize();
}
FWL_ERR CFWL_FormImp::GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize) {
  if (bAutoSize) {
    rect.Reset();
    FX_FLOAT fCapHeight = GetCaptionHeight();
    FX_FLOAT fCXBorder = GetBorderSize(TRUE);
    FX_FLOAT fCYBorder = GetBorderSize(FALSE);
    FX_FLOAT fEdge = GetEdgeWidth();
    if (m_pContent) {
      m_pContent->GetWidgetRect(rect, TRUE);
    }
    rect.height += fCapHeight + fCYBorder + fEdge + fEdge;
    rect.width += fCXBorder + fCXBorder + fEdge + fEdge;
  } else {
    rect = m_pProperties->m_rtWidget;
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_FormImp::GetClientRect(CFX_RectF& rect) {
  if ((m_pProperties->m_dwStyles & FWL_WGTSTYLE_Caption) == 0) {
    rect = m_pProperties->m_rtWidget;
    rect.Offset(-rect.left, -rect.top);
    return FWL_ERR_Succeeded;
  }
#ifdef FWL_UseMacSystemBorder
  rect = m_rtRelative;
  CFWL_WidgetMgr* pWidgetMgr = static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
  if (!pWidgetMgr)
    return FWL_ERR_Indefinite;
  IFWL_AdapterWidgetMgr* adapterWidgetMgr = pWidgetMgr->GetAdapterWidgetMgr();
  FX_FLOAT l, t, r, b;
  l = t = r = b = 0;
  adapterWidgetMgr->GetSystemBorder(l, t, r, b);
  rect.Deflate(l, t, r, b);
  rect.left = rect.top = 0;
  return FWL_ERR_Succeeded;
#else
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
    t = *static_cast<FX_FLOAT*>(
        pTheme->GetCapacity(&part, FWL_WGTCAPACITY_FRM_CYCaption));
  }
  rect = m_pProperties->m_rtWidget;
  rect.Offset(-rect.left, -rect.top);
  rect.Deflate(x, t, x, y);
  return FWL_ERR_Succeeded;
#endif
}
FWL_ERR CFWL_FormImp::Update() {
  if (m_iLock > 0) {
    return FWL_ERR_Succeeded;
  }
  if (!m_pProperties->m_pThemeProvider) {
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  }
#ifdef FWL_UseMacSystemBorder
#else
  SetThemeData();
  if (m_pProperties->m_dwStyles & FWL_WGTSTYLE_Icon) {
    UpdateIcon();
  }
#endif
  UpdateCaption();
  Layout();
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_FormImp::HitTest(FX_FLOAT fx, FX_FLOAT fy) {
  (void)GetAvailableTheme();
  if (m_pCloseBox && m_pCloseBox->m_rtBtn.Contains(fx, fy)) {
    return FWL_WGTHITTEST_CloseBox;
  }
  if (m_pMaxBox && m_pMaxBox->m_rtBtn.Contains(fx, fy)) {
    return FWL_WGTHITTEST_MaxBox;
  }
  if (m_pMinBox && m_pMinBox->m_rtBtn.Contains(fx, fy)) {
    return FWL_WGTHITTEST_MinBox;
  }
  CFX_RectF rtCap;
  rtCap.Set(m_rtCaption.left + m_fCYBorder, m_rtCaption.top + m_fCXBorder,
            m_rtCaption.width - FWL_SYSBTNSIZE * m_iSysBox - 2 * m_fCYBorder,
            m_rtCaption.height - m_fCXBorder);
  if (rtCap.Contains(fx, fy)) {
    return FWL_WGTHITTEST_Titlebar;
  }
  if ((m_pProperties->m_dwStyles & FWL_WGTSTYLE_Border) &&
      (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_FRM_Resize)) {
    FX_FLOAT fWidth =
        m_rtRelative.width - 2 * (m_fCYBorder + FWL_CornerEnlarge);
    FX_FLOAT fHeight =
        m_rtRelative.height - 2 * (m_fCXBorder + FWL_CornerEnlarge);
    CFX_RectF rt;
    rt.Set(0, m_fCXBorder + FWL_CornerEnlarge, m_fCYBorder, fHeight);
    if (rt.Contains(fx, fy)) {
      return FWL_WGTHITTEST_Left;
    }
    rt.Set(m_rtRelative.width - m_fCYBorder, m_fCXBorder + FWL_CornerEnlarge,
           m_fCYBorder, fHeight);
    if (rt.Contains(fx, fy)) {
      return FWL_WGTHITTEST_Right;
    }
    rt.Set(m_fCYBorder + FWL_CornerEnlarge, 0, fWidth, m_fCXBorder);
    if (rt.Contains(fx, fy)) {
      return FWL_WGTHITTEST_Top;
    }
    rt.Set(m_fCYBorder + FWL_CornerEnlarge, m_rtRelative.height - m_fCXBorder,
           fWidth, m_fCXBorder);
    if (rt.Contains(fx, fy)) {
      return FWL_WGTHITTEST_Bottom;
    }
    rt.Set(0, 0, m_fCYBorder + FWL_CornerEnlarge,
           m_fCXBorder + FWL_CornerEnlarge);
    if (rt.Contains(fx, fy)) {
      return FWL_WGTHITTEST_LeftTop;
    }
    rt.Set(0, m_rtRelative.height - m_fCXBorder - FWL_CornerEnlarge,
           m_fCYBorder + FWL_CornerEnlarge, m_fCXBorder + FWL_CornerEnlarge);
    if (rt.Contains(fx, fy)) {
      return FWL_WGTHITTEST_LeftBottom;
    }
    rt.Set(m_rtRelative.width - m_fCYBorder - FWL_CornerEnlarge, 0,
           m_fCYBorder + FWL_CornerEnlarge, m_fCXBorder + FWL_CornerEnlarge);
    if (rt.Contains(fx, fy)) {
      return FWL_WGTHITTEST_RightTop;
    }
    rt.Set(m_rtRelative.width - m_fCYBorder - FWL_CornerEnlarge,
           m_rtRelative.height - m_fCXBorder - FWL_CornerEnlarge,
           m_fCYBorder + FWL_CornerEnlarge, m_fCXBorder + FWL_CornerEnlarge);
    if (rt.Contains(fx, fy)) {
      return FWL_WGTHITTEST_RightBottom;
    }
  }
  return FWL_WGTHITTEST_Client;
}
FWL_ERR CFWL_FormImp::DrawWidget(CFX_Graphics* pGraphics,
                                 const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return FWL_ERR_Indefinite;
  if (!m_pProperties->m_pThemeProvider)
    return FWL_ERR_Indefinite;
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  FX_BOOL bInactive = !IsActive();
  int32_t iState =
      bInactive ? FWL_PARTSTATE_FRM_Inactive : FWL_PARTSTATE_FRM_Normal;
  if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_FRM_NoDrawClient) == 0) {
    DrawBackground(pGraphics, pTheme);
  }
#ifdef FWL_UseMacSystemBorder
  return FWL_ERR_Succeeded;
#endif
  CFWL_ThemeBackground param;
  param.m_pWidget = m_pInterface;
  param.m_dwStates = iState;
  param.m_pGraphics = pGraphics;
  param.m_rtPart = m_rtRelative;
  if (pMatrix) {
    param.m_matrix.Concat(*pMatrix);
  }
  if (m_pProperties->m_dwStyles & FWL_WGTSTYLE_Border) {
    param.m_iPart = FWL_PART_FRM_Border;
    pTheme->DrawBackground(&param);
  }
  if ((m_pProperties->m_dwStyleExes & FWL_WGTSTYLE_EdgeMask) !=
      FWL_WGTSTYLE_EdgeNone) {
    CFX_RectF rtEdge;
    GetEdgeRect(rtEdge);
    param.m_iPart = FWL_PART_FRM_Edge;
    param.m_rtPart = rtEdge;
    param.m_dwStates = iState;
    pTheme->DrawBackground(&param);
  }
  if (m_pProperties->m_dwStyles & FWL_WGTSTYLE_Caption) {
    param.m_iPart = FWL_PART_FRM_Caption;
    param.m_dwStates = iState;
    param.m_rtPart = m_rtCaption;
    pTheme->DrawBackground(&param);
    DrawCaptionText(pGraphics, pTheme, pMatrix);
  } else if (m_pProperties->m_dwStyles & FWL_WGTSTYLE_NarrowCaption) {
    param.m_iPart = FWL_PART_FRM_NarrowCaption;
    param.m_dwStates = iState;
    param.m_rtPart = m_rtCaption;
    pTheme->DrawBackground(&param);
    DrawCaptionText(pGraphics, pTheme, pMatrix);
  }
  if (m_pProperties->m_dwStyles & FWL_WGTSTYLE_Icon) {
    param.m_iPart = FWL_PART_FRM_Icon;
    if (HasIcon()) {
      DrawIconImage(pGraphics, pTheme, pMatrix);
    }
  }
#if (_FX_OS_ == _FX_MACOSX_)
  {
    if (m_pCloseBox) {
      param.m_iPart = FWL_PART_FRM_CloseBox;
      param.m_dwStates = m_pCloseBox->GetPartState();
      if (m_pProperties->m_dwStates & FWL_WGTSTATE_Deactivated) {
        param.m_dwStates = FWL_PARTSTATE_FRM_Disabled;
      } else if (FWL_PARTSTATE_FRM_Normal == param.m_dwStates && m_bMouseIn) {
        param.m_dwStates = FWL_PARTSTATE_FRM_Hover;
      }
      param.m_rtPart = m_pCloseBox->m_rtBtn;
      pTheme->DrawBackground(&param);
    }
    if (m_pMaxBox) {
      param.m_iPart = FWL_PART_FRM_MaximizeBox;
      param.m_dwStates = m_pMaxBox->GetPartState();
      if (m_pProperties->m_dwStates & FWL_WGTSTATE_Deactivated) {
        param.m_dwStates = FWL_PARTSTATE_FRM_Disabled;
      } else if (FWL_PARTSTATE_FRM_Normal == param.m_dwStates && m_bMouseIn) {
        param.m_dwStates = FWL_PARTSTATE_FRM_Hover;
      }
      param.m_rtPart = m_pMaxBox->m_rtBtn;
      param.m_dwData = m_bMaximized;
      pTheme->DrawBackground(&param);
    }
    if (m_pMinBox) {
      param.m_iPart = FWL_PART_FRM_MinimizeBox;
      param.m_dwStates = m_pMinBox->GetPartState();
      if (m_pProperties->m_dwStates & FWL_WGTSTATE_Deactivated) {
        param.m_dwStates = FWL_PARTSTATE_FRM_Disabled;
      } else if (FWL_PARTSTATE_FRM_Normal == param.m_dwStates && m_bMouseIn) {
        param.m_dwStates = FWL_PARTSTATE_FRM_Hover;
      }
      param.m_rtPart = m_pMinBox->m_rtBtn;
      pTheme->DrawBackground(&param);
    }
    m_bMouseIn = FALSE;
  }
#else
  {
    if (m_pCloseBox) {
      param.m_iPart = FWL_PART_FRM_CloseBox;
      param.m_dwStates = m_pCloseBox->GetPartState();
      param.m_rtPart = m_pCloseBox->m_rtBtn;
      pTheme->DrawBackground(&param);
    }
    if (m_pMaxBox) {
      param.m_iPart = FWL_PART_FRM_MaximizeBox;
      param.m_dwStates = m_pMaxBox->GetPartState();
      param.m_rtPart = m_pMaxBox->m_rtBtn;
      param.m_dwData = m_bMaximized;
      pTheme->DrawBackground(&param);
    }
    if (m_pMinBox) {
      param.m_iPart = FWL_PART_FRM_MinimizeBox;
      param.m_dwStates = m_pMinBox->GetPartState();
      param.m_rtPart = m_pMinBox->m_rtBtn;
      pTheme->DrawBackground(&param);
    }
  }
#endif
  return FWL_ERR_Succeeded;
}
FWL_FORMSIZE CFWL_FormImp::GetFormSize() {
  return m_eFormSize;
}
FWL_ERR CFWL_FormImp::SetFormSize(FWL_FORMSIZE eFormSize) {
  m_eFormSize = eFormSize;
  return FWL_ERR_Succeeded;
}
IFWL_Widget* CFWL_FormImp::DoModal() {
  IFWL_NoteThread* pThread = GetOwnerThread();
  if (!pThread)
    return NULL;
  IFWL_NoteDriver* pDriver = pThread->GetNoteDriver();
  if (!pDriver)
    return NULL;
  m_pNoteLoop = new CFWL_NoteLoop(this);
  pDriver->PushNoteLoop(m_pNoteLoop);
  m_bDoModalFlag = TRUE;
  SetStates(FWL_WGTSTATE_Invisible, FALSE);
  pDriver->Run();
#if (_FX_OS_ == _FX_MACOSX_)
#else
  pDriver->PopNoteLoop();
#endif
  delete m_pNoteLoop;
  m_pNoteLoop = NULL;
  return NULL;
}
IFWL_Widget* CFWL_FormImp::DoModal(FX_DWORD& dwCommandID) {
  return DoModal();
}
FWL_ERR CFWL_FormImp::EndDoModal() {
  if (!m_pNoteLoop)
    return FWL_ERR_Indefinite;
  m_bDoModalFlag = FALSE;
#if (_FX_OS_ == _FX_MACOSX_)
  m_pNoteLoop->EndModalLoop();
  IFWL_NoteThread* pThread = GetOwnerThread();
  if (!pThread)
    return NULL;
  CFWL_NoteDriver* pDriver =
      static_cast<CFWL_NoteDriver*>(pThread->GetNoteDriver());
  if (!pDriver)
    return NULL;
  pDriver->PopNoteLoop();
  SetStates(FWL_WGTSTATE_Invisible, TRUE);
  return FWL_ERR_Succeeded;
#else
  SetStates(FWL_WGTSTATE_Invisible, TRUE);
  return m_pNoteLoop->EndModalLoop();
#endif
}
FWL_ERR CFWL_FormImp::SetBorderRegion(CFX_Path* pPath) {
  return FWL_ERR_Succeeded;
}
void CFWL_FormImp::DrawBackground(CFX_Graphics* pGraphics,
                                  IFWL_ThemeProvider* pTheme) {
  CFWL_ThemeBackground param;
  param.m_pWidget = m_pInterface;
  param.m_iPart = FWL_PART_FRM_Background;
  param.m_pGraphics = pGraphics;
  param.m_rtPart = m_rtRelative;
  param.m_rtPart.Deflate(m_fCYBorder, m_rtCaption.height, m_fCYBorder,
                         m_fCXBorder);
  pTheme->DrawBackground(&param);
}
CFWL_WidgetImp* CFWL_FormImp::GetSubFocus() {
  return m_pSubFocus;
}
void CFWL_FormImp::SetSubFocus(CFWL_WidgetImp* pWidget) {
  m_pSubFocus = pWidget;
}
CFX_MapAccelerators& CFWL_FormImp::GetAccelerator() {
  return m_mapAccelerators;
}
void CFWL_FormImp::SetAccelerator(CFX_MapAccelerators* pAccelerators) {
  if (!pAccelerators)
    return;
  m_mapAccelerators.RemoveAll();
  FX_DWORD vrKey, rValue;
  FX_POSITION pos = pAccelerators->GetStartPosition();
  while (pos) {
    pAccelerators->GetNextAssoc(pos, vrKey, rValue);
    m_mapAccelerators.SetAt(vrKey, rValue);
  }
}
void CFWL_FormImp::ShowChildWidget(IFWL_Widget* pParent) {
  IFWL_App* pApp = FWL_GetApp();
  if (!pApp)
    return;
  CFWL_WidgetMgr* pWidgetMgr =
      static_cast<CFWL_WidgetMgr*>(pApp->GetWidgetMgr());
  if (!pWidgetMgr)
    return;
  IFWL_Widget* pChild =
      pWidgetMgr->GetWidget(pParent, FWL_WGTRELATION_FirstChild);
  while (pChild) {
    pWidgetMgr->ShowWidget_Native(pChild);
    ShowChildWidget(pChild);
    pChild = pWidgetMgr->GetWidget(pChild, FWL_WGTRELATION_NextSibling);
  }
}
void CFWL_FormImp::RemoveSysButtons() {
  m_rtCaption.Reset();
  if (m_pCloseBox) {
    delete m_pCloseBox;
    m_pCloseBox = NULL;
  }
  if (m_pMinBox) {
    delete m_pMinBox;
    m_pMinBox = NULL;
  }
  if (m_pMaxBox) {
    delete m_pMaxBox;
    m_pMaxBox = NULL;
  }
  if (m_pCaptionBox) {
    delete m_pCaptionBox;
    m_pCaptionBox = NULL;
  }
}
void CFWL_FormImp::CalcContentRect(CFX_RectF& rtContent) {
#ifdef FWL_UseMacSystemBorder
  rtContent = m_rtRelative;
#else
  GetEdgeRect(rtContent);
  if (HasEdge()) {
    FX_FLOAT fEdge = GetEdgeWidth();
    rtContent.Deflate(fEdge, fEdge);
  }
#endif
}
CFWL_SysBtn* CFWL_FormImp::GetSysBtnAtPoint(FX_FLOAT fx, FX_FLOAT fy) {
  if (m_pCloseBox && m_pCloseBox->m_rtBtn.Contains(fx, fy)) {
    return m_pCloseBox;
  }
  if (m_pMaxBox && m_pMaxBox->m_rtBtn.Contains(fx, fy)) {
    return m_pMaxBox;
  }
  if (m_pMinBox && m_pMinBox->m_rtBtn.Contains(fx, fy)) {
    return m_pMinBox;
  }
  if (m_pCaptionBox && m_pCaptionBox->m_rtBtn.Contains(fx, fy)) {
    return m_pCaptionBox;
  }
  return NULL;
}
CFWL_SysBtn* CFWL_FormImp::GetSysBtnByState(FX_DWORD dwState) {
  if (m_pCloseBox && (m_pCloseBox->m_dwState & dwState)) {
    return m_pCloseBox;
  }
  if (m_pMaxBox && (m_pMaxBox->m_dwState & dwState)) {
    return m_pMaxBox;
  }
  if (m_pMinBox && (m_pMinBox->m_dwState & dwState)) {
    return m_pMinBox;
  }
  if (m_pCaptionBox && (m_pCaptionBox->m_dwState & dwState)) {
    return m_pCaptionBox;
  }
  return NULL;
}
CFWL_SysBtn* CFWL_FormImp::GetSysBtnByIndex(int32_t nIndex) {
  if (nIndex < 0) {
    return NULL;
  }
  CFX_PtrArray arrBtn;
  if (m_pMinBox) {
    arrBtn.Add(m_pMinBox);
  }
  if (m_pMaxBox) {
    arrBtn.Add(m_pMaxBox);
  }
  if (m_pCloseBox) {
    arrBtn.Add(m_pCloseBox);
  }
  return static_cast<CFWL_SysBtn*>(arrBtn[nIndex]);
}
int32_t CFWL_FormImp::GetSysBtnIndex(CFWL_SysBtn* pBtn) {
  CFX_PtrArray arrBtn;
  if (m_pMinBox) {
    arrBtn.Add(m_pMinBox);
  }
  if (m_pMaxBox) {
    arrBtn.Add(m_pMaxBox);
  }
  if (m_pCloseBox) {
    arrBtn.Add(m_pCloseBox);
  }
  return arrBtn.Find(pBtn);
}
FX_FLOAT CFWL_FormImp::GetCaptionHeight() {
  FX_DWORD dwCapacity = 0;
  if (m_pProperties->m_dwStyles & FWL_WGTSTYLE_Caption) {
    dwCapacity = FWL_WGTCAPACITY_FRM_CYCaption;
  } else if (m_pProperties->m_dwStyles & FWL_WGTSTYLE_NarrowCaption) {
    dwCapacity = FWL_WGTCAPACITY_FRM_CYNarrowCaption;
  }
  if (dwCapacity > 0) {
    FX_FLOAT* pfCapHeight =
        static_cast<FX_FLOAT*>(GetThemeCapacity(dwCapacity));
    return pfCapHeight ? *pfCapHeight : 0;
  }
  return 0;
}
void CFWL_FormImp::DrawCaptionText(CFX_Graphics* pGs,
                                   IFWL_ThemeProvider* pTheme,
                                   const CFX_Matrix* pMatrix) {
  CFX_WideString wsText;
  IFWL_DataProvider* pData = m_pProperties->m_pDataProvider;
  pData->GetCaption(m_pInterface, wsText);
  if (wsText.IsEmpty()) {
    return;
  }
  CFWL_ThemeText textParam;
  textParam.m_pWidget = m_pInterface;
  textParam.m_iPart = FWL_PART_FRM_Caption;
  textParam.m_dwStates = FWL_PARTSTATE_FRM_Normal;
  textParam.m_pGraphics = pGs;
  if (pMatrix) {
    textParam.m_matrix.Concat(*pMatrix);
  }
  CFX_RectF rtText;
  if (m_bCustomizeLayout) {
    rtText = m_rtCaptionText;
    rtText.top -= 5;
  } else {
    rtText = m_rtCaption;
    FX_FLOAT fpos;
    fpos = HasIcon() ? 29.0f : 13.0f;
    rtText.left += fpos;
  }
  textParam.m_rtPart = rtText;
  textParam.m_wsText = wsText;
  textParam.m_dwTTOStyles = FDE_TTOSTYLE_SingleLine | FDE_TTOSTYLE_Ellipsis;
  textParam.m_iTTOAlign = m_bCustomizeLayout ? FDE_TTOALIGNMENT_Center
                                             : FDE_TTOALIGNMENT_CenterLeft;
  pTheme->DrawText(&textParam);
}
void CFWL_FormImp::DrawIconImage(CFX_Graphics* pGs,
                                 IFWL_ThemeProvider* pTheme,
                                 const CFX_Matrix* pMatrix) {
  IFWL_FormDP* pData =
      static_cast<IFWL_FormDP*>(m_pProperties->m_pDataProvider);
  CFWL_ThemeBackground param;
  param.m_pWidget = m_pInterface;
  param.m_iPart = FWL_PART_FRM_Icon;
  param.m_pGraphics = pGs;
  param.m_pImage = pData->GetIcon(m_pInterface, FALSE);
  param.m_rtPart = m_rtIcon;
  if (pMatrix) {
    param.m_matrix.Concat(*pMatrix);
  }
  pTheme->DrawBackground(&param);
}
void CFWL_FormImp::GetEdgeRect(CFX_RectF& rtEdge) {
  rtEdge = m_rtRelative;
  if (m_pProperties->m_dwStyles & FWL_WGTSTYLE_Border) {
    FX_FLOAT fCX = GetBorderSize();
    FX_FLOAT fCY = GetBorderSize(FALSE);
    rtEdge.Deflate(fCX, m_rtCaption.Height(), fCX, fCY);
  }
}
void CFWL_FormImp::SetWorkAreaRect() {
  m_rtRestore = m_pProperties->m_rtWidget;
  CFWL_WidgetMgr* pWidgetMgr = static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
  if (!pWidgetMgr)
    return;
  m_bSetMaximize = TRUE;
  pWidgetMgr->SetMaximize_Native(m_pInterface);
  Repaint(&m_rtRelative);
}
void CFWL_FormImp::SetCursor(FX_FLOAT fx, FX_FLOAT fy) {
  IFWL_AdapterNative* pNative = FWL_GetAdapterNative();
  IFWL_AdapterCursorMgr* pCursorMgr = pNative->GetCursorMgr();
  if (!pCursorMgr)
    return;
  FX_DWORD dwHitTest = HitTest(fx, fy);
  switch (dwHitTest) {
    case FWL_WGTHITTEST_Right: {
      FWL_HCURSOR hCursor = pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_SizeWE);
      pCursorMgr->SetCursor(hCursor);
      pCursorMgr->ShowCursor(TRUE);
      m_eResizeType = FORM_RESIZETYPE_Right;
      break;
    }
    case FWL_WGTHITTEST_Bottom: {
      FWL_HCURSOR hCursor = pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_SizeNS);
      pCursorMgr->SetCursor(hCursor);
      pCursorMgr->ShowCursor(TRUE);
      m_eResizeType = FORM_RESIZETYPE_Bottom;
      break;
    }
    case FWL_WGTHITTEST_Left: {
      FWL_HCURSOR hCursor = pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_SizeWE);
      pCursorMgr->SetCursor(hCursor);
      pCursorMgr->ShowCursor(TRUE);
      m_eResizeType = FORM_RESIZETYPE_Left;
      break;
    }
    case FWL_WGTHITTEST_Top: {
      FWL_HCURSOR hCursor = pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_SizeNS);
      pCursorMgr->SetCursor(hCursor);
      pCursorMgr->ShowCursor(TRUE);
      m_eResizeType = FORM_RESIZETYPE_Top;
      break;
    }
    case FWL_WGTHITTEST_LeftTop: {
      FWL_HCURSOR hCursor =
          pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_SizeNWSE);
      pCursorMgr->SetCursor(hCursor);
      pCursorMgr->ShowCursor(TRUE);
      m_eResizeType = FORM_RESIZETYPE_LeftTop;
      break;
    }
    case FWL_WGTHITTEST_LeftBottom: {
      FWL_HCURSOR hCursor =
          pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_SizeNESW);
      pCursorMgr->SetCursor(hCursor);
      pCursorMgr->ShowCursor(TRUE);
      m_eResizeType = FORM_RESIZETYPE_LeftBottom;
      break;
    }
    case FWL_WGTHITTEST_RightTop: {
      FWL_HCURSOR hCursor =
          pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_SizeNESW);
      pCursorMgr->SetCursor(hCursor);
      pCursorMgr->ShowCursor(TRUE);
      m_eResizeType = FORM_RESIZETYPE_RightTop;
      break;
    }
    case FWL_WGTHITTEST_RightBottom: {
      FWL_HCURSOR hCursor =
          pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_SizeNWSE);
      pCursorMgr->SetCursor(hCursor);
      pCursorMgr->ShowCursor(TRUE);
      m_eResizeType = FORM_RESIZETYPE_RightBottom;
      break;
    }
    default: {
      FWL_HCURSOR hCursor = pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_Arrow);
      pCursorMgr->SetCursor(hCursor);
      pCursorMgr->ShowCursor(TRUE);
    }
  }
}
void CFWL_FormImp::Layout() {
  GetRelativeRect(m_rtRelative);
#ifndef FWL_UseMacSystemBorder
  ReSetSysBtn();
#endif
  if (m_pContent) {
    CFX_RectF rtClient;
    GetClientRect(rtClient);
    m_pContent->SetWidgetRect(rtClient);
    m_pContent->Update();
  }
}
void CFWL_FormImp::ReSetSysBtn() {
  m_fCXBorder =
      *static_cast<FX_FLOAT*>(GetThemeCapacity(FWL_WGTCAPACITY_CXBorder));
  m_fCYBorder =
      *static_cast<FX_FLOAT*>(GetThemeCapacity(FWL_WGTCAPACITY_CYBorder));
  RemoveSysButtons();
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  m_bCustomizeLayout = pTheme->IsCustomizedLayout(m_pInterface);
  FX_FLOAT fCapHeight = GetCaptionHeight();
  if (fCapHeight > 0) {
    m_rtCaption = m_rtRelative;
    m_rtCaption.height = fCapHeight;
  }
  m_iSysBox = 0;
  if (m_pProperties->m_dwStyles & FWL_WGTSTYLE_CloseBox) {
    m_pCloseBox = new CFWL_SysBtn;
    if (m_bCustomizeLayout) {
      CFWL_ThemeBackground param;
      param.m_pWidget = m_pInterface;
      param.m_iPart = FWL_PART_FRM_CloseBox;
      pTheme->GetPartRect(&param, m_pCloseBox->m_rtBtn);
    } else {
      m_pCloseBox->m_rtBtn.Set(
          m_rtRelative.right() - FWL_SYSBTNMARGIN - FWL_SYSBTNSIZE,
          FWL_SYSBTNMARGIN, FWL_SYSBTNSIZE, FWL_SYSBTNSIZE);
    }
    m_iSysBox++;
  }
  if (m_pProperties->m_dwStyles & FWL_WGTSTYLE_MaximizeBox) {
    m_pMaxBox = new CFWL_SysBtn;
    if (m_bCustomizeLayout) {
      CFWL_ThemeBackground param;
      param.m_pWidget = m_pInterface;
      param.m_iPart = FWL_PART_FRM_MaximizeBox;
      pTheme->GetPartRect(&param, m_pMaxBox->m_rtBtn);
    } else {
      if (m_pCloseBox) {
        m_pMaxBox->m_rtBtn.Set(
            m_pCloseBox->m_rtBtn.left - FWL_SYSBTNSPAN - FWL_SYSBTNSIZE,
            m_pCloseBox->m_rtBtn.top, FWL_SYSBTNSIZE, FWL_SYSBTNSIZE);
      } else {
        m_pMaxBox->m_rtBtn.Set(
            m_rtRelative.right() - FWL_SYSBTNMARGIN - FWL_SYSBTNSIZE,
            FWL_SYSBTNMARGIN, FWL_SYSBTNSIZE, FWL_SYSBTNSIZE);
      }
    }
    m_iSysBox++;
  }
  if (m_pProperties->m_dwStyles & FWL_WGTSTYLE_MinimizeBox) {
    m_pMinBox = new CFWL_SysBtn;
    if (m_bCustomizeLayout) {
      CFWL_ThemeBackground param;
      param.m_pWidget = m_pInterface;
      param.m_iPart = FWL_PART_FRM_MinimizeBox;
      pTheme->GetPartRect(&param, m_pMinBox->m_rtBtn);
    } else {
      if (m_pMaxBox) {
        m_pMinBox->m_rtBtn.Set(
            m_pMaxBox->m_rtBtn.left - FWL_SYSBTNSPAN - FWL_SYSBTNSIZE,
            m_pMaxBox->m_rtBtn.top, FWL_SYSBTNSIZE, FWL_SYSBTNSIZE);
      } else if (m_pCloseBox) {
        m_pMinBox->m_rtBtn.Set(
            m_pCloseBox->m_rtBtn.left - FWL_SYSBTNSPAN - FWL_SYSBTNSIZE,
            m_pCloseBox->m_rtBtn.top, FWL_SYSBTNSIZE, FWL_SYSBTNSIZE);
      } else {
        m_pMinBox->m_rtBtn.Set(
            m_rtRelative.right() - FWL_SYSBTNMARGIN - FWL_SYSBTNSIZE,
            FWL_SYSBTNMARGIN, FWL_SYSBTNSIZE, FWL_SYSBTNSIZE);
      }
    }
    m_iSysBox++;
  }
  IFWL_FormDP* pData =
      static_cast<IFWL_FormDP*>(m_pProperties->m_pDataProvider);
  if (m_pProperties->m_dwStyles & FWL_WGTSTYLE_Icon &&
      pData->GetIcon(m_pInterface, FALSE)) {
    if (m_bCustomizeLayout) {
      CFWL_ThemeBackground param;
      param.m_pWidget = m_pInterface;
      param.m_iPart = FWL_PART_FRM_Icon;
      CFX_WideString wsText;
      m_pProperties->m_pDataProvider->GetCaption(m_pInterface, wsText);
      param.m_pData = &wsText;
      pTheme->GetPartRect(&param, m_rtIcon);
    } else {
      m_rtIcon.Set(5, (m_rtCaption.height - m_fSmallIconSz) / 2, m_fSmallIconSz,
                   m_fSmallIconSz);
    }
  }
  if (m_bCustomizeLayout) {
    CFWL_ThemeText parma;
    parma.m_pWidget = m_pInterface;
    parma.m_iPart = FWL_PART_FRM_HeadText;
    m_pProperties->m_pDataProvider->GetCaption(m_pInterface, parma.m_wsText);
    pTheme->GetPartRect(&parma, m_rtCaptionText);
  }
}
void CFWL_FormImp::RegisterForm() {
  IFWL_NoteThread* pThread = GetOwnerThread();
  if (!pThread)
    return;
  CFWL_NoteDriver* pDriver =
      static_cast<CFWL_NoteDriver*>(pThread->GetNoteDriver());
  if (!pDriver)
    return;
  pDriver->RegisterForm(this);
}
void CFWL_FormImp::UnRegisterForm() {
  IFWL_NoteThread* pThread = GetOwnerThread();
  if (!pThread)
    return;
  CFWL_NoteDriver* pDriver =
      static_cast<CFWL_NoteDriver*>(pThread->GetNoteDriver());
  if (!pDriver)
    return;
  pDriver->UnRegisterForm(this);
}
FX_BOOL CFWL_FormImp::IsDoModal() {
  return m_bDoModalFlag;
}
void CFWL_FormImp::SetThemeData() {
  m_fSmallIconSz =
      *static_cast<FX_FLOAT*>(GetThemeCapacity(FWL_WGTCAPACITY_FRM_SmallIcon));
  m_fBigIconSz =
      *static_cast<FX_FLOAT*>(GetThemeCapacity(FWL_WGTCAPACITY_FRM_BigIcon));
}
FX_BOOL CFWL_FormImp::HasIcon() {
  IFWL_FormDP* pData =
      static_cast<IFWL_FormDP*>(m_pProperties->m_pDataProvider);
  return !!pData->GetIcon(m_pInterface, FALSE);
}
void CFWL_FormImp::UpdateIcon() {
  CFWL_WidgetMgr* pWidgetMgr = static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
  if (!pWidgetMgr)
    return;
  IFWL_FormDP* pData =
      static_cast<IFWL_FormDP*>(m_pProperties->m_pDataProvider);
  CFX_DIBitmap* pBigIcon = pData->GetIcon(m_pInterface, TRUE);
  CFX_DIBitmap* pSmallIcon = pData->GetIcon(m_pInterface, FALSE);
  if (pBigIcon && pBigIcon != m_pBigIcon) {
    m_pBigIcon = pBigIcon;
    pWidgetMgr->SetWidgetIcon_Native(m_pInterface, m_pBigIcon, TRUE);
  }
  if (pSmallIcon && pSmallIcon != m_pSmallIcon) {
    m_pSmallIcon = pSmallIcon;
    pWidgetMgr->SetWidgetIcon_Native(m_pInterface, m_pBigIcon, FALSE);
  }
}
void CFWL_FormImp::UpdateCaption() {
  CFWL_WidgetMgr* pWidgetMgr = static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
  if (!pWidgetMgr)
    return;
  IFWL_FormDP* pData =
      static_cast<IFWL_FormDP*>(m_pProperties->m_pDataProvider);
  if (!pData)
    return;
  CFX_WideString text;
  pData->GetCaption(m_pInterface, text);
  pWidgetMgr->SetWidgetCaption_Native(m_pInterface, text);
}
void CFWL_FormImp::DoWidthLimit(FX_FLOAT& fLeft,
                                FX_FLOAT& fWidth,
                                FX_FLOAT fCurX,
                                FX_FLOAT fSpace,
                                FX_FLOAT fLimitMin,
                                FX_FLOAT fLimitMax,
                                FX_BOOL bLeft) {
  FX_FLOAT fx = fCurX;
  FX_FLOAT fy = 0;
  TransformTo(NULL, fx, fy);
  FX_FLOAT fTemp =
      bLeft ? (fWidth - fx + fLeft + fSpace) : (fx - fLeft + fSpace);
  if (fTemp >= fLimitMin && fTemp <= fLimitMax) {
    fWidth = fTemp;
    fLeft += bLeft ? (fx - fLeft - fSpace) : 0;
  } else {
    if (fTemp < fLimitMin && fWidth > fLimitMin) {
      fLeft += bLeft ? (fWidth - fLimitMin) : 0;
      fWidth = fLimitMin;
    } else if (fTemp > fLimitMax && fWidth < fLimitMax) {
      fLeft -= bLeft ? (fLimitMax - fWidth) : 0;
      fWidth = fLimitMax;
    }
  }
}
void CFWL_FormImp::DoHeightLimit(FX_FLOAT& fTop,
                                 FX_FLOAT& fHeight,
                                 FX_FLOAT fCurY,
                                 FX_FLOAT fSpace,
                                 FX_FLOAT fLimitMin,
                                 FX_FLOAT fLimitMax,
                                 FX_BOOL bTop) {
  FX_FLOAT fx = 0;
  FX_FLOAT fy = fCurY;
  TransformTo(NULL, fx, fy);
  FX_FLOAT fTemp = bTop ? (fHeight - fy + fTop + fSpace) : (fy - fTop + fSpace);
  if (fTemp >= fLimitMin && fTemp <= fLimitMax) {
    fHeight = fTemp;
    fTop += bTop ? (fy - fTop - fSpace) : 0;
  } else {
    if (fTemp < fLimitMin && fHeight > fLimitMin) {
      fTop += bTop ? (fHeight - fLimitMin) : 0;
      fHeight = fLimitMin;
    } else if (fTemp > fLimitMax && fHeight < fLimitMax) {
      fTop -= bTop ? (fLimitMax - fHeight) : 0;
      fHeight = fLimitMax;
    }
  }
}
CFWL_FormImpDelegate::CFWL_FormImpDelegate(CFWL_FormImp* pOwner)
    : m_pOwner(pOwner) {
}
int32_t CFWL_FormImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
#ifdef FWL_UseMacSystemBorder
  if (!pMessage)
    return 0;
  FX_DWORD dwMsgCode = pMessage->GetClassID();
  int32_t iRet = 1;
  switch (dwMsgCode) {
    case FWL_MSGHASH_Activate: {
      m_pOwner->m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Deactivated;
      m_pOwner->Repaint(&m_pOwner->m_rtRelative);
      break;
    }
    case FWL_MSGHASH_Deactivate: {
      m_pOwner->m_pProperties->m_dwStates |= FWL_WGTSTATE_Deactivated;
      m_pOwner->Repaint(&m_pOwner->m_rtRelative);
      break;
    }
  }
  return FWL_ERR_Succeeded;
#else
  if (!pMessage)
    return 0;
  FX_DWORD dwMsgCode = pMessage->GetClassID();
  int32_t iRet = 1;
  switch (dwMsgCode) {
    case FWL_MSGHASH_Activate: {
      m_pOwner->m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Deactivated;
      IFWL_NoteThread* pThread = m_pOwner->GetOwnerThread();
      CFWL_NoteDriver* pDriver =
          static_cast<CFWL_NoteDriver*>(pThread->GetNoteDriver());
      CFWL_WidgetImp* pSubFocusImp = m_pOwner->GetSubFocus();
      IFWL_Widget* pSubFocus =
          pSubFocusImp ? pSubFocusImp->GetInterface() : NULL;
      if (pSubFocus && pSubFocus != pDriver->GetFocus()) {
        pDriver->SetFocus(pSubFocus);
      }
      m_pOwner->Repaint(&m_pOwner->m_rtRelative);
      break;
    }
    case FWL_MSGHASH_Deactivate: {
      m_pOwner->m_pProperties->m_dwStates |= FWL_WGTSTATE_Deactivated;
      IFWL_NoteThread* pThread = m_pOwner->GetOwnerThread();
      CFWL_NoteDriver* pDriver =
          static_cast<CFWL_NoteDriver*>(pThread->GetNoteDriver());
      CFWL_WidgetImp* pSubFocusImp = m_pOwner->GetSubFocus();
      IFWL_Widget* pSubFocus =
          pSubFocusImp ? pSubFocusImp->GetInterface() : NULL;
      if (pSubFocus) {
        if (pSubFocus == pDriver->GetFocus()) {
          pDriver->SetFocus(NULL);
        } else if (pSubFocus->GetStates() & FWL_WGTSTATE_Focused) {
          CFWL_MsgKillFocus ms;
          IFWL_WidgetDelegate* pDelegate = pSubFocus->SetDelegate(NULL);
          if (pDelegate) {
            pDelegate->OnProcessMessage(&ms);
          }
        }
      }
      m_pOwner->Repaint(&m_pOwner->m_rtRelative);
      break;
    }
    case FWL_MSGHASH_Mouse: {
      CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
      switch (pMsg->m_dwCmd) {
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
        case FWL_MSGMOUSECMD_MouseHover: {
          OnMouseHover(pMsg);
          break;
        }
        case FWL_MSGMOUSECMD_MouseLeave: {
          OnMouseLeave(pMsg);
          break;
        }
        case FWL_MSGMOUSECMD_LButtonDblClk: {
          OnLButtonDblClk(pMsg);
          break;
        }
      }
      break;
    }
    case FWL_MSGHASH_Size: {
      CFWL_WidgetMgr* pWidgetMgr =
          static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
      if (!pWidgetMgr)
        return 0;
      pWidgetMgr->AddRedrawCounts(m_pOwner->m_pInterface);
      if (!m_pOwner->m_bSetMaximize) {
        break;
      }
      m_pOwner->m_bSetMaximize = FALSE;
      CFWL_MsgSize* pMsg = static_cast<CFWL_MsgSize*>(pMessage);
      CFX_RectF rt;
      pWidgetMgr->GetWidgetRect_Native(m_pOwner->m_pInterface, rt);
      m_pOwner->m_pProperties->m_rtWidget.left = rt.left;
      m_pOwner->m_pProperties->m_rtWidget.top = rt.top;
      m_pOwner->m_pProperties->m_rtWidget.width = (FX_FLOAT)pMsg->m_iWidth;
      m_pOwner->m_pProperties->m_rtWidget.height = (FX_FLOAT)pMsg->m_iHeight;
      m_pOwner->Update();
      break;
    }
    case FWL_MSGHASH_WindowMove: {
      OnWindowMove(static_cast<CFWL_MsgWindowMove*>(pMessage));
      break;
    }
    case FWL_MSGHASH_Close: {
      OnClose(static_cast<CFWL_MsgClose*>(pMessage));
      break;
    }
    default: { iRet = 0; }
  }
  return iRet;
#endif
}
FWL_ERR CFWL_FormImpDelegate::OnProcessEvent(CFWL_Event* pEvent) {
  if (!pEvent)
    return FWL_ERR_Indefinite;
  if (pEvent->GetClassID() == FWL_EVTHASH_Close &&
      pEvent->m_pSrcTarget == m_pOwner->m_pInterface) {
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_FormImpDelegate::OnDrawWidget(CFX_Graphics* pGraphics,
                                           const CFX_Matrix* pMatrix) {
  return m_pOwner->DrawWidget(pGraphics, pMatrix);
}
void CFWL_FormImpDelegate::OnLButtonDown(CFWL_MsgMouse* pMsg) {
  m_pOwner->SetGrab(TRUE);
  m_pOwner->m_bLButtonDown = TRUE;
  m_pOwner->m_eResizeType = FORM_RESIZETYPE_None;
  CFWL_SysBtn* pPressBtn = m_pOwner->GetSysBtnAtPoint(pMsg->m_fx, pMsg->m_fy);
  m_pOwner->m_iCaptureBtn = m_pOwner->GetSysBtnIndex(pPressBtn);
  CFX_RectF rtCap;
  rtCap.Set(m_pOwner->m_rtCaption.left + m_pOwner->m_fCYBorder,
            m_pOwner->m_rtCaption.top + m_pOwner->m_fCXBorder,
            m_pOwner->m_rtCaption.width - FWL_SYSBTNSIZE * m_pOwner->m_iSysBox -
                2 * m_pOwner->m_fCYBorder,
            m_pOwner->m_rtCaption.height - m_pOwner->m_fCXBorder);
  if (pPressBtn) {
    pPressBtn->SetPressed();
    m_pOwner->Repaint(&pPressBtn->m_rtBtn);
  } else if (rtCap.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_pOwner->m_eResizeType = FORM_RESIZETYPE_Cap;
  } else if ((m_pOwner->m_pProperties->m_dwStyles & FWL_WGTSTYLE_Border) &&
             (m_pOwner->m_pProperties->m_dwStyleExes &
              FWL_STYLEEXT_FRM_Resize) &&
             !m_pOwner->m_bMaximized) {
    m_pOwner->SetCursor(pMsg->m_fx, pMsg->m_fy);
  }
  m_pOwner->m_InfoStart.m_ptStart.Set(pMsg->m_fx, pMsg->m_fy);
  m_pOwner->m_InfoStart.m_szStart.Set(
      m_pOwner->m_pProperties->m_rtWidget.width,
      m_pOwner->m_pProperties->m_rtWidget.height);
}
void CFWL_FormImpDelegate::OnLButtonUp(CFWL_MsgMouse* pMsg) {
  m_pOwner->SetGrab(FALSE);
  m_pOwner->m_bLButtonDown = FALSE;
  CFWL_SysBtn* pPointBtn = m_pOwner->GetSysBtnAtPoint(pMsg->m_fx, pMsg->m_fy);
  CFWL_SysBtn* pPressedBtn =
      m_pOwner->GetSysBtnByIndex(m_pOwner->m_iCaptureBtn);
  if (!pPressedBtn || pPointBtn != pPressedBtn) {
    return;
  }
  if (pPressedBtn == m_pOwner->GetSysBtnByState(FWL_SYSBUTTONSTATE_Pressed)) {
    pPressedBtn->SetNormal();
  }
  if (pPressedBtn == m_pOwner->m_pMaxBox) {
    if (m_pOwner->m_bMaximized) {
      m_pOwner->SetWidgetRect(m_pOwner->m_rtRestore);
      m_pOwner->Update();
      m_pOwner->Repaint();
    } else {
      m_pOwner->SetWorkAreaRect();
      m_pOwner->Update();
    }
    m_pOwner->m_bMaximized = !m_pOwner->m_bMaximized;
  } else if (pPressedBtn == m_pOwner->m_pMinBox) {
    CFWL_WidgetMgr* pWidgetMgr =
        static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
    if (!pWidgetMgr)
      return;
    pWidgetMgr->SetMinimize_Native(m_pOwner->m_pInterface);
  } else {
    CFWL_EvtClose eClose;
    eClose.m_pSrcTarget = m_pOwner->m_pInterface;
    m_pOwner->DispatchEvent(&eClose);
  }
}
void CFWL_FormImpDelegate::OnMouseMove(CFWL_MsgMouse* pMsg) {
  CFWL_WidgetMgr* pWidgetMgr = static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
  if (m_pOwner->m_bLButtonDown) {
    IFWL_AdapterNative* pNative = FWL_GetAdapterNative();
    IFWL_AdapterCursorMgr* pCursorMgr = pNative->GetCursorMgr();
    if (!pCursorMgr)
      return;
    CFWL_SysBtn* pPressedBtn =
        m_pOwner->GetSysBtnByIndex(m_pOwner->m_iCaptureBtn);
    FX_FLOAT fTop, fLeft, fWidth, fHeight;
    fTop = m_pOwner->m_pProperties->m_rtWidget.top;
    fLeft = m_pOwner->m_pProperties->m_rtWidget.left;
    fWidth = m_pOwner->m_pProperties->m_rtWidget.width;
    fHeight = m_pOwner->m_pProperties->m_rtWidget.height;
    FX_FLOAT fWidthMax, fWidthMin, fHeightMax, fHeightMin;
    if (m_pOwner->m_pContent) {
      m_pOwner->GetContent()->GetMaxSize(fWidthMax, fHeightMax);
      m_pOwner->GetContent()->GetMinSize(fWidthMin, fHeightMin);
    } else {
      fWidthMax = fHeightMax = 1024 * 4;
      fWidthMin = fHeightMin = 0;
    }
    FX_BOOL bWidthlimit = (fWidthMin != 0 || fWidthMax != 0);
    FX_BOOL bHeightlimit = (fHeightMin != 0 || fHeightMax != 0);
    FX_BOOL bSizelimit = bWidthlimit || bHeightlimit;
    if (fWidthMax != 0 || fHeightMax != 0 || fWidthMin != 0 ||
        fHeightMin != 0) {
      bSizelimit = TRUE;
    }
    if (pPressedBtn) {
      if (!pPressedBtn->m_rtBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
        pPressedBtn->SetNormal();
      } else {
        pPressedBtn->SetPressed();
      }
      m_pOwner->Repaint(&pPressedBtn->m_rtBtn);
      return;
    } else if (m_pOwner->m_bMaximized) {
      return;
    } else if (m_pOwner->m_eResizeType == FORM_RESIZETYPE_Cap) {
      m_pOwner->m_pProperties->m_rtWidget.Offset(
          pMsg->m_fx - m_pOwner->m_InfoStart.m_ptStart.x,
          pMsg->m_fy - m_pOwner->m_InfoStart.m_ptStart.y);
      pWidgetMgr->SetWidgetPosition_Native(
          m_pOwner->m_pInterface, m_pOwner->m_pProperties->m_rtWidget.left,
          m_pOwner->m_pProperties->m_rtWidget.top);
      return;
    } else if (m_pOwner->m_eResizeType == FORM_RESIZETYPE_Right) {
      FWL_HCURSOR hCursor = pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_SizeWE);
      pCursorMgr->SetCursor(hCursor);
      pCursorMgr->ShowCursor(TRUE);
      if (!bWidthlimit) {
        fWidth += pMsg->m_fx - m_pOwner->m_InfoStart.m_ptStart.x;
        m_pOwner->m_InfoStart.m_ptStart.x = pMsg->m_fx;
      } else {
        m_pOwner->DoWidthLimit(fLeft, fWidth, pMsg->m_fx,
                               m_pOwner->m_InfoStart.m_szStart.x -
                                   m_pOwner->m_InfoStart.m_ptStart.x,
                               fWidthMin, fWidthMax, FALSE);
      }
    } else if (m_pOwner->m_eResizeType == FORM_RESIZETYPE_Left) {
      FWL_HCURSOR hCursor = pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_SizeWE);
      pCursorMgr->SetCursor(hCursor);
      pCursorMgr->ShowCursor(TRUE);
      if (!bWidthlimit) {
        fLeft -= m_pOwner->m_InfoStart.m_ptStart.x - pMsg->m_fx;
        fWidth += m_pOwner->m_InfoStart.m_ptStart.x - pMsg->m_fx;
      } else {
        m_pOwner->DoWidthLimit(fLeft, fWidth, pMsg->m_fx,
                               m_pOwner->m_InfoStart.m_ptStart.x, fWidthMin,
                               fWidthMax, TRUE);
      }
    } else if (m_pOwner->m_eResizeType == FORM_RESIZETYPE_Bottom) {
      FWL_HCURSOR hCursor = pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_SizeNS);
      pCursorMgr->SetCursor(hCursor);
      pCursorMgr->ShowCursor(TRUE);
      if (!bHeightlimit) {
        fHeight += pMsg->m_fy - m_pOwner->m_InfoStart.m_ptStart.y;
        m_pOwner->m_InfoStart.m_ptStart.y = pMsg->m_fy;
      } else {
        m_pOwner->DoHeightLimit(fTop, fHeight, pMsg->m_fy,
                                m_pOwner->m_InfoStart.m_szStart.y -
                                    m_pOwner->m_InfoStart.m_ptStart.y,
                                fHeightMin, fHeightMax, FALSE);
      }
    } else if (m_pOwner->m_eResizeType == FORM_RESIZETYPE_Top) {
      FWL_HCURSOR hCursor = pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_SizeNS);
      pCursorMgr->SetCursor(hCursor);
      pCursorMgr->ShowCursor(TRUE);
      if (!bHeightlimit) {
        fTop += pMsg->m_fy - m_pOwner->m_InfoStart.m_ptStart.y;
        fHeight -= pMsg->m_fy - m_pOwner->m_InfoStart.m_ptStart.y;
      } else {
        m_pOwner->DoHeightLimit(fTop, fHeight, pMsg->m_fy,
                                m_pOwner->m_InfoStart.m_ptStart.y, fHeightMin,
                                fHeightMax, TRUE);
      }
    } else if (m_pOwner->m_eResizeType == FORM_RESIZETYPE_LeftTop) {
      FWL_HCURSOR hCursor =
          pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_SizeNWSE);
      pCursorMgr->SetCursor(hCursor);
      pCursorMgr->ShowCursor(TRUE);
      if (!bSizelimit) {
        fLeft -= m_pOwner->m_InfoStart.m_ptStart.x - pMsg->m_fx;
        fTop += pMsg->m_fy - m_pOwner->m_InfoStart.m_ptStart.y;
        fWidth += m_pOwner->m_InfoStart.m_ptStart.x - pMsg->m_fx;
        fHeight -= pMsg->m_fy - m_pOwner->m_InfoStart.m_ptStart.y;
        m_pOwner->m_InfoStart.m_ptStart.x = pMsg->m_fx;
        m_pOwner->m_InfoStart.m_ptStart.y = pMsg->m_fy;
      } else {
        m_pOwner->DoWidthLimit(fLeft, fWidth, pMsg->m_fx,
                               m_pOwner->m_InfoStart.m_ptStart.x, fWidthMin,
                               fWidthMax, TRUE);
        m_pOwner->DoHeightLimit(fTop, fHeight, pMsg->m_fy,
                                m_pOwner->m_InfoStart.m_ptStart.y, fHeightMin,
                                fHeightMax, TRUE);
      }
    } else if (m_pOwner->m_eResizeType == FORM_RESIZETYPE_LeftBottom) {
      FWL_HCURSOR hCursor =
          pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_SizeNESW);
      pCursorMgr->SetCursor(hCursor);
      pCursorMgr->ShowCursor(TRUE);
      if (!bSizelimit) {
        fLeft -= m_pOwner->m_InfoStart.m_ptStart.x - pMsg->m_fx;
        fWidth += m_pOwner->m_InfoStart.m_ptStart.x - pMsg->m_fx;
        fHeight += pMsg->m_fy - m_pOwner->m_InfoStart.m_ptStart.y;
        m_pOwner->m_InfoStart.m_ptStart.x = pMsg->m_fx;
        m_pOwner->m_InfoStart.m_ptStart.y = pMsg->m_fy;
      } else {
        m_pOwner->DoWidthLimit(fLeft, fWidth, pMsg->m_fx,
                               m_pOwner->m_InfoStart.m_ptStart.x, fWidthMin,
                               fWidthMax, TRUE);
        m_pOwner->DoHeightLimit(fTop, fHeight, pMsg->m_fy,
                                m_pOwner->m_InfoStart.m_szStart.y -
                                    m_pOwner->m_InfoStart.m_ptStart.y,
                                fHeightMin, fHeightMax, FALSE);
      }
    } else if (m_pOwner->m_eResizeType == FORM_RESIZETYPE_RightTop) {
      FWL_HCURSOR hCursor =
          pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_SizeNESW);
      pCursorMgr->SetCursor(hCursor);
      pCursorMgr->ShowCursor(TRUE);
      if (!bSizelimit) {
        fTop += pMsg->m_fy - m_pOwner->m_InfoStart.m_ptStart.y;
        fWidth += pMsg->m_fx - m_pOwner->m_InfoStart.m_ptStart.x;
        fHeight -= pMsg->m_fy - m_pOwner->m_InfoStart.m_ptStart.y;
        m_pOwner->m_InfoStart.m_ptStart.x = pMsg->m_fx;
        m_pOwner->m_InfoStart.m_ptStart.y = pMsg->m_fy;
      } else {
        m_pOwner->DoWidthLimit(fLeft, fWidth, pMsg->m_fx,
                               m_pOwner->m_InfoStart.m_szStart.x -
                                   m_pOwner->m_InfoStart.m_ptStart.x,
                               fWidthMin, fWidthMax, FALSE);
        m_pOwner->DoHeightLimit(fTop, fHeight, pMsg->m_fy,
                                m_pOwner->m_InfoStart.m_ptStart.y, fHeightMin,
                                fHeightMax, TRUE);
      }
    } else if (m_pOwner->m_eResizeType == FORM_RESIZETYPE_RightBottom) {
      FWL_HCURSOR hCursor =
          pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_SizeNWSE);
      pCursorMgr->SetCursor(hCursor);
      pCursorMgr->ShowCursor(TRUE);
      if (!bSizelimit) {
        fWidth += pMsg->m_fx - m_pOwner->m_InfoStart.m_ptStart.x;
        fHeight += pMsg->m_fy - m_pOwner->m_InfoStart.m_ptStart.y;
        m_pOwner->m_InfoStart.m_ptStart.x = pMsg->m_fx;
        m_pOwner->m_InfoStart.m_ptStart.y = pMsg->m_fy;
      } else {
        m_pOwner->DoWidthLimit(fLeft, fWidth, pMsg->m_fx,
                               m_pOwner->m_InfoStart.m_szStart.x -
                                   m_pOwner->m_InfoStart.m_ptStart.x,
                               fWidthMin, fWidthMax, FALSE);
        m_pOwner->DoHeightLimit(fTop, fHeight, pMsg->m_fy,
                                m_pOwner->m_InfoStart.m_szStart.y -
                                    m_pOwner->m_InfoStart.m_ptStart.y,
                                fHeightMin, fHeightMax, FALSE);
      }
    }
    if (m_pOwner->m_pContent) {
    }
    CFX_RectF rtForm;
    rtForm.Set(fLeft, fTop, fWidth, fHeight);
#if (_FX_OS_ == _FX_MACOSX_)
    m_pOwner->m_pProperties->m_rtWidget = rtForm;
    m_pOwner->Update();
    m_pOwner->SetWidgetRect(rtForm);
#else
    m_pOwner->SetWidgetRect(rtForm);
    m_pOwner->Update();
#endif
    return;
  }
  if ((m_pOwner->m_pProperties->m_dwStyles & FWL_WGTSTYLE_Border) &&
      (m_pOwner->m_pProperties->m_dwStyleExes & FWL_STYLEEXT_FRM_Resize) &&
      !m_pOwner->m_bMaximized) {
    m_pOwner->SetCursor(pMsg->m_fx, pMsg->m_fy);
  }
  CFX_RectF rtInvalidate;
  rtInvalidate.Reset();
  CFWL_SysBtn* pPointBtn = m_pOwner->GetSysBtnAtPoint(pMsg->m_fx, pMsg->m_fy);
  CFWL_SysBtn* pOldHover = m_pOwner->GetSysBtnByState(FWL_SYSBUTTONSTATE_Hover);
#if (_FX_OS_ == _FX_MACOSX_)
  {
    if (pOldHover && pPointBtn != pOldHover) {
      pOldHover->SetNormal();
    }
    if (pPointBtn && pPointBtn != pOldHover) {
      pPointBtn->SetHover();
    }
    if (m_pOwner->m_pCloseBox) {
      rtInvalidate = m_pOwner->m_pCloseBox->m_rtBtn;
    }
    if (m_pOwner->m_pMaxBox) {
      if (rtInvalidate.IsEmpty()) {
        rtInvalidate = m_pOwner->m_pMaxBox->m_rtBtn;
      } else {
        rtInvalidate.Union(m_pOwner->m_pMaxBox->m_rtBtn);
      }
    }
    if (m_pOwner->m_pMinBox) {
      if (rtInvalidate.IsEmpty()) {
        rtInvalidate = m_pOwner->m_pMinBox->m_rtBtn;
      } else {
        rtInvalidate.Union(m_pOwner->m_pMinBox->m_rtBtn);
      }
    }
    if (!rtInvalidate.IsEmpty() &&
        rtInvalidate.Contains(pMsg->m_fx, pMsg->m_fy)) {
      m_pOwner->m_bMouseIn = TRUE;
    }
  }
#else
  {
    if (pOldHover && pPointBtn != pOldHover) {
      pOldHover->SetNormal();
      rtInvalidate = pOldHover->m_rtBtn;
    }
    if (pPointBtn && pPointBtn != pOldHover) {
      pPointBtn->SetHover();
      if (rtInvalidate.IsEmpty()) {
        rtInvalidate = pPointBtn->m_rtBtn;
      } else {
        rtInvalidate.Union(pPointBtn->m_rtBtn);
      }
    }
  }
#endif
  if (!rtInvalidate.IsEmpty()) {
    m_pOwner->Repaint(&rtInvalidate);
  }
}
void CFWL_FormImpDelegate::OnMouseHover(CFWL_MsgMouse* pMsg) {
  m_pOwner->SetCursor(pMsg->m_fx, pMsg->m_fy);
}
void CFWL_FormImpDelegate::OnMouseLeave(CFWL_MsgMouse* pMsg) {
  CFWL_SysBtn* pHover = m_pOwner->GetSysBtnByState(FWL_SYSBUTTONSTATE_Hover);
  if (pHover) {
    pHover->SetNormal();
    m_pOwner->Repaint(&pHover->m_rtBtn);
  }
  if (pMsg->m_dwCmd == FWL_MSGMOUSECMD_MouseLeave &&
      !m_pOwner->m_bLButtonDown) {
    m_pOwner->SetCursor(pMsg->m_fx, pMsg->m_fy);
  }
}
void CFWL_FormImpDelegate::OnLButtonDblClk(CFWL_MsgMouse* pMsg) {
  if ((m_pOwner->m_pProperties->m_dwStyleExes & FWL_STYLEEXT_FRM_Resize) &&
      m_pOwner->HitTest(pMsg->m_fx, pMsg->m_fy) == FWL_WGTHITTEST_Titlebar) {
    if (m_pOwner->m_bMaximized) {
      m_pOwner->SetWidgetRect(m_pOwner->m_rtRestore);
    } else {
      m_pOwner->SetWorkAreaRect();
    }
    m_pOwner->Update();
    m_pOwner->m_bMaximized = !m_pOwner->m_bMaximized;
  }
}
void CFWL_FormImpDelegate::OnWindowMove(CFWL_MsgWindowMove* pMsg) {
  m_pOwner->m_pProperties->m_rtWidget.left = pMsg->m_fx;
  m_pOwner->m_pProperties->m_rtWidget.top = pMsg->m_fy;
}
void CFWL_FormImpDelegate::OnClose(CFWL_MsgClose* pMsg) {
  CFWL_EvtClose eClose;
  eClose.m_pSrcTarget = m_pOwner->m_pInterface;
  m_pOwner->DispatchEvent(&eClose);
}
FWL_ERR FWL_Accelerator_SetForm(IFWL_Form* pFrom,
                                CFX_MapAccelerators* pMapAccel) {
  CFWL_FormImp* pImp = static_cast<CFWL_FormImp*>(pFrom->GetImpl());
  if (!pImp)
    return FWL_ERR_Indefinite;
  return FWL_ERR_Succeeded;
}
