// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
CFWL_CheckBox* CFWL_CheckBox::Create() {
  return new CFWL_CheckBox;
}
FWL_ERR CFWL_CheckBox::Initialize(const CFWL_WidgetProperties* pProperties) {
  if (m_pIface)
    return FWL_ERR_Indefinite;
  if (pProperties) {
    *m_pProperties = *pProperties;
  }
  CFWL_WidgetImpProperties prop;
  prop.m_ctmOnParent = m_pProperties->m_ctmOnParent;
  prop.m_rtWidget = m_pProperties->m_rtWidget;
  prop.m_dwStyles = m_pProperties->m_dwStyles;
  prop.m_dwStyleExes = m_pProperties->m_dwStyleExes;
  prop.m_dwStates = m_pProperties->m_dwStates;
  prop.m_pDataProvider = &m_checkboxData;
  if (m_pProperties->m_pParent) {
    prop.m_pParent = m_pProperties->m_pParent->GetWidget();
  }
  if (m_pProperties->m_pOwner) {
    prop.m_pOwner = m_pProperties->m_pOwner->GetWidget();
  }
  m_pIface = new IFWL_CheckBox;
  FWL_ERR ret = ((IFWL_CheckBox*)m_pIface)->Initialize(prop, nullptr);
  if (ret == FWL_ERR_Succeeded) {
    CFWL_Widget::Initialize();
  }
  return ret;
}
FWL_ERR CFWL_CheckBox::SetCaption(const CFX_WideStringC& wsCaption) {
  m_checkboxData.m_wsCaption = wsCaption;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_CheckBox::SetBoxSize(FX_FLOAT fHeight) {
  m_checkboxData.m_fBoxHeight = fHeight;
  return FWL_ERR_Succeeded;
}
int32_t CFWL_CheckBox::GetCheckState() {
  return ((IFWL_CheckBox*)m_pIface)->GetCheckState();
}
FWL_ERR CFWL_CheckBox::SetCheckState(int32_t iCheck) {
  return ((IFWL_CheckBox*)m_pIface)->SetCheckState(iCheck);
}
CFWL_CheckBox::CFWL_CheckBox() {}
CFWL_CheckBox::~CFWL_CheckBox() {}
CFWL_CheckBox::CFWL_CheckBoxDP::CFWL_CheckBoxDP()
    : m_fBoxHeight(16.0f), m_wsCaption(L"Check box") {}
FWL_ERR CFWL_CheckBox::CFWL_CheckBoxDP::GetCaption(IFWL_Widget* pWidget,
                                                   CFX_WideString& wsCaption) {
  wsCaption = m_wsCaption;
  return FWL_ERR_Succeeded;
}
FX_FLOAT CFWL_CheckBox::CFWL_CheckBoxDP::GetBoxSize(IFWL_Widget* pWidget) {
  return m_fBoxHeight;
}
