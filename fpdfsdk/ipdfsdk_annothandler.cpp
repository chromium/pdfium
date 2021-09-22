// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/ipdfsdk_annothandler.h"

#include "third_party/base/notreached.h"

IPDFSDK_AnnotHandler::IPDFSDK_AnnotHandler() = default;

IPDFSDK_AnnotHandler::~IPDFSDK_AnnotHandler() = default;

#ifdef PDF_ENABLE_XFA
std::unique_ptr<CPDFSDK_Annot> IPDFSDK_AnnotHandler::NewAnnotForXFA(
    CXFA_FFWidget* pFFWidget,
    CPDFSDK_PageView* pPageView) {
  NOTREACHED();
  return nullptr;
}

bool IPDFSDK_AnnotHandler::OnXFAChangedFocus(
    ObservedPtr<CPDFSDK_Annot>& pNewAnnot) {
  NOTREACHED();
  return false;
}
#endif
