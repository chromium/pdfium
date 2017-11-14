// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_validatedata.h"

#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/xfa_utils.h"

CXFA_ValidateData::CXFA_ValidateData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

int32_t CXFA_ValidateData::GetFormatTest() {
  return m_pNode->JSNode()->GetEnum(XFA_Attribute::FormatTest);
}

bool CXFA_ValidateData::SetNullTest(WideString wsValue) {
  const XFA_ATTRIBUTEENUMINFO* pInfo =
      XFA_GetAttributeEnumByName(wsValue.AsStringView());
  m_pNode->JSNode()->SetEnum(XFA_Attribute::NullTest,
                             pInfo ? pInfo->eName : XFA_ATTRIBUTEENUM_Disabled,
                             false);
  return true;
}

int32_t CXFA_ValidateData::GetNullTest() {
  return m_pNode->JSNode()->GetEnum(XFA_Attribute::NullTest);
}

int32_t CXFA_ValidateData::GetScriptTest() {
  return m_pNode->JSNode()->GetEnum(XFA_Attribute::ScriptTest);
}

void CXFA_ValidateData::GetMessageText(WideString& wsMessage,
                                       const WideString& wsMessageType) {
  CXFA_Node* pNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Message, false);
  if (!pNode)
    return;

  CXFA_Node* pItemNode = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pItemNode;
       pItemNode = pItemNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pItemNode->GetElementType() != XFA_Element::Text)
      continue;

    WideString wsName;
    pItemNode->JSNode()->TryCData(XFA_Attribute::Name, wsName, true);
    if (wsName.IsEmpty() || wsName == wsMessageType) {
      pItemNode->JSNode()->TryContent(wsMessage, false, true);
      return;
    }
  }
}

void CXFA_ValidateData::SetFormatMessageText(WideString wsMessage) {
  SetMessageText(wsMessage, L"formatTest");
}

void CXFA_ValidateData::GetFormatMessageText(WideString& wsMessage) {
  GetMessageText(wsMessage, L"formatTest");
}

void CXFA_ValidateData::SetNullMessageText(WideString wsMessage) {
  SetMessageText(wsMessage, L"nullTest");
}

void CXFA_ValidateData::GetNullMessageText(WideString& wsMessage) {
  GetMessageText(wsMessage, L"nullTest");
}

void CXFA_ValidateData::SetMessageText(WideString& wsMessage,
                                       const WideString& wsMessageType) {
  CXFA_Node* pNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Message, true);
  if (!pNode)
    return;

  CXFA_Node* pItemNode = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pItemNode;
       pItemNode = pItemNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pItemNode->GetElementType() != XFA_Element::Text)
      continue;

    WideString wsName;
    pItemNode->JSNode()->TryCData(XFA_Attribute::Name, wsName, true);
    if (wsName.IsEmpty() || wsName == wsMessageType) {
      pItemNode->JSNode()->SetContent(wsMessage, wsMessage, false, false, true);
      return;
    }
  }
  CXFA_Node* pTextNode = pNode->CreateSamePacketNode(XFA_Element::Text);
  pNode->InsertChild(pTextNode, nullptr);
  pTextNode->JSNode()->SetCData(XFA_Attribute::Name, wsMessageType, false,
                                false);
  pTextNode->JSNode()->SetContent(wsMessage, wsMessage, false, false, true);
}

void CXFA_ValidateData::GetScriptMessageText(WideString& wsMessage) {
  GetMessageText(wsMessage, L"scriptTest");
}

void CXFA_ValidateData::SetScriptMessageText(WideString wsMessage) {
  SetMessageText(wsMessage, L"scriptTest");
}

void CXFA_ValidateData::GetPicture(WideString& wsPicture) {
  if (CXFA_Node* pNode = m_pNode->GetChild(0, XFA_Element::Picture, false))
    pNode->JSNode()->TryContent(wsPicture, false, true);
}

CXFA_ScriptData CXFA_ValidateData::GetScriptData() {
  return CXFA_ScriptData(m_pNode->GetChild(0, XFA_Element::Script, false));
}
