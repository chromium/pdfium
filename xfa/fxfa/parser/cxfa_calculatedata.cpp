// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_calculatedata.h"

#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_textdata.h"

CXFA_CalculateData::CXFA_CalculateData(CXFA_Node* pNode)
    : CXFA_DataData(pNode) {}

XFA_AttributeEnum CXFA_CalculateData::GetOverride() const {
  return m_pNode->JSNode()
      ->TryEnum(XFA_Attribute::Override, false)
      .value_or(XFA_AttributeEnum::Error);
}

CXFA_ScriptData CXFA_CalculateData::GetScriptData() const {
  return CXFA_ScriptData(m_pNode->GetChild(0, XFA_Element::Script, false));
}

WideString CXFA_CalculateData::GetMessageText() const {
  CXFA_Node* pNode = m_pNode->GetChild(0, XFA_Element::Message, false);
  if (!pNode)
    return L"";

  CXFA_TextData textData(pNode->GetChild(0, XFA_Element::Text, false));
  if (!textData.HasValidNode())
    return L"";
  return textData.GetContent();
}
