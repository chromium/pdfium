// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/lightwidget/cfwl_widget.h"

#include "xfa/fde/tto/fde_textout.h"
#include "xfa/fwl/core/cfwl_themetext.h"
#include "xfa/fwl/core/fwl_noteimp.h"
#include "xfa/fwl/core/fwl_noteimp.h"
#include "xfa/fwl/core/fwl_targetimp.h"
#include "xfa/fwl/core/fwl_widgetimp.h"
#include "xfa/fwl/core/fwl_widgetmgrimp.h"
#include "xfa/fwl/core/ifwl_notethread.h"
#include "xfa/fwl/core/ifwl_themeprovider.h"

IFWL_Widget* CFWL_Widget::GetWidget() {
  return m_pIface;
}

FWL_ERR CFWL_Widget::GetClassName(CFX_WideString& wsClass) const {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->GetClassName(wsClass);
}

uint32_t CFWL_Widget::GetClassID() const {
  if (!m_pIface)
    return 0;
  return m_pIface->GetClassID();
}

FX_BOOL CFWL_Widget::IsInstance(const CFX_WideStringC& wsClass) const {
  if (!m_pIface)
    return FALSE;
  return m_pIface->IsInstance(wsClass);
}

static void* gs_pFWLWidget = (void*)FXBSTR_ID('l', 'i', 'g', 't');

FWL_ERR CFWL_Widget::Initialize(const CFWL_WidgetProperties* pProperties) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->SetPrivateData(gs_pFWLWidget, this, NULL);
}

FWL_ERR CFWL_Widget::GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->GetWidgetRect(rect, bAutoSize);
}

FWL_ERR CFWL_Widget::GetGlobalRect(CFX_RectF& rect) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->GetGlobalRect(rect);
}

FWL_ERR CFWL_Widget::SetWidgetRect(const CFX_RectF& rect) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->SetWidgetRect(rect);
}

FWL_ERR CFWL_Widget::GetClientRect(CFX_RectF& rect) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->GetClientRect(rect);
}

CFWL_Widget* CFWL_Widget::GetParent() {
  if (!m_pIface)
    return NULL;
  IFWL_Widget* parent = m_pIface->GetParent();
  if (parent) {
    return static_cast<CFWL_Widget*>(parent->GetPrivateData(gs_pFWLWidget));
  }
  return NULL;
}

FWL_ERR CFWL_Widget::SetParent(CFWL_Widget* pParent) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->SetParent(pParent ? pParent->GetWidget() : NULL);
}

CFWL_Widget* CFWL_Widget::GetOwner() {
  if (!m_pIface)
    return NULL;
  return NULL;
}

FWL_ERR CFWL_Widget::SetOwner(CFWL_Widget* pOwner) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return FWL_ERR_Succeeded;
}

uint32_t CFWL_Widget::GetStyles() {
  if (!m_pIface)
    return 0;
  return m_pIface->GetStyles();
}

FWL_ERR CFWL_Widget::ModifyStyles(uint32_t dwStylesAdded,
                                  uint32_t dwStylesRemoved) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->ModifyStyles(dwStylesAdded, dwStylesRemoved);
}
uint32_t CFWL_Widget::GetStylesEx() {
  if (!m_pIface)
    return 0;
  return m_pIface->GetStylesEx();
}

FWL_ERR CFWL_Widget::ModifyStylesEx(uint32_t dwStylesExAdded,
                                    uint32_t dwStylesExRemoved) {
  return m_pIface->ModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
}

uint32_t CFWL_Widget::GetStates() {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->GetStates();
}

FWL_ERR CFWL_Widget::SetStates(uint32_t dwStates, FX_BOOL bSet) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->SetStates(dwStates, bSet);
}

FWL_ERR CFWL_Widget::SetPrivateData(void* module_id,
                                    void* pData,
                                    PD_CALLBACK_FREEDATA callback) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->SetPrivateData(module_id, pData, callback);
}

void* CFWL_Widget::GetPrivateData(void* module_id) {
  if (!m_pIface)
    return NULL;
  return m_pIface->GetPrivateData(module_id);
}

FWL_ERR CFWL_Widget::Update() {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->Update();
}

FWL_ERR CFWL_Widget::LockUpdate() {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->LockUpdate();
}

FWL_ERR CFWL_Widget::UnlockUpdate() {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->UnlockUpdate();
}

uint32_t CFWL_Widget::HitTest(FX_FLOAT fx, FX_FLOAT fy) {
  if (!m_pIface)
    return 0;
  return m_pIface->HitTest(fx, fy);
}

FWL_ERR CFWL_Widget::TransformTo(CFWL_Widget* pWidget,
                                 FX_FLOAT& fx,
                                 FX_FLOAT& fy) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->TransformTo(pWidget ? pWidget->GetWidget() : NULL, fx, fy);
}

FWL_ERR CFWL_Widget::TransformTo(CFWL_Widget* pWidget, CFX_RectF& rt) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->TransformTo(pWidget ? pWidget->GetWidget() : NULL, rt);
}

FWL_ERR CFWL_Widget::GetMatrix(CFX_Matrix& matrix, FX_BOOL bGlobal) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->GetMatrix(matrix, bGlobal);
}

