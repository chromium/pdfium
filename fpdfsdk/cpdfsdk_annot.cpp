// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/cpdfsdk_annot.h"

#include <algorithm>

#include "fpdfsdk/include/fsdk_mgr.h"
#include "third_party/base/stl_util.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_doc.h"
#endif  // PDF_ENABLE_XFA

namespace {

const float kMinWidth = 1.0f;
const float kMinHeight = 1.0f;

}  // namespace

CPDFSDK_Annot::Observer::Observer(CPDFSDK_Annot** pWatchedPtr)
    : m_pWatchedPtr(pWatchedPtr) {
  (*m_pWatchedPtr)->AddObserver(this);
}

CPDFSDK_Annot::Observer::~Observer() {
  if (m_pWatchedPtr)
    (*m_pWatchedPtr)->RemoveObserver(this);
}

void CPDFSDK_Annot::Observer::OnAnnotDestroyed() {
  ASSERT(m_pWatchedPtr);
  *m_pWatchedPtr = nullptr;
  m_pWatchedPtr = nullptr;
}

CPDFSDK_Annot::CPDFSDK_Annot(CPDFSDK_PageView* pPageView)
    : m_pPageView(pPageView), m_bSelected(FALSE) {}

CPDFSDK_Annot::~CPDFSDK_Annot() {
  for (auto* pObserver : m_Observers)
    pObserver->OnAnnotDestroyed();
}

void CPDFSDK_Annot::AddObserver(Observer* pObserver) {
  ASSERT(!pdfium::ContainsKey(m_Observers, pObserver));
  m_Observers.insert(pObserver);
}

void CPDFSDK_Annot::RemoveObserver(Observer* pObserver) {
  ASSERT(pdfium::ContainsKey(m_Observers, pObserver));
  m_Observers.erase(pObserver);
}

#ifdef PDF_ENABLE_XFA

FX_BOOL CPDFSDK_Annot::IsXFAField() {
  return FALSE;
}

CXFA_FFWidget* CPDFSDK_Annot::GetXFAWidget() const {
  return nullptr;
}

CPDFXFA_Page* CPDFSDK_Annot::GetPDFXFAPage() {
  return m_pPageView ? m_pPageView->GetPDFXFAPage() : nullptr;
}

#endif  // PDF_ENABLE_XFA

FX_FLOAT CPDFSDK_Annot::GetMinWidth() const {
  return kMinWidth;
}

FX_FLOAT CPDFSDK_Annot::GetMinHeight() const {
  return kMinHeight;
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

void CPDFSDK_Annot::Annot_OnDraw(CFX_RenderDevice* pDevice,
                                 CFX_Matrix* pUser2Device,
                                 CPDF_RenderOptions* pOptions) {}

FX_BOOL CPDFSDK_Annot::IsSelected() {
  return m_bSelected;
}

void CPDFSDK_Annot::SetSelected(FX_BOOL bSelected) {
  m_bSelected = bSelected;
}

UnderlyingPageType* CPDFSDK_Annot::GetUnderlyingPage() {
#ifdef PDF_ENABLE_XFA
  return GetPDFXFAPage();
#else   // PDF_ENABLE_XFA
  return GetPDFPage();
#endif  // PDF_ENABLE_XFA
}

CPDF_Page* CPDFSDK_Annot::GetPDFPage() {
  return m_pPageView ? m_pPageView->GetPDFPage() : nullptr;
}
