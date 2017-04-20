// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmlelement.h"

#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/xml/cfx_xmlchardata.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

CFX_XMLElement::CFX_XMLElement(const CFX_WideString& wsTag)
    : CFX_XMLAttributeNode(wsTag) {}

CFX_XMLElement::~CFX_XMLElement() {}

FX_XMLNODETYPE CFX_XMLElement::GetType() const {
  return FX_XMLNODE_Element;
}

std::unique_ptr<CFX_XMLNode> CFX_XMLElement::Clone() {
  auto pClone = pdfium::MakeUnique<CFX_XMLElement>(GetName());
  pClone->SetAttributes(GetAttributes());

  CFX_WideString wsText;
  CFX_XMLNode* pChild = m_pChild;
  while (pChild) {
    switch (pChild->GetType()) {
      case FX_XMLNODE_Text:
        wsText += static_cast<CFX_XMLText*>(pChild)->GetText();
        break;
      default:
        break;
    }
    pChild = pChild->m_pNext;
  }
  pClone->SetTextData(wsText);
  return pClone;
}

CFX_WideString CFX_XMLElement::GetLocalTagName() const {
  FX_STRSIZE iFind = GetName().Find(L':', 0);
  if (iFind < 0)
    return GetName();
  return GetName().Right(GetName().GetLength() - iFind - 1);
}

CFX_WideString CFX_XMLElement::GetNamespacePrefix() const {
  FX_STRSIZE iFind = GetName().Find(L':', 0);
  if (iFind < 0)
    return CFX_WideString();
  return GetName().Left(iFind);
}

CFX_WideString CFX_XMLElement::GetNamespaceURI() const {
  CFX_WideString wsAttri(L"xmlns");
  CFX_WideString wsPrefix = GetNamespacePrefix();
  if (wsPrefix.GetLength() > 0) {
    wsAttri += L":";
    wsAttri += wsPrefix;
  }

  auto* pNode = static_cast<const CFX_XMLNode*>(this);
  while (pNode) {
    if (pNode->GetType() != FX_XMLNODE_Element)
      break;

    auto* pElement = static_cast<const CFX_XMLElement*>(pNode);
    if (!pElement->HasAttribute(wsAttri)) {
      pNode = pNode->GetNodeItem(CFX_XMLNode::Parent);
      continue;
    }
    return pElement->GetString(wsAttri);
  }
  return CFX_WideString();
}

CFX_WideString CFX_XMLElement::GetTextData() const {
  CFX_WideTextBuf buffer;
  CFX_XMLNode* pChild = m_pChild;
  while (pChild) {
    switch (pChild->GetType()) {
      case FX_XMLNODE_Text:
      case FX_XMLNODE_CharData:
        buffer << static_cast<CFX_XMLText*>(pChild)->GetText();
        break;
      default:
        break;
    }
    pChild = pChild->m_pNext;
  }
  return buffer.MakeString();
}

void CFX_XMLElement::SetTextData(const CFX_WideString& wsText) {
  if (wsText.GetLength() < 1)
    return;
  InsertChildNode(new CFX_XMLText(wsText));
}
