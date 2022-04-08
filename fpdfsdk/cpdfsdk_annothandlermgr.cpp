// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_annothandlermgr.h"

#include <utility>

#include "core/fpdfdoc/cpdf_annot.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/ipdfsdk_annothandler.h"
#include "third_party/base/check.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/cpdfxfa_widget.h"
#endif  // PDF_ENABLE_XFA

CPDFSDK_AnnotHandlerMgr::CPDFSDK_AnnotHandlerMgr(
    std::unique_ptr<IPDFSDK_AnnotHandler> pBAAnnotHandler,
    std::unique_ptr<IPDFSDK_AnnotHandler> pWidgetHandler,
    std::unique_ptr<IPDFSDK_AnnotHandler> pXFAWidgetHandler)
    : m_pBAAnnotHandler(std::move(pBAAnnotHandler)),
      m_pWidgetHandler(std::move(pWidgetHandler)),
      m_pXFAWidgetHandler(std::move(pXFAWidgetHandler)) {
  DCHECK(m_pBAAnnotHandler);
  DCHECK(m_pWidgetHandler);
}

CPDFSDK_AnnotHandlerMgr::~CPDFSDK_AnnotHandlerMgr() = default;

void CPDFSDK_AnnotHandlerMgr::SetFormFillEnv(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  m_pBAAnnotHandler->SetFormFillEnvironment(pFormFillEnv);
  m_pWidgetHandler->SetFormFillEnvironment(pFormFillEnv);
  if (m_pXFAWidgetHandler)
    m_pXFAWidgetHandler->SetFormFillEnvironment(pFormFillEnv);
}

std::unique_ptr<CPDFSDK_Annot> CPDFSDK_AnnotHandlerMgr::NewAnnot(
    CPDF_Annot* pAnnot,
    CPDFSDK_PageView* pPageView) {
  DCHECK(pPageView);
  return GetAnnotHandlerOfType(pAnnot->GetSubtype())
      ->NewAnnot(pAnnot, pPageView);
}

IPDFSDK_AnnotHandler* CPDFSDK_AnnotHandlerMgr::GetAnnotHandler(
    CPDFSDK_Annot* pAnnot) const {
  return GetAnnotHandlerOfType(pAnnot->GetAnnotSubtype());
}

IPDFSDK_AnnotHandler* CPDFSDK_AnnotHandlerMgr::GetAnnotHandlerOfType(
    CPDF_Annot::Subtype nAnnotSubtype) const {
  if (nAnnotSubtype == CPDF_Annot::Subtype::WIDGET)
    return m_pWidgetHandler.get();

#ifdef PDF_ENABLE_XFA
  if (nAnnotSubtype == CPDF_Annot::Subtype::XFAWIDGET)
    return m_pXFAWidgetHandler.get();
#endif  // PDF_ENABLE_XFA

  return m_pBAAnnotHandler.get();
}
