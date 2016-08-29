// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_page/include/cpdf_textstatedata.h"

#include "core/fpdfapi/fpdf_font/include/cpdf_font.h"
#include "core/fpdfapi/fpdf_page/pageint.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"

bool SetTextRenderingModeFromInt(int iMode, TextRenderingMode* mode) {
  if (iMode < 0 || iMode > 7)
    return false;
  *mode = static_cast<TextRenderingMode>(iMode);
  return true;
}

bool TextRenderingModeIsClipMode(const TextRenderingMode& mode) {
  switch (mode) {
    case TextRenderingMode::MODE_FILL_CLIP:
    case TextRenderingMode::MODE_STROKE_CLIP:
    case TextRenderingMode::MODE_FILL_STROKE_CLIP:
    case TextRenderingMode::MODE_CLIP:
      return true;
    default:
      return false;
  }
}

bool TextRenderingModeIsStrokeMode(const TextRenderingMode& mode) {
  switch (mode) {
    case TextRenderingMode::MODE_STROKE:
    case TextRenderingMode::MODE_FILL_STROKE:
    case TextRenderingMode::MODE_STROKE_CLIP:
    case TextRenderingMode::MODE_FILL_STROKE_CLIP:
      return true;
    default:
      return false;
  }
}

CPDF_TextStateData::CPDF_TextStateData()
    : m_pFont(nullptr),
      m_pDocument(nullptr),
      m_FontSize(1.0f),
      m_CharSpace(0),
      m_WordSpace(0),
      m_TextMode(TextRenderingMode::MODE_FILL) {
  m_Matrix[0] = m_Matrix[3] = 1.0f;
  m_Matrix[1] = m_Matrix[2] = 0;
  m_CTM[0] = m_CTM[3] = 1.0f;
  m_CTM[1] = m_CTM[2] = 0;
}

CPDF_TextStateData::CPDF_TextStateData(const CPDF_TextStateData& src) {
  if (this == &src)
    return;

  FXSYS_memcpy(this, &src, sizeof(CPDF_TextStateData));
  if (m_pDocument && m_pFont) {
    m_pFont =
        m_pDocument->GetPageData()->GetFont(m_pFont->GetFontDict(), FALSE);
  }
}

CPDF_TextStateData::~CPDF_TextStateData() {
  if (m_pDocument && m_pFont) {
    CPDF_DocPageData* pPageData = m_pDocument->GetPageData();
    if (pPageData && !pPageData->IsForceClear())
      pPageData->ReleaseFont(m_pFont->GetFontDict());
  }
}

void CPDF_TextStateData::SetFont(CPDF_Font* pFont) {
  CPDF_DocPageData* pPageData =
      m_pDocument ? m_pDocument->GetPageData() : nullptr;
  if (pPageData && m_pFont && !pPageData->IsForceClear())
    pPageData->ReleaseFont(m_pFont->GetFontDict());

  m_pDocument = pFont ? pFont->m_pDocument : nullptr;
  m_pFont = pFont;
}

FX_FLOAT CPDF_TextStateData::GetFontSizeV() const {
  const FX_FLOAT* pMatrix = GetMatrix();
  FX_FLOAT unit = FXSYS_sqrt2(pMatrix[1], pMatrix[3]);
  FX_FLOAT size = unit * GetFontSize();
  return (FX_FLOAT)FXSYS_fabs(size);
}

FX_FLOAT CPDF_TextStateData::GetFontSizeH() const {
  const FX_FLOAT* pMatrix = GetMatrix();
  FX_FLOAT unit = FXSYS_sqrt2(pMatrix[0], pMatrix[2]);
  FX_FLOAT size = unit * GetFontSize();
  return (FX_FLOAT)FXSYS_fabs(size);
}

FX_FLOAT CPDF_TextStateData::GetBaselineAngle() const {
  const FX_FLOAT* pMatrix = GetMatrix();
  return FXSYS_atan2(pMatrix[2], pMatrix[0]);
}

FX_FLOAT CPDF_TextStateData::GetShearAngle() const {
  const FX_FLOAT* pMatrix = GetMatrix();
  FX_FLOAT shear_angle = FXSYS_atan2(pMatrix[1], pMatrix[3]);
  return GetBaselineAngle() + shear_angle;
}
