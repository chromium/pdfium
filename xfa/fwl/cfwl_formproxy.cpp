// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_formproxy.h"

#include <memory>
#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/cfwl_notedriver.h"

CFWL_FormProxy::CFWL_FormProxy(
    const CFWL_App* app,
    std::unique_ptr<CFWL_WidgetProperties> properties,
    CFWL_Widget* pOuter)
    : CFWL_Form(app, std::move(properties), pOuter) {}

CFWL_FormProxy::~CFWL_FormProxy() {}

FWL_Type CFWL_FormProxy::GetClassID() const {
  return FWL_Type::FormProxy;
}

bool CFWL_FormProxy::IsInstance(const WideStringView& wsClass) const {
  if (wsClass == WideStringView(FWL_CLASS_FormProxy))
    return true;
  return CFWL_Form::IsInstance(wsClass);
}

void CFWL_FormProxy::Update() {}

void CFWL_FormProxy::DrawWidget(CXFA_Graphics* pGraphics,
                                const CFX_Matrix& matrix) {}

void CFWL_FormProxy::OnProcessMessage(CFWL_Message* pMessage) {
  m_pOuter->GetDelegate()->OnProcessMessage(pMessage);
}
