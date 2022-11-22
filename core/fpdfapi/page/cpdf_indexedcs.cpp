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
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_2d_size.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/retain_ptr.h"
#include "third_party/base/check_op.h"
#include "third_party/base/span.h"

CPDF_IndexedCS::CPDF_IndexedCS() : CPDF_BasedCS(Family::kIndexed) {}

CPDF_IndexedCS::~CPDF_IndexedCS() = default;

const CPDF_IndexedCS* CPDF_IndexedCS::AsIndexedCS() const {
  return this;
}

uint32_t CPDF_IndexedCS::v_Load(CPDF_Document* pDoc,
                                const CPDF_Array* pArray,
                                std::set<const CPDF_Object*>* pVisited) {
  if (pArray->size() < 4)
    return 0;

  RetainPtr<const CPDF_Object> pBaseObj = pArray->GetDirectObjectAt(1);
  if (HasSameArray(pBaseObj.Get()))
    return 0;

  auto* pDocPageData = CPDF_DocPageData::FromDocument(pDoc);
  m_pBaseCS =
      pDocPageData->GetColorSpaceGuarded(pBaseObj.Get(), nullptr, pVisited);
  if (!m_pBaseCS)
    return 0;

  // The base color space cannot be a Pattern or Indexed space, according to the
  // PDF 1.7 spec, page 263.
  Family family = m_pBaseCS->GetFamily();
  if (family == Family::kIndexed || family == Family::kPattern)
    return 0;

  m_nBaseComponents = m_pBaseCS->CountComponents();
  DCHECK(m_nBaseComponents);
  m_pCompMinMax = DataVector<float>(Fx2DSizeOrDie(m_nBaseComponents, 2));
  float defvalue;
  for (uint32_t i = 0; i < m_nBaseComponents; i++) {
    m_pBaseCS->GetDefaultValue(i, &defvalue, &m_pCompMinMax[i * 2],
                               &m_pCompMinMax[i * 2 + 1]);
    m_pCompMinMax[i * 2 + 1] -= m_pCompMinMax[i * 2];
  }
  m_MaxIndex = pArray->GetIntegerAt(2);

  RetainPtr<const CPDF_Object> pTableObj = pArray->GetDirectObjectAt(3);
  if (!pTableObj)
    return 0;

  if (const CPDF_String* pString = pTableObj->AsString()) {
    m_Table = pString->GetString();
  } else if (const CPDF_Stream* pStream = pTableObj->AsStream()) {
    auto pAcc = pdfium::MakeRetain<CPDF_StreamAcc>(pdfium::WrapRetain(pStream));
    pAcc->LoadAllDataFiltered();
    m_Table = ByteStringView(pAcc->GetSpan());
  }
  return 1;
}

bool CPDF_IndexedCS::GetRGB(pdfium::span<const float> pBuf,
                            float* R,
                            float* G,
                            float* B) const {
  int32_t index = static_cast<int32_t>(pBuf[0]);
  if (index < 0 || index > m_MaxIndex)
    return false;

  DCHECK(m_nBaseComponents);
  DCHECK_EQ(m_nBaseComponents, m_pBaseCS->CountComponents());

  FX_SAFE_SIZE_T length = index;
  length += 1;
  length *= m_nBaseComponents;
  if (!length.IsValid() || length.ValueOrDie() > m_Table.GetLength()) {
    *R = 0;
    *G = 0;
    *B = 0;
    return false;
  }

  std::vector<float> comps(m_nBaseComponents);
  const uint8_t* pTable = m_Table.raw_str();
  for (uint32_t i = 0; i < m_nBaseComponents; ++i) {
    comps[i] =
        m_pCompMinMax[i * 2] +
        m_pCompMinMax[i * 2 + 1] * pTable[index * m_nBaseComponents + i] / 255;
  }
  return m_pBaseCS->GetRGB(comps, R, G, B);
}
