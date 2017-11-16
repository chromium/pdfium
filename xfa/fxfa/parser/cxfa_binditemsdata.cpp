// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_binditemsdata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_BindItemsData::CXFA_BindItemsData(CXFA_Node* pNode)
    : CXFA_DataData(pNode) {}

void CXFA_BindItemsData::GetLabelRef(WideString& wsLabelRef) {
  wsLabelRef = m_pNode->JSNode()->GetCData(XFA_Attribute::LabelRef);
}

void CXFA_BindItemsData::GetValueRef(WideString& wsValueRef) {
  wsValueRef = m_pNode->JSNode()->GetCData(XFA_Attribute::ValueRef);
}

void CXFA_BindItemsData::GetRef(WideString& wsRef) {
  wsRef = m_pNode->JSNode()->GetCData(XFA_Attribute::Ref);
}

bool CXFA_BindItemsData::SetConnection(const WideString& wsConnection) {
  return m_pNode->JSNode()->SetCData(XFA_Attribute::Connection, wsConnection,
                                     false, false);
}
