// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/page/cpdf_indexedcs.h"

#include <set>
#include <vector>

#include "core/fpdfapi/page/cpdf_colorspace.h"
#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"

CPDF_IndexedCS::CPDF_IndexedCS() : CPDF_BasedCS(Family::kIndexed) {}

CPDF_IndexedCS::~CPDF_IndexedCS() = default;

const CPDF_IndexedCS* CPDF_IndexedCS::AsIndexedCS() const {
  return this;
}

uint32_t CPDF_IndexedCS::v_Load(CPDF_Document* pDoc,
                                const CPDF_Array* pArray,
                                std::set<const CPDF_Object*>* pVisited) {
  if (pArray->size() < 4) {
    return 0;
  }

  RetainPtr<const CPDF_Object> pBaseObj = pArray->GetDirectObjectAt(1);
  if (HasSameArray(pBaseObj.Get())) {
    return 0;
  }

  auto* pDocPageData = CPDF_DocPageData::FromDocument(pDoc);
  m_pBaseCS =
      pDocPageData->GetColorSpaceGuarded(pBaseObj.Get(), nullptr, pVisited);
  if (!m_pBaseCS) {
    return 0;
  }

  // The base color space cannot be a Pattern or Indexed space, according to ISO
  // 32000-1:2008 section 8.6.6.3.
  Family family = m_pBaseCS->GetFamily();
  if (family == Family::kIndexed || family == Family::kPattern) {
    return 0;
  }

  base_component_count_ = m_pBaseCS->ComponentCount();
  DCHECK(base_component_count_);
  component_min_max_ = DataVector<IndexedColorMinMax>(base_component_count_);
  float defvalue;
  for (uint32_t i = 0; i < component_min_max_.size(); i++) {
    IndexedColorMinMax& comp = component_min_max_[i];
    m_pBaseCS->GetDefaultValue(i, &defvalue, &comp.min, &comp.max);
    comp.max -= comp.min;
  }

  // ISO 32000-1:2008 section 8.6.6.3 says the maximum value is 255.
  max_index_ = pArray->GetIntegerAt(2);
  if (max_index_ < 0 || max_index_ > 255) {
    return 0;
  }

  RetainPtr<const CPDF_Object> pTableObj = pArray->GetDirectObjectAt(3);
  if (!pTableObj) {
    return 0;
  }

  if (const CPDF_String* pString = pTableObj->AsString()) {
    lookup_table_ = pString->GetString();
  } else if (const CPDF_Stream* pStream = pTableObj->AsStream()) {
    auto pAcc = pdfium::MakeRetain<CPDF_StreamAcc>(pdfium::WrapRetain(pStream));
    pAcc->LoadAllDataFiltered();
    lookup_table_ = ByteStringView(pAcc->GetSpan());
  }
  return 1;
}

bool CPDF_IndexedCS::GetRGB(pdfium::span<const float> pBuf,
                            float* R,
                            float* G,
                            float* B) const {
  int32_t index = static_cast<int32_t>(pBuf[0]);
  if (index < 0 || index > max_index_) {
    return false;
  }

  DCHECK(base_component_count_);
  DCHECK_EQ(base_component_count_, m_pBaseCS->ComponentCount());

  FX_SAFE_SIZE_T length = index;
  length += 1;
  length *= base_component_count_;
  if (!length.IsValid() || length.ValueOrDie() > lookup_table_.GetLength()) {
    *R = 0;
    *G = 0;
    *B = 0;
    return false;
  }

  std::vector<float> comps(base_component_count_);
  pdfium::span<const uint8_t> pTable = lookup_table_.unsigned_span();
  for (uint32_t i = 0; i < base_component_count_; ++i) {
    comps[i] = component_min_max_[i].min +
               component_min_max_[i].max *
                   pTable[index * base_component_count_ + i] / 255;
  }
  return m_pBaseCS->GetRGB(comps, R, G, B);
}
