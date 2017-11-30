// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_validatedata.h"

#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

constexpr wchar_t kFormatTest[] = L"formatTest";
constexpr wchar_t kNullTest[] = L"nullTest";
constexpr wchar_t kScriptTest[] = L"scriptTest";

}  // namespace

CXFA_ValidateData::CXFA_ValidateData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

XFA_AttributeEnum CXFA_ValidateData::GetFormatTest() const {
  return m_pNode->JSNode()->GetEnum(XFA_Attribute::FormatTest);
}

void CXFA_ValidateData::SetNullTest(const WideString& wsValue) {
  pdfium::Optional<XFA_AttributeEnum> item =
      CXFA_Node::NameToAttributeEnum(wsValue.AsStringView());
  m_pNode->JSNode()->SetEnum(XFA_Attribute::NullTest,
                             item ? *item : XFA_AttributeEnum::Disabled, false);
}

XFA_AttributeEnum CXFA_ValidateData::GetNullTest() const {
  return m_pNode->JSNode()->GetEnum(XFA_Attribute::NullTest);
}

XFA_AttributeEnum CXFA_ValidateData::GetScriptTest() const {
  return m_pNode->JSNode()->GetEnum(XFA_Attribute::ScriptTest);
}

WideString CXFA_ValidateData::GetMessageText(
    const WideString& wsMessageType) const {
  CXFA_Node* pNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Message, false);
  if (!pNode)
    return L"";

  for (CXFA_Node* pItemNode = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
       pItemNode;
       pItemNode = pItemNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pItemNode->GetElementType() != XFA_Element::Text)
      continue;

    WideString wsName = pItemNode->JSNode()->GetCData(XFA_Attribute::Name);
    if (wsName.IsEmpty() || wsName == wsMessageType)
      return pItemNode->JSNode()->GetContent(false);
  }
  return L"";
}

void CXFA_ValidateData::SetFormatMessageText(const WideString& wsMessage) {
  SetMessageText(kFormatTest, wsMessage);
}

WideString CXFA_ValidateData::GetFormatMessageText() const {
  return GetMessageText(kFormatTest);
}

void CXFA_ValidateData::SetNullMessageText(const WideString& wsMessage) {
  SetMessageText(kNullTest, wsMessage);
}

WideString CXFA_ValidateData::GetNullMessageText() const {
  return GetMessageText(kNullTest);
}

WideString CXFA_ValidateData::GetScriptMessageText() const {
  return GetMessageText(kScriptTest);
}

void CXFA_ValidateData::SetScriptMessageText(const WideString& wsMessage) {
  SetMessageText(kScriptTest, wsMessage);
}

void CXFA_ValidateData::SetMessageText(const WideString& wsMessageType,
                                       const WideString& wsMessage) {
  CXFA_Node* pNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Message, true);
  if (!pNode)
    return;

  for (CXFA_Node* pItemNode = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
       pItemNode;
       pItemNode = pItemNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pItemNode->GetElementType() != XFA_Element::Text)
      continue;

    WideString wsName = pItemNode->JSNode()->GetCData(XFA_Attribute::Name);
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

WideString CXFA_ValidateData::GetPicture() const {
  CXFA_Node* pNode = m_pNode->GetChild(0, XFA_Element::Picture, false);
  if (pNode)
    return pNode->JSNode()->GetContent(false);
  return L"";
}

CXFA_ScriptData CXFA_ValidateData::GetScriptData() const {
  return CXFA_ScriptData(m_pNode->GetChild(0, XFA_Element::Script, false));
}
