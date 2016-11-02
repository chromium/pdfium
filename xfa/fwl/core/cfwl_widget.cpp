// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_widget.h"

#include "xfa/fde/tto/fde_textout.h"
#include "xfa/fwl/core/cfwl_themetext.h"
#include "xfa/fwl/core/cfwl_widgetmgr.h"
#include "xfa/fwl/core/fwl_noteimp.h"
#include "xfa/fwl/core/fwl_noteimp.h"
#include "xfa/fwl/core/ifwl_app.h"
#include "xfa/fwl/core/ifwl_themeprovider.h"

#define FWL_WGT_CalcHeight 2048
#define FWL_WGT_CalcWidth 2048
#define FWL_WGT_CalcMultiLineDefWidth 120.0f

CFWL_Widget::CFWL_Widget(const IFWL_App* app)
    : m_pApp(app),
      m_pWidgetMgr(app->GetWidgetMgr()),
      m_pProperties(new CFWL_WidgetProperties) {
  ASSERT(m_pWidgetMgr);
}

CFWL_Widget::~CFWL_Widget() {}

void CFWL_Widget::Initialize() {
  ASSERT(m_pIface);
  m_pIface->SetAssociateWidget(this);
}

IFWL_Widget* CFWL_Widget::GetWidget() {
  return m_pIface.get();
}

const IFWL_Widget* CFWL_Widget::GetWidget() const {
  return m_pIface.get();
}

FWL_Error CFWL_Widget::GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize) {
  if (!m_pIface)
    return FWL_Error::Indefinite;
  return m_pIface->GetWidgetRect(rect, bAutoSize);
}

FWL_Error CFWL_Widget::GetGlobalRect(CFX_RectF& rect) {
  if (!m_pIface)
    return FWL_Error::Indefinite;
  return m_pIface->GetGlobalRect(rect);
}

FWL_Error CFWL_Widget::SetWidgetRect(const CFX_RectF& rect) {
  if (!m_pIface)
    return FWL_Error::Indefinite;
  return m_pIface->SetWidgetRect(rect);
}

FWL_Error CFWL_Widget::GetClientRect(CFX_RectF& rect) {
  if (!m_pIface)
    return FWL_Error::Indefinite;
  return m_pIface->GetClientRect(rect);
}

FWL_Error CFWL_Widget::ModifyStyles(uint32_t dwStylesAdded,
                                    uint32_t dwStylesRemoved) {
  if (!m_pIface)
    return FWL_Error::Indefinite;
  return m_pIface->ModifyStyles(dwStylesAdded, dwStylesRemoved);
}

uint32_t CFWL_Widget::GetStylesEx() {
  if (!m_pIface)
    return 0;
  return m_pIface->GetStylesEx();
}

FWL_Error CFWL_Widget::ModifyStylesEx(uint32_t dwStylesExAdded,
                                      uint32_t dwStylesExRemoved) {
  return m_pIface->ModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
}

uint32_t CFWL_Widget::GetStates() {
  return m_pIface ? m_pIface->GetStates() : 0;
}

void CFWL_Widget::SetStates(uint32_t dwStates, FX_BOOL bSet) {
  if (m_pIface)
    m_pIface->SetStates(dwStates, bSet);
}

void CFWL_Widget::SetLayoutItem(void* pItem) {
  if (m_pIface)
    m_pIface->SetLayoutItem(pItem);
}

void CFWL_Widget::Update() {
  if (!m_pIface)
    return;
  m_pIface->Update();
}

void CFWL_Widget::LockUpdate() {
  if (!m_pIface)
    return;
  m_pIface->LockUpdate();
}

void CFWL_Widget::UnlockUpdate() {
  if (!m_pIface)
    return;
  m_pIface->UnlockUpdate();
}

FWL_WidgetHit CFWL_Widget::HitTest(FX_FLOAT fx, FX_FLOAT fy) {
  if (!m_pIface)
    return FWL_WidgetHit::Unknown;
  return m_pIface->HitTest(fx, fy);
}

FWL_Error CFWL_Widget::DrawWidget(CFX_Graphics* pGraphics,
                                  const CFX_Matrix* pMatrix) {
  if (!m_pIface)
    return FWL_Error::Indefinite;
  return m_pIface->DrawWidget(pGraphics, pMatrix);
}

IFWL_WidgetDelegate* CFWL_Widget::GetCurrentDelegate() {
  return m_pIface ? m_pIface->GetCurrentDelegate() : nullptr;
}

void CFWL_Widget::SetCurrentDelegate(IFWL_WidgetDelegate* pDelegate) {
  if (m_pIface)
    m_pIface->SetCurrentDelegate(pDelegate);
}
