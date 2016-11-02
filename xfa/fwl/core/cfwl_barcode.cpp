// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_barcode.h"

#include <memory>

#include "third_party/base/ptr_util.h"

namespace {

IFWL_Barcode* ToBarcode(IFWL_Widget* widget) {
  return static_cast<IFWL_Barcode*>(widget);
}

}  // namespace

CFWL_Barcode::CFWL_Barcode(const IFWL_App* app) : CFWL_Edit(app) {}

CFWL_Barcode::~CFWL_Barcode() {}

void CFWL_Barcode::Initialize() {
  ASSERT(!m_pIface);

  m_pIface = pdfium::MakeUnique<IFWL_Barcode>(
      m_pApp, m_pProperties->MakeWidgetImpProperties(&m_barcodeData));

  CFWL_Widget::Initialize();
}

void CFWL_Barcode::SetCharEncoding(BC_CHAR_ENCODING encoding) {
  m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_CHARENCODING;
  m_barcodeData.m_eCharEncoding = encoding;
}
void CFWL_Barcode::SetModuleHeight(int32_t height) {
  m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_MODULEHEIGHT;
  m_barcodeData.m_nModuleHeight = height;
}
void CFWL_Barcode::SetModuleWidth(int32_t width) {
  m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_MODULEWIDTH;
  m_barcodeData.m_nModuleWidth = width;
}
void CFWL_Barcode::SetDataLength(int32_t dataLength) {
  m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_DATALENGTH;
  m_barcodeData.m_nDataLength = dataLength;
  ToBarcode(GetWidget())->SetLimit(dataLength);
}
void CFWL_Barcode::SetCalChecksum(bool calChecksum) {
  m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_CALCHECKSUM;
  m_barcodeData.m_bCalChecksum = calChecksum;
}
void CFWL_Barcode::SetPrintChecksum(bool printChecksum) {
  m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_PRINTCHECKSUM;
  m_barcodeData.m_bPrintChecksum = printChecksum;
}
void CFWL_Barcode::SetTextLocation(BC_TEXT_LOC location) {
  m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_TEXTLOCATION;
  m_barcodeData.m_eTextLocation = location;
}
void CFWL_Barcode::SetWideNarrowRatio(int32_t ratio) {
  m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_WIDENARROWRATIO;
  m_barcodeData.m_nWideNarrowRatio = ratio;
}
void CFWL_Barcode::SetStartChar(FX_CHAR startChar) {
  m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_STARTCHAR;
  m_barcodeData.m_cStartChar = startChar;
}
void CFWL_Barcode::SetEndChar(FX_CHAR endChar) {
  m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_ENDCHAR;
  m_barcodeData.m_cEndChar = endChar;
}
void CFWL_Barcode::SetVersion(int32_t version) {
  m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_VERSION;
  m_barcodeData.m_nVersion = version;
}
void CFWL_Barcode::SetErrorCorrectionLevel(int32_t ecLevel) {
  m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_ECLEVEL;
  m_barcodeData.m_nECLevel = ecLevel;
}
void CFWL_Barcode::SetTruncated(bool truncated) {
  m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_TRUNCATED;
  m_barcodeData.m_bTruncated = truncated;
}
void CFWL_Barcode::ResetBarcodeAttributes() {
  m_barcodeData.m_dwAttributeMask = FWL_BCDATTRIBUTE_NONE;
}

void CFWL_Barcode::SetType(BC_TYPE type) {
  if (GetWidget())
    ToBarcode(GetWidget())->SetType(type);
}

bool CFWL_Barcode::IsProtectedType() {
  return GetWidget() ? ToBarcode(GetWidget())->IsProtectedType() : false;
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

bool CFWL_Barcode::CFWL_BarcodeDP::GetCalChecksum() const {
  return m_bCalChecksum;
}

bool CFWL_Barcode::CFWL_BarcodeDP::GetPrintChecksum() const {
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

bool CFWL_Barcode::CFWL_BarcodeDP::GetTruncated() const {
  return m_bTruncated;
}

uint32_t CFWL_Barcode::CFWL_BarcodeDP::GetBarcodeAttributeMask() const {
  return m_dwAttributeMask;
}
