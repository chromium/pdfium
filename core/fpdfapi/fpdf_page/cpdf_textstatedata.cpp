// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_page/include/cpdf_textstatedata.h"

#include "core/fpdfapi/fpdf_font/include/cpdf_font.h"
#include "core/fpdfapi/fpdf_page/pageint.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"

CPDF_TextStateData::CPDF_TextStateData()
    : m_pFont(nullptr),
      m_pDocument(nullptr),
      m_FontSize(1.0f),
      m_CharSpace(0),
      m_WordSpace(0),
      m_TextMode(0) {
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
