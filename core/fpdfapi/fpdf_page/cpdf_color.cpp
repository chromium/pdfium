// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_page/include/cpdf_color.h"

#include "core/fpdfapi/fpdf_page/pageint.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fxcrt/include/fx_system.h"

CPDF_Color::CPDF_Color(int family) {
  m_pCS = CPDF_ColorSpace::GetStockCS(family);
  int nComps = 3;
  if (family == PDFCS_DEVICEGRAY)
    nComps = 1;
  else if (family == PDFCS_DEVICECMYK)
    nComps = 4;

  m_pBuffer = FX_Alloc(FX_FLOAT, nComps);
  for (int i = 0; i < nComps; i++)
    m_pBuffer[i] = 0;
}

CPDF_Color::~CPDF_Color() {
  ReleaseBuffer();
  ReleaseColorSpace();
}

void CPDF_Color::ReleaseBuffer() {
  if (!m_pBuffer)
    return;

  if (m_pCS->GetFamily() == PDFCS_PATTERN) {
    PatternValue* pvalue = (PatternValue*)m_pBuffer;
    CPDF_Pattern* pPattern =
        pvalue->m_pCountedPattern ? pvalue->m_pCountedPattern->get() : nullptr;
    if (pPattern && pPattern->m_pDocument) {
      CPDF_DocPageData* pPageData = pPattern->m_pDocument->GetPageData();
      if (pPageData)
        pPageData->ReleasePattern(pPattern->m_pPatternObj);
    }
  }
  FX_Free(m_pBuffer);
  m_pBuffer = nullptr;
}

void CPDF_Color::ReleaseColorSpace() {
  if (m_pCS && m_pCS->m_pDocument && m_pCS->GetArray()) {
    m_pCS->m_pDocument->GetPageData()->ReleaseColorSpace(m_pCS->GetArray());
    m_pCS = nullptr;
  }
}

void CPDF_Color::SetColorSpace(CPDF_ColorSpace* pCS) {
  if (m_pCS == pCS) {
    if (!m_pBuffer)
      m_pBuffer = pCS->CreateBuf();

    ReleaseColorSpace();
    m_pCS = pCS;
    return;
  }
  ReleaseBuffer();
  ReleaseColorSpace();

  m_pCS = pCS;
  if (m_pCS) {
    m_pBuffer = pCS->CreateBuf();
    pCS->GetDefaultColor(m_pBuffer);
  }
}

void CPDF_Color::SetValue(FX_FLOAT* comps) {
  if (!m_pBuffer)
    return;
  if (m_pCS->GetFamily() != PDFCS_PATTERN)
    FXSYS_memcpy(m_pBuffer, comps, m_pCS->CountComponents() * sizeof(FX_FLOAT));
}

void CPDF_Color::SetValue(CPDF_Pattern* pPattern, FX_FLOAT* comps, int ncomps) {
  if (ncomps > MAX_PATTERN_COLORCOMPS)
    return;

  if (!m_pCS || m_pCS->GetFamily() != PDFCS_PATTERN) {
    FX_Free(m_pBuffer);
    m_pCS = CPDF_ColorSpace::GetStockCS(PDFCS_PATTERN);
    m_pBuffer = m_pCS->CreateBuf();
  }

  CPDF_DocPageData* pDocPageData = nullptr;
  PatternValue* pvalue = (PatternValue*)m_pBuffer;
  if (pvalue->m_pPattern && pvalue->m_pPattern->m_pDocument) {
    pDocPageData = pvalue->m_pPattern->m_pDocument->GetPageData();
    if (pDocPageData)
      pDocPageData->ReleasePattern(pvalue->m_pPattern->m_pPatternObj);
  }
  pvalue->m_nComps = ncomps;
  pvalue->m_pPattern = pPattern;
  if (ncomps)
    FXSYS_memcpy(pvalue->m_Comps, comps, ncomps * sizeof(FX_FLOAT));

  pvalue->m_pCountedPattern = nullptr;
  if (pPattern && pPattern->m_pDocument) {
    if (!pDocPageData)
      pDocPageData = pPattern->m_pDocument->GetPageData();

    pvalue->m_pCountedPattern =
        pDocPageData->FindPatternPtr(pPattern->m_pPatternObj);
  }
}

void CPDF_Color::Copy(const CPDF_Color* pSrc) {
  ReleaseBuffer();
  ReleaseColorSpace();

  m_pCS = pSrc->m_pCS;
  if (m_pCS && m_pCS->m_pDocument) {
    CPDF_Array* pArray = m_pCS->GetArray();
    if (pArray)
      m_pCS = m_pCS->m_pDocument->GetPageData()->GetCopiedColorSpace(pArray);
  }
  if (!m_pCS)
    return;

  m_pBuffer = m_pCS->CreateBuf();
  FXSYS_memcpy(m_pBuffer, pSrc->m_pBuffer, m_pCS->GetBufSize());
  if (m_pCS->GetFamily() == PDFCS_PATTERN) {
    PatternValue* pvalue = (PatternValue*)m_pBuffer;
    if (pvalue->m_pPattern && pvalue->m_pPattern->m_pDocument) {
      pvalue->m_pPattern =
          pvalue->m_pPattern->m_pDocument->GetPageData()->GetPattern(
              pvalue->m_pPattern->m_pPatternObj, FALSE,
              &pvalue->m_pPattern->m_ParentMatrix);
    }
  }
}

FX_BOOL CPDF_Color::GetRGB(int& R, int& G, int& B) const {
  if (!m_pCS || !m_pBuffer)
    return FALSE;

  FX_FLOAT r = 0.0f, g = 0.0f, b = 0.0f;
  if (!m_pCS->GetRGB(m_pBuffer, r, g, b))
    return FALSE;

  R = (int32_t)(r * 255 + 0.5f);
  G = (int32_t)(g * 255 + 0.5f);
  B = (int32_t)(b * 255 + 0.5f);
  return TRUE;
}

CPDF_Pattern* CPDF_Color::GetPattern() const {
  if (!m_pBuffer || m_pCS->GetFamily() != PDFCS_PATTERN)
    return nullptr;

  PatternValue* pvalue = (PatternValue*)m_pBuffer;
  return pvalue->m_pPattern;
}

CPDF_ColorSpace* CPDF_Color::GetPatternCS() const {
  if (!m_pBuffer || m_pCS->GetFamily() != PDFCS_PATTERN)
    return nullptr;
  return m_pCS->GetBaseCS();
}

FX_FLOAT* CPDF_Color::GetPatternColor() const {
  if (!m_pBuffer || m_pCS->GetFamily() != PDFCS_PATTERN)
    return nullptr;

  PatternValue* pvalue = (PatternValue*)m_pBuffer;
  return pvalue->m_nComps ? pvalue->m_Comps : nullptr;
}

FX_BOOL CPDF_Color::IsEqual(const CPDF_Color& other) const {
  return m_pCS && m_pCS == other.m_pCS &&
         FXSYS_memcmp(m_pBuffer, other.m_pBuffer, m_pCS->GetBufSize()) == 0;
}
