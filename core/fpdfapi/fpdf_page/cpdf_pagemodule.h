// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_CPDF_PAGEMODULE_H_
#define CORE_FPDFAPI_FPDF_PAGE_CPDF_PAGEMODULE_H_

#include "core/fpdfapi/fpdf_font/font_int.h"
#include "core/fpdfapi/fpdf_page/include/cpdf_colorspace.h"
#include "core/fpdfapi/fpdf_page/pageint.h"

class CPDF_Document;

class CPDF_PageModule {
 public:
  CPDF_PageModule()
      : m_StockGrayCS(nullptr, PDFCS_DEVICEGRAY),
        m_StockRGBCS(nullptr, PDFCS_DEVICERGB),
        m_StockCMYKCS(nullptr, PDFCS_DEVICECMYK),
        m_StockPatternCS(nullptr) {}
  ~CPDF_PageModule() {}

  CPDF_FontGlobals* GetFontGlobals() { return &m_FontGlobals; }
  CPDF_ColorSpace* GetStockCS(int family);
  void ClearStockFont(CPDF_Document* pDoc) { m_FontGlobals.Clear(pDoc); }

 private:
  CPDF_FontGlobals m_FontGlobals;
  CPDF_DeviceCS m_StockGrayCS;
  CPDF_DeviceCS m_StockRGBCS;
  CPDF_DeviceCS m_StockCMYKCS;
  CPDF_PatternCS m_StockPatternCS;
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_CPDF_PAGEMODULE_H_
