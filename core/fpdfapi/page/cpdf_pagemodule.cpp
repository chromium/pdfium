// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_pagemodule.h"

#include "core/fpdfapi/font/cpdf_fontglobals.h"
#include "core/fpdfapi/page/cpdf_colorspace.h"
#include "core/fpdfapi/page/cpdf_devicecs.h"
#include "core/fpdfapi/page/cpdf_patterncs.h"
#include "third_party/base/check.h"

namespace {

CPDF_PageModule* g_PageModule = nullptr;

}  // namespace

// static
void CPDF_PageModule::Create() {
  DCHECK(!g_PageModule);
  g_PageModule = new CPDF_PageModule();
}

// static
void CPDF_PageModule::Destroy() {
  DCHECK(g_PageModule);
  delete g_PageModule;
  g_PageModule = nullptr;
}

// static
CPDF_PageModule* CPDF_PageModule::GetInstance() {
  DCHECK(g_PageModule);
  return g_PageModule;
}

CPDF_PageModule::CPDF_PageModule()
    : m_StockGrayCS(pdfium::MakeRetain<CPDF_DeviceCS>(
          CPDF_ColorSpace::Family::kDeviceGray)),
      m_StockRGBCS(pdfium::MakeRetain<CPDF_DeviceCS>(
          CPDF_ColorSpace::Family::kDeviceRGB)),
      m_StockCMYKCS(pdfium::MakeRetain<CPDF_DeviceCS>(
          CPDF_ColorSpace::Family::kDeviceCMYK)),
      m_StockPatternCS(pdfium::MakeRetain<CPDF_PatternCS>()) {
  m_StockPatternCS->InitializeStockPattern();
  CPDF_FontGlobals::Create();
  CPDF_FontGlobals::GetInstance()->LoadEmbeddedMaps();
}

CPDF_PageModule::~CPDF_PageModule() {
  CPDF_FontGlobals::Destroy();
}

RetainPtr<CPDF_ColorSpace> CPDF_PageModule::GetStockCS(
    CPDF_ColorSpace::Family family) {
  if (family == CPDF_ColorSpace::Family::kDeviceGray)
    return m_StockGrayCS;
  if (family == CPDF_ColorSpace::Family::kDeviceRGB)
    return m_StockRGBCS;
  if (family == CPDF_ColorSpace::Family::kDeviceCMYK)
    return m_StockCMYKCS;
  if (family == CPDF_ColorSpace::Family::kPattern)
    return m_StockPatternCS;
  return nullptr;
}

void CPDF_PageModule::ClearStockFont(CPDF_Document* pDoc) {
  CPDF_FontGlobals::GetInstance()->Clear(pDoc);
}
