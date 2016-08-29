// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_CPDF_COLORSTATEDATA_H_
#define CORE_FPDFAPI_FPDF_PAGE_CPDF_COLORSTATEDATA_H_

#include "core/fpdfapi/fpdf_page/include/cpdf_color.h"
#include "core/fxcrt/include/fx_system.h"

class CPDF_Color;
class CPDF_ColorSpace;
class CPDF_Pattern;

class CPDF_ColorStateData {
 public:
  CPDF_ColorStateData() : m_FillRGB(0), m_StrokeRGB(0) {}
  CPDF_ColorStateData(const CPDF_ColorStateData& src);

  void SetDefault();

  uint32_t GetFillRGB() const { return m_FillRGB; }
  void SetFillRGB(uint32_t rgb) { m_FillRGB = rgb; }

  uint32_t GetStrokeRGB() const { return m_StrokeRGB; }
  void SetStrokeRGB(uint32_t rgb) { m_StrokeRGB = rgb; }

  CPDF_Color* GetFillColor() { return &m_FillColor; }
  const CPDF_Color* GetFillColor() const { return &m_FillColor; }
  void SetFillColor(CPDF_ColorSpace* pCS, FX_FLOAT* pValue, uint32_t nValues);

  CPDF_Color* GetStrokeColor() { return &m_StrokeColor; }
  const CPDF_Color* GetStrokeColor() const { return &m_StrokeColor; }
  void SetStrokeColor(CPDF_ColorSpace* pCS, FX_FLOAT* pValue, uint32_t nValues);

  void SetFillPattern(CPDF_Pattern* pattern,
                      FX_FLOAT* pValue,
                      uint32_t nValues);
  void SetStrokePattern(CPDF_Pattern* pattern,
                        FX_FLOAT* pValue,
                        uint32_t nValues);

 private:
  void SetColor(CPDF_Color& color,
                uint32_t& rgb,
                CPDF_ColorSpace* pCS,
                FX_FLOAT* pValue,
                uint32_t nValues);

  uint32_t m_FillRGB;
  uint32_t m_StrokeRGB;
  CPDF_Color m_FillColor;
  CPDF_Color m_StrokeColor;
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_CPDF_COLORSTATEDATA_H_
