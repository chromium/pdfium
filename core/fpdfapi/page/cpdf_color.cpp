// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_color.h"

#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fxcrt/fx_system.h"

CPDF_Color::CPDF_Color() {}

CPDF_Color::~CPDF_Color() {
  ReleaseBuffer();
  ReleaseColorSpace();
}

bool CPDF_Color::IsPattern() const {
  return m_pCS && IsPatternInternal();
}

void CPDF_Color::ReleaseBuffer() {
  if (!m_pBuffer)
    return;

  if (IsPatternInternal()) {
    PatternValue* pvalue = reinterpret_cast<PatternValue*>(m_pBuffer);
    CPDF_Pattern* pPattern =
        pvalue->m_pCountedPattern ? pvalue->m_pCountedPattern->get() : nullptr;
    if (pPattern) {
      CPDF_DocPageData* pPageData = pPattern->document()->GetPageData();
      if (pPageData)
        pPageData->ReleasePattern(pPattern->pattern_obj());
    }
  }
  FX_Free(m_pBuffer);
  m_pBuffer = nullptr;
}

void CPDF_Color::ReleaseColorSpace() {
  if (!m_pCS)
    return;

  CPDF_Document* pDoc = m_pCS->GetDocument();
  if (!pDoc)
    return;

  auto* pPageData = pDoc->GetPageData();
  if (pPageData)
    pPageData->ReleaseColorSpace(m_pCS->GetArray());

  m_pCS = nullptr;
}

bool CPDF_Color::IsPatternInternal() const {
  return m_pCS->GetFamily() == PDFCS_PATTERN;
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
  if (pCS) {
    m_pBuffer = pCS->CreateBuf();
    pCS->GetDefaultColor(m_pBuffer);
  }
}

void CPDF_Color::SetValue(const float* comps) {
  if (!m_pBuffer)
    return;
  if (!IsPatternInternal())
    memcpy(m_pBuffer, comps, m_pCS->CountComponents() * sizeof(float));
}

void CPDF_Color::SetValue(CPDF_Pattern* pPattern,
                          const float* comps,
                          uint32_t ncomps) {
  if (ncomps > kMaxPatternColorComps)
    return;

  if (!IsPattern()) {
    FX_Free(m_pBuffer);
    m_pCS = CPDF_ColorSpace::GetStockCS(PDFCS_PATTERN);
    m_pBuffer = m_pCS->CreateBuf();
  }

  CPDF_DocPageData* pDocPageData = nullptr;
  PatternValue* pvalue = reinterpret_cast<PatternValue*>(m_pBuffer);
  if (pvalue->m_pPattern) {
    pDocPageData = pvalue->m_pPattern->document()->GetPageData();
    pDocPageData->ReleasePattern(pvalue->m_pPattern->pattern_obj());
  }
  pvalue->m_nComps = ncomps;
  pvalue->m_pPattern = pPattern;
  if (ncomps)
    memcpy(pvalue->m_Comps, comps, ncomps * sizeof(float));

  pvalue->m_pCountedPattern = nullptr;
  if (pPattern) {
    if (!pDocPageData)
      pDocPageData = pPattern->document()->GetPageData();

    pvalue->m_pCountedPattern =
        pDocPageData->FindPatternPtr(pPattern->pattern_obj());
  }
}

void CPDF_Color::Copy(const CPDF_Color* pSrc) {
  ReleaseBuffer();
  ReleaseColorSpace();
  m_pCS = pSrc->m_pCS;
  if (!m_pCS)
    return;

  CPDF_Document* pDoc = m_pCS->GetDocument();
  CPDF_Array* pArray = m_pCS->GetArray();
  if (pDoc && pArray) {
    m_pCS = pDoc->GetPageData()->GetCopiedColorSpace(pArray);
    if (!m_pCS)
      return;
  }
  m_pBuffer = m_pCS->CreateBuf();
  memcpy(m_pBuffer, pSrc->m_pBuffer, m_pCS->GetBufSize());
  if (!IsPatternInternal())
    return;

  PatternValue* pValue = reinterpret_cast<PatternValue*>(m_pBuffer);
  CPDF_Pattern* pPattern = pValue->m_pPattern;
  if (!pPattern)
    return;

  pValue->m_pPattern = pPattern->document()->GetPageData()->GetPattern(
      pPattern->pattern_obj(), false, pPattern->parent_matrix());
}

bool CPDF_Color::GetRGB(int* R, int* G, int* B) const {
  if (!m_pCS || !m_pBuffer)
    return false;

  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  if (!m_pCS->GetRGB(m_pBuffer, &r, &g, &b))
    return false;

  *R = static_cast<int32_t>(r * 255 + 0.5f);
  *G = static_cast<int32_t>(g * 255 + 0.5f);
  *B = static_cast<int32_t>(b * 255 + 0.5f);
  return true;
}

CPDF_Pattern* CPDF_Color::GetPattern() const {
  if (!m_pBuffer || !IsPatternInternal())
    return nullptr;

  PatternValue* pvalue = reinterpret_cast<PatternValue*>(m_pBuffer);
  return pvalue->m_pPattern;
}
