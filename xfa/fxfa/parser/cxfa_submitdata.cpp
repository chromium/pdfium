// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_submitdata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_SubmitData::CXFA_SubmitData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

bool CXFA_SubmitData::IsSubmitEmbedPDF() {
  return m_pNode->JSNode()->GetBoolean(XFA_Attribute::EmbedPDF);
}

int32_t CXFA_SubmitData::GetSubmitFormat() {
  return m_pNode->JSNode()->GetEnum(XFA_Attribute::Format);
}

void CXFA_SubmitData::GetSubmitTarget(WideString& wsTarget) {
  wsTarget = m_pNode->JSNode()->GetCData(XFA_Attribute::Target);
}

void CXFA_SubmitData::GetSubmitXDPContent(WideString& wsContent) {
  wsContent = m_pNode->JSNode()->GetCData(XFA_Attribute::XdpContent);
}
