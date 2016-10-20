// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/ifwl_formproxy.h"

#include "xfa/fwl/core/fwl_noteimp.h"

// static
IFWL_FormProxy* IFWL_FormProxy::Create(CFWL_WidgetImpProperties& properties,
                                       IFWL_Widget* pOuter) {
  return new IFWL_FormProxy(properties, pOuter);
}

IFWL_FormProxy::IFWL_FormProxy(const CFWL_WidgetImpProperties& properties,
                               IFWL_Widget* pOuter)
    : IFWL_Form(properties, pOuter) {}

IFWL_FormProxy::~IFWL_FormProxy() {}

FWL_Error IFWL_FormProxy::GetClassName(CFX_WideString& wsClass) const {
  wsClass = FWL_CLASS_FormProxy;
  return FWL_Error::Succeeded;
}

FWL_Type IFWL_FormProxy::GetClassID() const {
  return FWL_Type::FormProxy;
}

FX_BOOL IFWL_FormProxy::IsInstance(const CFX_WideStringC& wsClass) const {
  if (wsClass == CFX_WideStringC(FWL_CLASS_FormProxy)) {
    return TRUE;
  }
  return IFWL_Form::IsInstance(wsClass);
}

FWL_Error IFWL_FormProxy::Initialize() {
  if (IFWL_Widget::Initialize() != FWL_Error::Succeeded)
    return FWL_Error::Indefinite;
  m_pDelegate = new CFWL_FormProxyImpDelegate(this);
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_FormProxy::Finalize() {
  delete m_pDelegate;
  m_pDelegate = nullptr;
  return IFWL_Widget::Finalize();
}

FWL_Error IFWL_FormProxy::Update() {
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_FormProxy::DrawWidget(CFX_Graphics* pGraphics,
                                     const CFX_Matrix* pMatrix) {
  return FWL_Error::Succeeded;
}

CFWL_FormProxyImpDelegate::CFWL_FormProxyImpDelegate(IFWL_FormProxy* pOwner)
    : m_pOwner(pOwner) {}

void CFWL_FormProxyImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
  IFWL_WidgetDelegate* pDelegate = m_pOwner->m_pOuter->SetDelegate(nullptr);
  pDelegate->OnProcessMessage(pMessage);
}
