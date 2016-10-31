// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_barcode.h"

#include <memory>

CFWL_Barcode::CFWL_Barcode(const IFWL_App* app) : CFWL_Edit(app) {}

CFWL_Barcode::~CFWL_Barcode() {}

void CFWL_Barcode::Initialize(const CFWL_WidgetProperties* pProperties) {
  ASSERT(!m_pIface);

  if (pProperties)
    *m_pProperties = *pProperties;

  std::unique_ptr<IFWL_Barcode> pBarcode(new IFWL_Barcode(
      m_pApp, m_pProperties->MakeWidgetImpProperties(&m_barcodeData)));
  pBarcode->Initialize();

  m_pIface = std::move(pBarcode);
  CFWL_Widget::Initialize(pProperties);
}

IFWL_Barcode* CFWL_Barcode::GetWidget() {
  return static_cast<IFWL_Barcode*>(m_pIface.get());
}

const IFWL_Barcode* CFWL_Barcode::GetWidget() const {
  return static_cast<IFWL_Barcode*>(m_pIface.get());
}

void CFWL_Barcode::SetType(BC_TYPE type) {
  if (GetWidget())
    GetWidget()->SetType(type);
}

FX_BOOL CFWL_Barcode::IsProtectedType() {
  return GetWidget() ? GetWidget()->IsProtectedType() : FALSE;
}

CFWL_Barcode::CFWL_BarcodeDP::CFWL_BarcodeDP()
    : m_dwAttributeMask(FWL_BCDATTRIBUTE_NONE) {}

FWL_Error CFWL_Barcode::CFWL_BarcodeDP::GetCaption(IFWL_Widget* pWidget,
                                                   CFX_WideString& wsCaption) {
  return FWL_Error::Succeeded;
}

BC_CHAR_ENCODING CFWL_Barcode::CFWL_BarcodeDP::GetCharEncoding() const {
  return m_eCharEncoding;
}

int32_t CFWL_Barcode::CFWL_BarcodeDP::GetModuleHeight() const {
  return m_nModuleHeight;
}

int32_t CFWL_Barcode::CFWL_BarcodeDP::GetModuleWidth() const {
  return m_nModuleWidth;
}

int32_t CFWL_Barcode::CFWL_BarcodeDP::GetDataLength() const {
  return m_nDataLength;
}

FX_BOOL CFWL_Barcode::CFWL_BarcodeDP::GetCalChecksum() const {
  return m_bCalChecksum;
}

FX_BOOL CFWL_Barcode::CFWL_BarcodeDP::GetPrintChecksum() const {
  return m_bPrintChecksum;
}

BC_TEXT_LOC CFWL_Barcode::CFWL_BarcodeDP::GetTextLocation() const {
  return m_eTextLocation;
}

int32_t CFWL_Barcode::CFWL_BarcodeDP::GetWideNarrowRatio() const {
  return m_nWideNarrowRatio;
}

FX_CHAR CFWL_Barcode::CFWL_BarcodeDP::GetStartChar() const {
  return m_cStartChar;
}

FX_CHAR CFWL_Barcode::CFWL_BarcodeDP::GetEndChar() const {
  return m_cEndChar;
}

int32_t CFWL_Barcode::CFWL_BarcodeDP::GetVersion() const {
  return m_nVersion;
}

int32_t CFWL_Barcode::CFWL_BarcodeDP::GetErrorCorrectionLevel() const {
  return m_nECLevel;
}

FX_BOOL CFWL_Barcode::CFWL_BarcodeDP::GetTruncated() const {
  return m_bTruncated;
}

uint32_t CFWL_Barcode::CFWL_BarcodeDP::GetBarcodeAttributeMask() const {
  return m_dwAttributeMask;
}
