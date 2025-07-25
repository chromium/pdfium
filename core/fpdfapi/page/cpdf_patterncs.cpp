// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_patterncs.h"

#include <optional>

#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fxcrt/notreached.h"

CPDF_PatternCS::CPDF_PatternCS() : CPDF_BasedCS(Family::kPattern) {}

CPDF_PatternCS::~CPDF_PatternCS() = default;

void CPDF_PatternCS::InitializeStockPattern() {
  SetComponentsForStockCS(1);
}

uint32_t CPDF_PatternCS::v_Load(CPDF_Document* doc,
                                const CPDF_Array* pArray,
                                std::set<const CPDF_Object*>* pVisited) {
  RetainPtr<const CPDF_Object> pBaseCS = pArray->GetDirectObjectAt(1);
  if (HasSameArray(pBaseCS.Get())) {
    return 0;
  }

  auto* pDocPageData = CPDF_DocPageData::FromDocument(doc);
  base_cs_ =
      pDocPageData->GetColorSpaceGuarded(pBaseCS.Get(), nullptr, pVisited);
  if (!base_cs_) {
    return 1;
  }

  if (base_cs_->GetFamily() == Family::kPattern) {
    return 0;
  }

  if (base_cs_->ComponentCount() > kMaxPatternColorComps) {
    return 0;
  }

  return base_cs_->ComponentCount() + 1;
}

std::optional<FX_RGB_STRUCT<float>> CPDF_PatternCS::GetRGB(
    pdfium::span<const float> pBuf) const {
  NOTREACHED();
}

const CPDF_PatternCS* CPDF_PatternCS::AsPatternCS() const {
  return this;
}

std::optional<FX_RGB_STRUCT<float>> CPDF_PatternCS::GetPatternRGB(
    const PatternValue& value) const {
  if (!base_cs_) {
    return std::nullopt;
  }

  return base_cs_->GetRGB(value.GetComps());
}
