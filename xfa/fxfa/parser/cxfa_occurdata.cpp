// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_occurdata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_OccurData::CXFA_OccurData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

int32_t CXFA_OccurData::GetMax() {
  if (!m_pNode)
    return 1;

  pdfium::Optional<int32_t> max =
      m_pNode->JSNode()->TryInteger(XFA_Attribute::Max, true);
  return max ? *max : GetMin();
}

int32_t CXFA_OccurData::GetMin() {
  if (!m_pNode)
    return 1;

  pdfium::Optional<int32_t> min =
      m_pNode->JSNode()->TryInteger(XFA_Attribute::Min, true);
  return min && *min >= 0 ? *min : 1;
}

bool CXFA_OccurData::GetOccurInfo(int32_t& iMin,
                                  int32_t& iMax,
                                  int32_t& iInit) {
  if (!m_pNode)
    return false;

  iMin = GetMin();
  iMax = GetMax();

  pdfium::Optional<int32_t> init =
      m_pNode->JSNode()->TryInteger(XFA_Attribute::Initial, false);
  iInit = init && *init >= iMin ? *init : iMin;

  return true;
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
