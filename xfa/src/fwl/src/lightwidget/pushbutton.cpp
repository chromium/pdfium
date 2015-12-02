// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
CFWL_PushButton* CFWL_PushButton::Create() {
  return new CFWL_PushButton;
}
FWL_ERR CFWL_PushButton::Initialize(const CFWL_WidgetProperties* pProperties) {
  _FWL_RETURN_VALUE_IF_FAIL(!m_pIface, FWL_ERR_Indefinite);
  if (pProperties) {
    *m_pProperties = *pProperties;
  }
  m_pIface = IFWL_PushButton::Create();
  FWL_ERR ret =
      ((IFWL_PushButton*)m_pIface)
          ->Initialize(m_pProperties->MakeWidgetImpProperties(&m_buttonData),
                       nullptr);
  if (ret == FWL_ERR_Succeeded) {
    CFWL_Widget::Initialize();
  }
  return ret;
}
FWL_ERR CFWL_PushButton::GetCaption(CFX_WideString& wsCaption) {
  wsCaption = m_buttonData.m_wsCaption;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PushButton::SetCaption(const CFX_WideStringC& wsCaption) {
  m_buttonData.m_wsCaption = wsCaption;
  return FWL_ERR_Succeeded;
}
CFX_DIBitmap* CFWL_PushButton::GetPicture() {
  return m_buttonData.m_pBitmap;
}
FWL_ERR CFWL_PushButton::SetPicture(CFX_DIBitmap* pBitmap) {
  m_buttonData.m_pBitmap = pBitmap;
  return FWL_ERR_Succeeded;
}
CFWL_PushButton::CFWL_PushButton() {}
CFWL_PushButton::~CFWL_PushButton() {}
FWL_ERR CFWL_PushButton::CFWL_PushButtonDP::GetCaption(
    IFWL_Widget* pWidget,
    CFX_WideString& wsCaption) {
  wsCaption = m_wsCaption;
  return FWL_ERR_Succeeded;
}
CFX_DIBitmap* CFWL_PushButton::CFWL_PushButtonDP::GetPicture(
    IFWL_Widget* pWidget) {
  return m_pBitmap;
}
