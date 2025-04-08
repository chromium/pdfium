// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_CPDFSDK_ANNOTITERATOR_H_
#define FPDFSDK_CPDFSDK_ANNOTITERATOR_H_

#include <vector>

#include "core/fpdfdoc/cpdf_annot.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/unowned_ptr.h"

class CPDFSDK_Annot;
class CPDFSDK_PageView;

class CPDFSDK_AnnotIterator {
 public:
  CPDFSDK_AnnotIterator(
      CPDFSDK_PageView* pPageView,
      const std::vector<CPDF_Annot::Subtype>& subtypes_to_iterate);
  ~CPDFSDK_AnnotIterator();

  CPDFSDK_Annot* GetFirstAnnot();
  CPDFSDK_Annot* GetLastAnnot();
  CPDFSDK_Annot* GetNextAnnot(CPDFSDK_Annot* pAnnot);
  CPDFSDK_Annot* GetPrevAnnot(CPDFSDK_Annot* pAnnot);

 private:
  enum class TabOrder : uint8_t { kStructure = 0, kRow, kColumn };

  static TabOrder GetTabOrder(CPDFSDK_PageView* pPageView);

  void GenerateResults();
  void CollectAnnots(std::vector<UnownedPtr<CPDFSDK_Annot>>* pArray);
  CFX_FloatRect AddToAnnotsList(std::vector<UnownedPtr<CPDFSDK_Annot>>& sa,
                                size_t idx);
  void AddSelectedToAnnots(std::vector<UnownedPtr<CPDFSDK_Annot>>& sa,
                           pdfium::span<const size_t> aSelect);

  UnownedPtr<CPDFSDK_PageView> const page_view_;
  const std::vector<CPDF_Annot::Subtype> subtypes_;
  const TabOrder tab_order_;
  std::vector<UnownedPtr<CPDFSDK_Annot>> annots_;
};

#endif  // FPDFSDK_CPDFSDK_ANNOTITERATOR_H_
