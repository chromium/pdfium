// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
CFWL_Barcode* CFWL_Barcode::Create() {
  return new CFWL_Barcode;
}
FWL_ERR CFWL_Barcode::Initialize(const CFWL_WidgetProperties* pProperties) {
  if (m_pIface)
    return FWL_ERR_Indefinite;
  if (pProperties) {
    *m_pProperties = *pProperties;
  }
  m_pIface = new IFWL_Barcode;
  FWL_ERR ret =
      ((IFWL_Barcode*)m_pIface)
          ->Initialize(m_pProperties->MakeWidgetImpProperties(&m_barcodeData),
                       nullptr);
  if (ret == FWL_ERR_Succeeded) {
    CFWL_Widget::Initialize();
  }
  return ret;
}
CFWL_Barcode::CFWL_Barcode() {}
CFWL_Barcode::~CFWL_Barcode() {}
void CFWL_Barcode::SetType(BC_TYPE type) {
  if (!m_pIface)
    return;
  ((IFWL_Barcode*)m_pIface)->SetType(type);
}
FX_BOOL CFWL_Barcode::IsProtectedType() {
  if (!m_pIface)
    return 0;
  return ((IFWL_Barcode*)m_pIface)->IsProtectedType();
}
FWL_ERR CFWL_Barcode::CFWL_BarcodeDP::GetCaption(IFWL_Widget* pWidget,
                                                 CFX_WideString& wsCaption) {
  return FWL_ERR_Succeeded;
}
