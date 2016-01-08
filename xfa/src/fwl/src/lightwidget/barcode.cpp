// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <memory>

#include "xfa/src/foxitlib.h"

CFWL_Barcode* CFWL_Barcode::Create() {
  return new CFWL_Barcode;
}
FWL_ERR CFWL_Barcode::Initialize(const CFWL_WidgetProperties* pProperties) {
  if (m_pIface)
    return FWL_ERR_Indefinite;
  if (pProperties) {
    *m_pProperties = *pProperties;
  }
  std::unique_ptr<IFWL_Barcode> pBarcode(IFWL_Barcode::Create(
      m_pProperties->MakeWidgetImpProperties(&m_barcodeData)));
  FWL_ERR ret = pBarcode->Initialize();
  if (ret != FWL_ERR_Succeeded) {
    return ret;
  }
  m_pIface = pBarcode.release();
  CFWL_Widget::Initialize();
  return FWL_ERR_Succeeded;
}
CFWL_Barcode::CFWL_Barcode() {}
CFWL_Barcode::~CFWL_Barcode() {}
void CFWL_Barcode::SetType(BC_TYPE type) {
  if (!m_pIface)
    return;
  static_cast<IFWL_Barcode*>(m_pIface)->SetType(type);
}
FX_BOOL CFWL_Barcode::IsProtectedType() {
  if (!m_pIface)
    return 0;
  return static_cast<IFWL_Barcode*>(m_pIface)->IsProtectedType();
}
FWL_ERR CFWL_Barcode::CFWL_BarcodeDP::GetCaption(IFWL_Widget* pWidget,
                                                 CFX_WideString& wsCaption) {
  return FWL_ERR_Succeeded;
}
