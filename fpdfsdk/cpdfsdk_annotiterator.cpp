// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_annotiterator.h"

#include <algorithm>

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxcrt/containers/adapters.h"
#include "core/fxcrt/containers/contains.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/stl_util.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/cpdfsdk_widget.h"

namespace {

CFX_FloatRect GetAnnotRect(const CPDFSDK_Annot* pAnnot) {
  return pAnnot->GetPDFAnnot()->GetRect();
}

bool CompareByLeftAscending(const CPDFSDK_Annot* p1, const CPDFSDK_Annot* p2) {
  return GetAnnotRect(p1).left < GetAnnotRect(p2).left;
}

bool CompareByTopDescending(const CPDFSDK_Annot* p1, const CPDFSDK_Annot* p2) {
  return GetAnnotRect(p1).top > GetAnnotRect(p2).top;
}

}  // namespace

CPDFSDK_AnnotIterator::CPDFSDK_AnnotIterator(
    CPDFSDK_PageView* pPageView,
    const std::vector<CPDF_Annot::Subtype>& subtypes_to_iterate)
    : page_view_(pPageView),
      subtypes_(subtypes_to_iterate),
      tab_order_(GetTabOrder(pPageView)) {
  GenerateResults();
}

CPDFSDK_AnnotIterator::~CPDFSDK_AnnotIterator() = default;

CPDFSDK_Annot* CPDFSDK_AnnotIterator::GetFirstAnnot() {
  return annots_.empty() ? nullptr : annots_.front();
}

CPDFSDK_Annot* CPDFSDK_AnnotIterator::GetLastAnnot() {
  return annots_.empty() ? nullptr : annots_.back();
}

CPDFSDK_Annot* CPDFSDK_AnnotIterator::GetNextAnnot(CPDFSDK_Annot* pAnnot) {
  auto iter = std::ranges::find(annots_, pAnnot);
  if (iter == annots_.end()) {
    return nullptr;
  }
  ++iter;
  if (iter == annots_.end()) {
    return nullptr;
  }
  return *iter;
}

CPDFSDK_Annot* CPDFSDK_AnnotIterator::GetPrevAnnot(CPDFSDK_Annot* pAnnot) {
  auto iter = std::ranges::find(annots_, pAnnot);
  if (iter == annots_.begin() || iter == annots_.end()) {
    return nullptr;
  }
  return *(--iter);
}

void CPDFSDK_AnnotIterator::CollectAnnots(
    std::vector<UnownedPtr<CPDFSDK_Annot>>* pArray) {
  for (auto* pAnnot : page_view_->GetAnnotList()) {
    if (pdfium::Contains(subtypes_, pAnnot->GetAnnotSubtype())) {
      CPDFSDK_Widget* pWidget = ToCPDFSDKWidget(pAnnot);
      if (!pWidget || !pWidget->IsSignatureWidget()) {
        pArray->emplace_back(pAnnot);
      }
    }
  }
}

CFX_FloatRect CPDFSDK_AnnotIterator::AddToAnnotsList(
    std::vector<UnownedPtr<CPDFSDK_Annot>>& sa,
    size_t idx) {
  CPDFSDK_Annot* pLeftTopAnnot = sa[idx];
  CFX_FloatRect rcLeftTop = GetAnnotRect(pLeftTopAnnot);
  annots_.emplace_back(pLeftTopAnnot);
  sa.erase(sa.begin() + idx);
  return rcLeftTop;
}

void CPDFSDK_AnnotIterator::AddSelectedToAnnots(
    std::vector<UnownedPtr<CPDFSDK_Annot>>& sa,
    pdfium::span<const size_t> aSelect) {
  for (size_t select_idx : aSelect) {
    annots_.emplace_back(sa[select_idx]);
  }

  for (size_t select_idx : pdfium::Reversed(aSelect)) {
    sa.erase(sa.begin() + select_idx);
  }
}

// static
CPDFSDK_AnnotIterator::TabOrder CPDFSDK_AnnotIterator::GetTabOrder(
    CPDFSDK_PageView* pPageView) {
  CPDF_Page* pPDFPage = pPageView->GetPDFPage();
  ByteString sTabs = pPDFPage->GetDict()->GetByteStringFor("Tabs");
  if (sTabs == "R") {
    return TabOrder::kRow;
  }
  if (sTabs == "C") {
    return TabOrder::kColumn;
  }
  return TabOrder::kStructure;
}

void CPDFSDK_AnnotIterator::GenerateResults() {
  switch (tab_order_) {
    case TabOrder::kStructure:
      CollectAnnots(&annots_);
      break;

    case TabOrder::kRow: {
      std::vector<UnownedPtr<CPDFSDK_Annot>> sa;
      CollectAnnots(&sa);
      std::sort(sa.begin(), sa.end(), CompareByLeftAscending);

      while (!sa.empty()) {
        int nLeftTopIndex = -1;
        float fTop = 0.0f;
        for (int i = fxcrt::CollectionSize<int>(sa) - 1; i >= 0; i--) {
          CFX_FloatRect rcAnnot = GetAnnotRect(sa[i]);
          if (rcAnnot.top > fTop) {
            nLeftTopIndex = i;
            fTop = rcAnnot.top;
          }
        }
        if (nLeftTopIndex < 0) {
          continue;
        }

        CFX_FloatRect rcLeftTop = AddToAnnotsList(sa, nLeftTopIndex);

        std::vector<size_t> aSelect;
        for (size_t i = 0; i < sa.size(); ++i) {
          CFX_FloatRect rcAnnot = GetAnnotRect(sa[i]);
          float fCenterY = (rcAnnot.top + rcAnnot.bottom) / 2.0f;
          if (fCenterY > rcLeftTop.bottom && fCenterY < rcLeftTop.top) {
            aSelect.push_back(i);
          }
        }
        AddSelectedToAnnots(sa, aSelect);
      }
      break;
    }

    case TabOrder::kColumn: {
      std::vector<UnownedPtr<CPDFSDK_Annot>> sa;
      CollectAnnots(&sa);
      std::sort(sa.begin(), sa.end(), CompareByTopDescending);

      while (!sa.empty()) {
        int nLeftTopIndex = -1;
        float fLeft = -1.0f;
        for (int i = fxcrt::CollectionSize<int>(sa) - 1; i >= 0; --i) {
          CFX_FloatRect rcAnnot = GetAnnotRect(sa[i]);
          if (fLeft < 0) {
            nLeftTopIndex = 0;
            fLeft = rcAnnot.left;
          } else if (rcAnnot.left < fLeft) {
            nLeftTopIndex = i;
            fLeft = rcAnnot.left;
          }
        }
        if (nLeftTopIndex < 0) {
          continue;
        }

        CFX_FloatRect rcLeftTop = AddToAnnotsList(sa, nLeftTopIndex);

        std::vector<size_t> aSelect;
        for (size_t i = 0; i < sa.size(); ++i) {
          CFX_FloatRect rcAnnot = GetAnnotRect(sa[i]);
          float fCenterX = (rcAnnot.left + rcAnnot.right) / 2.0f;
          if (fCenterX > rcLeftTop.left && fCenterX < rcLeftTop.right) {
            aSelect.push_back(i);
          }
        }
        AddSelectedToAnnots(sa, aSelect);
      }
      break;
    }
  }
}
