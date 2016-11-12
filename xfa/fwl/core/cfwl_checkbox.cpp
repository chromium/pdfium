// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_checkbox.h"

#include <memory>

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/core/fwl_error.h"

namespace {

IFWL_CheckBox* ToCheckBox(IFWL_Widget* widget) {
  return static_cast<IFWL_CheckBox*>(widget);
}

}  // namespace

CFWL_CheckBox::CFWL_CheckBox(const IFWL_App* app)
    : CFWL_Widget(app), m_fBoxHeight(16.0f), m_wsCaption(L"Check box") {}

CFWL_CheckBox::~CFWL_CheckBox() {}

void CFWL_CheckBox::Initialize() {
  ASSERT(!m_pIface);

  m_pIface = pdfium::MakeUnique<IFWL_CheckBox>(
      m_pApp, pdfium::MakeUnique<CFWL_WidgetProperties>(this));

  CFWL_Widget::Initialize();
}

FWL_Error CFWL_CheckBox::SetCaption(const CFX_WideStringC& wsCaption) {
  m_wsCaption = wsCaption;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_CheckBox::SetBoxSize(FX_FLOAT fHeight) {
  m_fBoxHeight = fHeight;
  return FWL_Error::Succeeded;
}

int32_t CFWL_CheckBox::GetCheckState() {
  return ToCheckBox(GetWidget())->GetCheckState();
}

FWL_Error CFWL_CheckBox::SetCheckState(int32_t iCheck) {
  return ToCheckBox(GetWidget())->SetCheckState(iCheck);
}

void CFWL_CheckBox::GetCaption(IFWL_Widget* pWidget,
                               CFX_WideString& wsCaption) {
  wsCaption = m_wsCaption;
}

FX_FLOAT CFWL_CheckBox::GetBoxSize(IFWL_Widget* pWidget) {
  return m_fBoxHeight;
}
