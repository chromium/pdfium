// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_pagemodule.h"

#include "core/fpdfapi/font/cpdf_fontglobals.h"
#include "core/fpdfapi/page/cpdf_colorspace.h"
#include "core/fpdfapi/page/cpdf_devicecs.h"
#include "core/fpdfapi/page/cpdf_patterncs.h"

namespace {

CPDF_PageModule* g_PageModule = nullptr;

}  // namespace

// static
void CPDF_PageModule::Create() {
  ASSERT(!g_PageModule);
  g_PageModule = new CPDF_PageModule();
}

// static
void CPDF_PageModule::Destroy() {
  ASSERT(g_PageModule);
  delete g_PageModule;
  g_PageModule = nullptr;
}

// static
CPDF_PageModule* CPDF_PageModule::GetInstance() {
  ASSERT(g_PageModule);
  return g_PageModule;
}

CPDF_PageModule::CPDF_PageModule()
    : m_StockGrayCS(pdfium::MakeRetain<CPDF_DeviceCS>(PDFCS_DEVICEGRAY)),
      m_StockRGBCS(pdfium::MakeRetain<CPDF_DeviceCS>(PDFCS_DEVICERGB)),
      m_StockCMYKCS(pdfium::MakeRetain<CPDF_DeviceCS>(PDFCS_DEVICECMYK)),
      m_StockPatternCS(pdfium::MakeRetain<CPDF_PatternCS>(nullptr)) {
  m_StockPatternCS->InitializeStockPattern();
  CPDF_FontGlobals::Create();
  CPDF_FontGlobals::GetInstance()->LoadEmbeddedMaps();
}

CPDF_PageModule::~CPDF_PageModule() {
  CPDF_FontGlobals::Destroy();
}

RetainPtr<CPDF_ColorSpace> CPDF_PageModule::GetStockCS(int family) {
  if (family == PDFCS_DEVICEGRAY)
    return m_StockGrayCS;
  if (family == PDFCS_DEVICERGB)
    return m_StockRGBCS;
  if (family == PDFCS_DEVICECMYK)
    return m_StockCMYKCS;
  if (family == PDFCS_PATTERN)
    return m_StockPatternCS;
  return nullptr;
}

void CPDF_PageModule::ClearStockFont(CPDF_Document* pDoc) {
  CPDF_FontGlobals::GetInstance()->Clear(pDoc);
}
