// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_occurdata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_OccurData::CXFA_OccurData(CXFA_Node* pNode) : CXFA_Data(pNode) {}

int32_t CXFA_OccurData::GetMax() {
  int32_t iMax = 1;
  if (m_pNode) {
    if (!m_pNode->JSNode()->TryInteger(XFA_ATTRIBUTE_Max, iMax, true))
      iMax = GetMin();
  }
  return iMax;
}

int32_t CXFA_OccurData::GetMin() {
  int32_t iMin = 1;
  if (m_pNode) {
    if (!m_pNode->JSNode()->TryInteger(XFA_ATTRIBUTE_Min, iMin, true) ||
        iMin < 0)
      iMin = 1;
  }
  return iMin;
}

bool CXFA_OccurData::GetOccurInfo(int32_t& iMin,
                                  int32_t& iMax,
                                  int32_t& iInit) {
  if (!m_pNode)
    return false;
  if (!m_pNode->JSNode()->TryInteger(XFA_ATTRIBUTE_Min, iMin, false) ||
      iMin < 0)
    iMin = 1;
  if (!m_pNode->JSNode()->TryInteger(XFA_ATTRIBUTE_Max, iMax, false)) {
    if (iMin == 0)
      iMax = 1;
    else
      iMax = iMin;
  }
  if (!m_pNode->JSNode()->TryInteger(XFA_ATTRIBUTE_Initial, iInit, false) ||
      iInit < iMin) {
    iInit = iMin;
  }
  return true;
}

void CXFA_OccurData::SetMax(int32_t iMax) {
  iMax = (iMax != -1 && iMax < 1) ? 1 : iMax;
  m_pNode->JSNode()->SetInteger(XFA_ATTRIBUTE_Max, iMax, false);
  int32_t iMin = GetMin();
  if (iMax != -1 && iMax < iMin) {
    iMin = iMax;
    m_pNode->JSNode()->SetInteger(XFA_ATTRIBUTE_Min, iMin, false);
  }
}

void CXFA_OccurData::SetMin(int32_t iMin) {
  iMin = (iMin < 0) ? 1 : iMin;
  m_pNode->JSNode()->SetInteger(XFA_ATTRIBUTE_Min, iMin, false);
  int32_t iMax = GetMax();
  if (iMax > 0 && iMax < iMin) {
    iMax = iMin;
    m_pNode->JSNode()->SetInteger(XFA_ATTRIBUTE_Max, iMax, false);
  }
}