FWL_ERR CFWL_Widget::SetMatrix(const CFX_Matrix& matrix) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->SetMatrix(matrix);
}

FWL_ERR CFWL_Widget::DrawWidget(CFX_Graphics* pGraphics,
                                const CFX_Matrix* pMatrix) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return m_pIface->DrawWidget(pGraphics, pMatrix);
}

IFWL_WidgetDelegate* CFWL_Widget::SetDelegate(IFWL_WidgetDelegate* pDelegate) {
  if (!m_pIface)
    return NULL;
  m_pDelegate = m_pIface->SetDelegate(pDelegate);
  return m_pDelegate;
}

CFWL_Widget::CFWL_Widget()
    : m_pIface(NULL), m_pDelegate(NULL), m_pProperties(NULL) {
  m_pProperties = new CFWL_WidgetProperties;
  m_pWidgetMgr = static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
  FXSYS_assert(m_pWidgetMgr != NULL);
}

CFWL_Widget::~CFWL_Widget() {
  delete m_pProperties;
  if (m_pIface) {
    m_pIface->Finalize();
    delete m_pIface;
  }
}

FWL_ERR CFWL_Widget::Repaint(const CFX_RectF* pRect) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  CFX_RectF rect;
  if (pRect) {
    rect = *pRect;
  } else {
    m_pIface->GetWidgetRect(rect);
    rect.left = rect.top = 0;
  }
  return m_pWidgetMgr->RepaintWidget(m_pIface, &rect);
}

FWL_ERR CFWL_Widget::SetFocus(FX_BOOL bFocus) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  IFWL_NoteThread* pThread = m_pIface->GetOwnerThread();
  if (!pThread)
    return FWL_ERR_Indefinite;
  IFWL_NoteDriver* pDriver = pThread->GetNoteDriver();
  if (!pDriver)
    return FWL_ERR_Indefinite;
  if (bFocus) {
    pDriver->SetFocus(m_pIface);
  } else {
    if (pDriver->GetFocus() == m_pIface) {
      pDriver->SetFocus(NULL);
    }
  }
  return FWL_ERR_Succeeded;
}

FWL_ERR CFWL_Widget::SetGrab(FX_BOOL bSet) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  IFWL_NoteThread* pThread = m_pIface->GetOwnerThread();
  if (!pThread)
    return FWL_ERR_Indefinite;
  IFWL_NoteDriver* pDriver = pThread->GetNoteDriver();
  if (!pDriver)
    return FWL_ERR_Indefinite;
  pDriver->SetGrab(m_pIface, bSet);
  return FWL_ERR_Succeeded;
}

void CFWL_Widget::RegisterEventTarget(CFWL_Widget* pEventSource,
                                      uint32_t dwFilter) {
  if (!m_pIface)
    return;
  IFWL_NoteThread* pThread = m_pIface->GetOwnerThread();
  if (!pThread)
    return;
  IFWL_NoteDriver* pNoteDriver = pThread->GetNoteDriver();
  if (!pNoteDriver)
    return;
  IFWL_Widget* pEventSourceImp =
      !pEventSource ? NULL : pEventSource->GetWidget();
  pNoteDriver->RegisterEventTarget(GetWidget(), pEventSourceImp, dwFilter);
}

void CFWL_Widget::DispatchEvent(CFWL_Event* pEvent) {
  if (!m_pIface)
    return;
  if (m_pIface->GetOuter()) {
    return;
  }
  IFWL_NoteThread* pThread = m_pIface->GetOwnerThread();
  if (!pThread)
    return;
  IFWL_NoteDriver* pNoteDriver = pThread->GetNoteDriver();
  if (!pNoteDriver)
    return;
  pNoteDriver->SendNote(pEvent);
}

#define FWL_WGT_CalcHeight 2048
#define FWL_WGT_CalcWidth 2048
#define FWL_WGT_CalcMultiLineDefWidth 120.0f

CFX_SizeF CFWL_Widget::CalcTextSize(const CFX_WideString& wsText,
                                    FX_BOOL bMultiLine,
                                    int32_t iLineWidth) {
  if (!m_pIface)
    return CFX_SizeF();
  IFWL_ThemeProvider* pTheme = m_pIface->GetThemeProvider();
  if (!pTheme)
    return CFX_SizeF();

  CFWL_ThemeText calPart;
  calPart.m_pWidget = m_pIface;
  calPart.m_wsText = wsText;
  calPart.m_dwTTOStyles =
      bMultiLine ? FDE_TTOSTYLE_LineWrap : FDE_TTOSTYLE_SingleLine;
  calPart.m_iTTOAlign = FDE_TTOALIGNMENT_TopLeft;
  CFX_RectF rect;
  FX_FLOAT fWidth = bMultiLine
                        ? (iLineWidth > 0 ? (FX_FLOAT)iLineWidth
                                          : FWL_WGT_CalcMultiLineDefWidth)
                        : FWL_WGT_CalcWidth;
  rect.Set(0, 0, fWidth, FWL_WGT_CalcHeight);
  pTheme->CalcTextRect(&calPart, rect);
  return CFX_SizeF(rect.width, rect.height);
}
