// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/ifwl_formproxy.h"

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/core/fwl_noteimp.h"

IFWL_FormProxy::IFWL_FormProxy(const IFWL_App* app,
                               const CFWL_WidgetImpProperties& properties,
                               IFWL_Widget* pOuter)
    : IFWL_Form(app, properties, pOuter) {
  SetDelegate(pdfium::MakeUnique<CFWL_FormProxyImpDelegate>(this));
}

IFWL_FormProxy::~IFWL_FormProxy() {}

FWL_Type IFWL_FormProxy::GetClassID() const {
  return FWL_Type::FormProxy;
}

FX_BOOL IFWL_FormProxy::IsInstance(const CFX_WideStringC& wsClass) const {
  if (wsClass == CFX_WideStringC(FWL_CLASS_FormProxy)) {
    return TRUE;
  }
  return IFWL_Form::IsInstance(wsClass);
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
  m_pOwner->m_pOuter->GetCurrentDelegate()->OnProcessMessage(pMessage);
}
