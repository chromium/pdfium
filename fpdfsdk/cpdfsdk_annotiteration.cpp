// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_annotiteration.h"

#include <algorithm>

#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/cpdfsdk_pageview.h"

CPDFSDK_AnnotIteration::CPDFSDK_AnnotIteration(CPDFSDK_PageView* pPageView) {
  // Copying ObservedPtrs is expensive, so do it once at the end.
  std::vector<CPDFSDK_Annot*> copied_list = pPageView->GetAnnotList();
  std::stable_sort(copied_list.begin(), copied_list.end(),
                   [](const CPDFSDK_Annot* p1, const CPDFSDK_Annot* p2) {
                     return p1->GetLayoutOrder() < p2->GetLayoutOrder();
                   });

  CPDFSDK_Annot* pTopMostAnnot = pPageView->GetFocusAnnot();
  if (pTopMostAnnot) {
    auto it = std::find(copied_list.begin(), copied_list.end(), pTopMostAnnot);
    if (it != copied_list.end()) {
      copied_list.erase(it);
      copied_list.insert(copied_list.begin(), pTopMostAnnot);
    }
  }

  m_List.reserve(copied_list.size());
  for (auto* pAnnot : copied_list)
    m_List.emplace_back(pAnnot);
}

CPDFSDK_AnnotIteration::~CPDFSDK_AnnotIteration() = default;
