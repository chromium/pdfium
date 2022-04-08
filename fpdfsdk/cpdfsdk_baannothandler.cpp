// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_baannothandler.h"

#include <memory>
#include <vector>

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfdoc/cpdf_action.h"
#include "core/fpdfdoc/cpdf_interactiveform.h"
#include "fpdfsdk/cpdfsdk_actionhandler.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/cpdfsdk_baannot.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/formfiller/cffl_formfield.h"
#include "public/fpdf_fwlevent.h"
#include "third_party/base/check.h"
#include "third_party/base/containers/contains.h"

CPDFSDK_BAAnnotHandler::CPDFSDK_BAAnnotHandler() = default;

CPDFSDK_BAAnnotHandler::~CPDFSDK_BAAnnotHandler() = default;

std::unique_ptr<CPDFSDK_Annot> CPDFSDK_BAAnnotHandler::NewAnnot(
    CPDF_Annot* pAnnot,
    CPDFSDK_PageView* pPageView) {
  CHECK(pPageView);
  return std::make_unique<CPDFSDK_BAAnnot>(pAnnot, pPageView);
}

bool CPDFSDK_BAAnnotHandler::SetIndexSelected(
    ObservedPtr<CPDFSDK_Annot>& pAnnot,
    int index,
    bool selected) {
  return false;
}

bool CPDFSDK_BAAnnotHandler::IsIndexSelected(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                                             int index) {
  return false;
}

WideString CPDFSDK_BAAnnotHandler::GetText(CPDFSDK_Annot* pAnnot) {
  return WideString();
}

WideString CPDFSDK_BAAnnotHandler::GetSelectedText(CPDFSDK_Annot* pAnnot) {
  return WideString();
}

void CPDFSDK_BAAnnotHandler::ReplaceSelection(CPDFSDK_Annot* pAnnot,
                                              const WideString& text) {}

bool CPDFSDK_BAAnnotHandler::SelectAllText(CPDFSDK_Annot* pAnnot) {
  return false;
}
