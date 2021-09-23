// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_annot.h"

#include "fpdfsdk/cpdfsdk_pageview.h"
#include "third_party/base/check.h"

CPDFSDK_Annot::CPDFSDK_Annot(CPDFSDK_PageView* pPageView)
    : m_pPageView(pPageView) {
  DCHECK(m_pPageView);
}

CPDFSDK_Annot::~CPDFSDK_Annot() = default;

CPDFSDK_BAAnnot* CPDFSDK_Annot::AsBAAnnot() {
  return nullptr;
}

CPDFXFA_Widget* CPDFSDK_Annot::AsXFAWidget() {
  return nullptr;
}

IPDF_Page* CPDFSDK_Annot::GetXFAPage() {
#ifdef PDF_ENABLE_XFA
  return m_pPageView->GetXFAPage();
#else
  return nullptr;
#endif
}

int CPDFSDK_Annot::GetLayoutOrder() const {
  return 5;
}

CPDF_Annot* CPDFSDK_Annot::GetPDFAnnot() const {
  return nullptr;
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
  return m_pPageView->GetPDFPage();
}
