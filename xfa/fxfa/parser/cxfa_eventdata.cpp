// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_eventdata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_EventData::CXFA_EventData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

int32_t CXFA_EventData::GetActivity() {
  return m_pNode->JSNode()->GetEnum(XFA_Attribute::Activity);
}

XFA_Element CXFA_EventData::GetEventType() const {
  CXFA_Node* pChild = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pChild) {
    XFA_Element eType = pChild->GetElementType();
    if (eType != XFA_Element::Extras)
      return eType;

    pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  return XFA_Element::Unknown;
}

void CXFA_EventData::GetRef(WideString& wsRef) {
  m_pNode->JSNode()->TryCData(XFA_Attribute::Ref, wsRef, true);
}

CXFA_ScriptData CXFA_EventData::GetScriptData() const {
  return CXFA_ScriptData(m_pNode->GetChild(0, XFA_Element::Script, false));
}

CXFA_SubmitData CXFA_EventData::GetSubmitData() const {
  return CXFA_SubmitData(m_pNode->GetChild(0, XFA_Element::Submit, false));
}

void CXFA_EventData::GetSignDataTarget(WideString& wsTarget) {
  CXFA_Node* pNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::SignData, true);
  if (!pNode)
    return;

  pNode->JSNode()->TryCData(XFA_Attribute::Target, wsTarget, true);
}
