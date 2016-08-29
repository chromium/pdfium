// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_page/cpdf_colorstatedata.h"

#include "core/fpdfapi/fpdf_page/cpdf_tilingpattern.h"
#include "core/fxge/include/fx_dib.h"

CPDF_ColorStateData::CPDF_ColorStateData(const CPDF_ColorStateData& src)
    : m_FillRGB(src.m_FillRGB), m_StrokeRGB(src.m_StrokeRGB) {
  m_FillColor.Copy(&src.m_FillColor);
  m_StrokeColor.Copy(&src.m_StrokeColor);
}

void CPDF_ColorStateData::SetDefault() {
  m_FillRGB = 0;
  m_StrokeRGB = 0;
  m_FillColor.SetColorSpace(CPDF_ColorSpace::GetStockCS(PDFCS_DEVICEGRAY));
  m_StrokeColor.SetColorSpace(CPDF_ColorSpace::GetStockCS(PDFCS_DEVICEGRAY));
}

void CPDF_ColorStateData::SetFillColor(CPDF_ColorSpace* pCS,
                                       FX_FLOAT* pValue,
                                       uint32_t nValues) {
  SetColor(m_FillColor, m_FillRGB, pCS, pValue, nValues);
}

void CPDF_ColorStateData::SetStrokeColor(CPDF_ColorSpace* pCS,
                                         FX_FLOAT* pValue,
                                         uint32_t nValues) {
  SetColor(m_StrokeColor, m_StrokeRGB, pCS, pValue, nValues);
}

void CPDF_ColorStateData::SetFillPattern(CPDF_Pattern* pPattern,
                                         FX_FLOAT* pValue,
                                         uint32_t nValues) {
  m_FillColor.SetValue(pPattern, pValue, nValues);
  int R, G, B;
  FX_BOOL ret = m_FillColor.GetRGB(R, G, B);
  if (CPDF_TilingPattern* pTilingPattern = pPattern->AsTilingPattern()) {
    if (!ret && pTilingPattern->colored()) {
      m_FillRGB = 0x00BFBFBF;
      return;
    }
  }
  m_FillRGB = ret ? FXSYS_RGB(R, G, B) : (uint32_t)-1;
}

void CPDF_ColorStateData::SetStrokePattern(CPDF_Pattern* pPattern,
                                           FX_FLOAT* pValue,
                                           uint32_t nValues) {
  m_StrokeColor.SetValue(pPattern, pValue, nValues);
  int R, G, B;
  FX_BOOL ret = m_StrokeColor.GetRGB(R, G, B);
  if (CPDF_TilingPattern* pTilingPattern = pPattern->AsTilingPattern()) {
    if (!ret && pTilingPattern->colored()) {
      m_StrokeRGB = 0x00BFBFBF;
      return;
    }
  }
  m_StrokeRGB =
      m_StrokeColor.GetRGB(R, G, B) ? FXSYS_RGB(R, G, B) : (uint32_t)-1;
}

void CPDF_ColorStateData::SetColor(CPDF_Color& color,
                                   uint32_t& rgb,
                                   CPDF_ColorSpace* pCS,
                                   FX_FLOAT* pValue,
                                   uint32_t nValues) {
  if (pCS) {
    color.SetColorSpace(pCS);
  } else if (color.IsNull()) {
    color.SetColorSpace(CPDF_ColorSpace::GetStockCS(PDFCS_DEVICEGRAY));
  }
  if (color.GetColorSpace()->CountComponents() > nValues)
    return;

  color.SetValue(pValue);
  int R, G, B;
  rgb = color.GetRGB(R, G, B) ? FXSYS_RGB(R, G, B) : (uint32_t)-1;
}
