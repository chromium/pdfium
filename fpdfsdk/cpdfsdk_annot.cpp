// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_annot.h"

#include <algorithm>

#include "fpdfsdk/cpdfsdk_pageview.h"

CPDFSDK_Annot::CPDFSDK_Annot(CPDFSDK_PageView* pPageView)
    : m_pPageView(pPageView) {}

CPDFSDK_Annot::~CPDFSDK_Annot() {}

CPDFSDK_BAAnnot* CPDFSDK_Annot::AsBAAnnot() {
  return nullptr;
}

CPDFXFA_Widget* CPDFSDK_Annot::AsXFAWidget() {
  return nullptr;
}

IPDF_Page* CPDFSDK_Annot::GetXFAPage() {
#ifdef PDF_ENABLE_XFA
  if (m_pPageView)
    return m_pPageView->GetXFAPage();
#endif
  return nullptr;
}

int CPDFSDK_Annot::GetLayoutOrder() const {
  return 5;
}

CPDF_Annot* CPDFSDK_Annot::GetPDFAnnot() const {
  return nullptr;
}

CPDF_Annot::Subtype CPDFSDK_Annot::GetAnnotSubtype() const {
  return CPDF_Annot::Subtype::UNKNOWN;
}

bool CPDFSDK_Annot::IsSignatureWidget() const {
  return false;
}

void CPDFSDK_Annot::SetRect(const CFX_FloatRect& rect) {}

CFX_FloatRect CPDFSDK_Annot::GetRect() const {
  return CFX_FloatRect();
}

IPDF_Page* CPDFSDK_Annot::GetPage() {
#ifdef PDF_ENABLE_XFA
  IPDF_Page* pXFAPage = GetXFAPage();
  if (pXFAPage)
    return pXFAPage;
#endif  // PDF_ENABLE_XFA
  return GetPDFPage();
}

CPDF_Page* CPDFSDK_Annot::GetPDFPage() {
  return m_pPageView ? m_pPageView->GetPDFPage() : nullptr;
}
