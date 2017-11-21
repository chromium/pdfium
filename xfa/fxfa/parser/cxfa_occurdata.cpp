// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_occurdata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_OccurData::CXFA_OccurData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

int32_t CXFA_OccurData::GetMax() const {
  if (!m_pNode)
    return 1;

  pdfium::Optional<int32_t> max =
      m_pNode->JSNode()->TryInteger(XFA_Attribute::Max, true);
  return max ? *max : GetMin();
}

int32_t CXFA_OccurData::GetMin() const {
  if (!m_pNode)
    return 1;

  pdfium::Optional<int32_t> min =
      m_pNode->JSNode()->TryInteger(XFA_Attribute::Min, true);
  return min && *min >= 0 ? *min : 1;
}

std::tuple<int32_t, int32_t, int32_t> CXFA_OccurData::GetOccurInfo() const {
  ASSERT(m_pNode);

  int32_t iMin = GetMin();
  int32_t iMax = GetMax();

  pdfium::Optional<int32_t> init =
      m_pNode->JSNode()->TryInteger(XFA_Attribute::Initial, false);
  return {iMin, iMax, init && *init >= iMin ? *init : iMin};
}

void CXFA_OccurData::SetMax(int32_t iMax) {
  iMax = (iMax != -1 && iMax < 1) ? 1 : iMax;
  m_pNode->JSNode()->SetInteger(XFA_Attribute::Max, iMax, false);

  int32_t iMin = GetMin();
  if (iMax != -1 && iMax < iMin) {
    iMin = iMax;
    m_pNode->JSNode()->SetInteger(XFA_Attribute::Min, iMin, false);
  }
}

void CXFA_OccurData::SetMin(int32_t iMin) {
  iMin = (iMin < 0) ? 1 : iMin;
  m_pNode->JSNode()->SetInteger(XFA_Attribute::Min, iMin, false);

  int32_t iMax = GetMax();
  if (iMax > 0 && iMax < iMin) {
    iMax = iMin;
    m_pNode->JSNode()->SetInteger(XFA_Attribute::Max, iMax, false);
  }
}
