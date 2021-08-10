// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_barcode.h"

#include "fxbarcode/cfx_barcode.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_themepart.h"
#include "xfa/fwl/ifwl_themeprovider.h"
#include "xfa/fwl/theme/cfwl_utils.h"

CFWL_Barcode::CFWL_Barcode(CFWL_App* app)
    : CFWL_Edit(app, Properties(), nullptr) {}

CFWL_Barcode::~CFWL_Barcode() = default;

FWL_Type CFWL_Barcode::GetClassID() const {
  return FWL_Type::Barcode;
}

void CFWL_Barcode::Update() {
  if (IsLocked())
    return;

  CFWL_Edit::Update();
  GenerateBarcodeImageCache();
}

void CFWL_Barcode::DrawWidget(CFGAS_GEGraphics* pGraphics,
                              const CFX_Matrix& matrix) {
  if (!pGraphics)
    return;

  if ((m_Properties.m_dwStates & FWL_STATE_WGT_Focused) == 0) {
    GenerateBarcodeImageCache();
    if (!m_pBarcodeEngine || m_eStatus != Status::kEncodeSuccess)
      return;

    CFX_Matrix mt;
    mt.e = GetRTClient().left;
    mt.f = GetRTClient().top;
    mt.Concat(matrix);

    // TODO(tsepez): Curious as to why |mt| is unused?
    m_pBarcodeEngine->RenderDevice(pGraphics->GetRenderDevice(), matrix);
    return;
  }
  CFWL_Edit::DrawWidget(pGraphics, matrix);
}

void CFWL_Barcode::SetType(BC_TYPE type) {
  if (m_type == type)
    return;

  m_pBarcodeEngine.reset();
  m_type = type;
  m_eStatus = Status::kNeedUpdate;
}

void CFWL_Barcode::SetText(const WideString& wsText) {
  m_pBarcodeEngine.reset();
  m_eStatus = Status::kNeedUpdate;
  CFWL_Edit::SetText(wsText);
}

void CFWL_Barcode::SetTextSkipNotify(const WideString& wsText) {
  m_pBarcodeEngine.reset();
  m_eStatus = Status::kNeedUpdate;
  CFWL_Edit::SetTextSkipNotify(wsText);
}

bool CFWL_Barcode::IsProtectedType() const {
  if (!m_pBarcodeEngine)
    return true;

  BC_TYPE tEngineType = m_pBarcodeEngine->GetType();
  return tEngineType == BC_QR_CODE || tEngineType == BC_PDF417 ||
         tEngineType == BC_DATAMATRIX;
}

void CFWL_Barcode::OnProcessEvent(CFWL_Event* pEvent) {
  if (pEvent->GetType() == CFWL_Event::Type::TextWillChange) {
    m_pBarcodeEngine.reset();
    m_eStatus = Status::kNeedUpdate;
  }
  CFWL_Edit::OnProcessEvent(pEvent);
}

void CFWL_Barcode::SetCharEncoding(BC_CHAR_ENCODING encoding) {
  m_eCharEncoding = encoding;
}

void CFWL_Barcode::SetModuleHeight(int32_t height) {
  m_nModuleHeight = height;
}

void CFWL_Barcode::SetModuleWidth(int32_t width) {
  m_nModuleWidth = width;
}

void CFWL_Barcode::SetDataLength(int32_t dataLength) {
  m_nDataLength = dataLength;
  SetLimit(dataLength);
}

void CFWL_Barcode::SetCalChecksum(bool calChecksum) {
  m_bCalChecksum = calChecksum;
}

void CFWL_Barcode::SetPrintChecksum(bool printChecksum) {
  m_bPrintChecksum = printChecksum;
}

void CFWL_Barcode::SetTextLocation(BC_TEXT_LOC location) {
  m_eTextLocation = location;
}

void CFWL_Barcode::SetWideNarrowRatio(int8_t ratio) {
  m_nWideNarrowRatio = ratio;
}

void CFWL_Barcode::SetStartChar(char startChar) {
  m_cStartChar = startChar;
}

void CFWL_Barcode::SetEndChar(char endChar) {
  m_cEndChar = endChar;
}

void CFWL_Barcode::SetErrorCorrectionLevel(int32_t ecLevel) {
  m_nECLevel = ecLevel;
}

void CFWL_Barcode::GenerateBarcodeImageCache() {
  if (m_eStatus != Status::kNeedUpdate)
    return;

  m_eStatus = Status::kNormal;
  CreateBarcodeEngine();
  if (!m_pBarcodeEngine)
    return;

  IFWL_ThemeProvider* pTheme = GetThemeProvider();
  CFWL_ThemePart part(this);
  if (RetainPtr<CFGAS_GEFont> pFont = pTheme->GetFont(part)) {
    if (CFX_Font* pCXFont = pFont->GetDevFont())
      m_pBarcodeEngine->SetFont(pCXFont);
  }
  m_pBarcodeEngine->SetFontSize(pTheme->GetFontSize(part));
  m_pBarcodeEngine->SetFontColor(pTheme->GetTextColor(part));
  m_pBarcodeEngine->SetHeight(int32_t(GetRTClient().height));
  m_pBarcodeEngine->SetWidth(int32_t(GetRTClient().width));
  if (m_eCharEncoding.has_value())
    m_pBarcodeEngine->SetCharEncoding(m_eCharEncoding.value());
  if (m_nModuleHeight.has_value())
    m_pBarcodeEngine->SetModuleHeight(m_nModuleHeight.value());
  if (m_nModuleWidth.has_value())
    m_pBarcodeEngine->SetModuleWidth(m_nModuleWidth.value());
  if (m_nDataLength.has_value())
    m_pBarcodeEngine->SetDataLength(m_nDataLength.value());
  if (m_bCalChecksum.value())
    m_pBarcodeEngine->SetCalChecksum(m_bCalChecksum.value());
  if (m_bPrintChecksum.has_value())
    m_pBarcodeEngine->SetPrintChecksum(m_bPrintChecksum.value());
  if (m_eTextLocation.has_value())
    m_pBarcodeEngine->SetTextLocation(m_eTextLocation.value());
  if (m_nWideNarrowRatio.has_value())
    m_pBarcodeEngine->SetWideNarrowRatio(m_nWideNarrowRatio.value());
  if (m_cStartChar.has_value())
    m_pBarcodeEngine->SetStartChar(m_cStartChar.value());
  if (m_cEndChar.has_value())
    m_pBarcodeEngine->SetEndChar(m_cEndChar.value());
  if (m_nECLevel.has_value())
    m_pBarcodeEngine->SetErrorCorrectionLevel(m_nECLevel.value());

  m_eStatus = m_pBarcodeEngine->Encode(GetText().AsStringView())
                  ? Status::kEncodeSuccess
                  : Status::kNormal;
}

void CFWL_Barcode::CreateBarcodeEngine() {
  if (m_pBarcodeEngine || m_type == BC_UNKNOWN)
    return;

  m_pBarcodeEngine = CFX_Barcode::Create(m_type);
}
