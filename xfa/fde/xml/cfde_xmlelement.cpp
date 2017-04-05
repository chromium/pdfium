// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/xml/cfde_xmlelement.h"

#include "core/fxcrt/fx_ext.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/xml/cfde_xmlchardata.h"
#include "xfa/fde/xml/cfde_xmltext.h"

CFDE_XMLElement::CFDE_XMLElement(const CFX_WideString& wsTag)
    : CFDE_XMLAttributeNode(wsTag) {}

CFDE_XMLElement::~CFDE_XMLElement() {}

FDE_XMLNODETYPE CFDE_XMLElement::GetType() const {
  return FDE_XMLNODE_Element;
}

std::unique_ptr<CFDE_XMLNode> CFDE_XMLElement::Clone() {
  auto pClone = pdfium::MakeUnique<CFDE_XMLElement>(GetName());
  pClone->SetAttributes(GetAttributes());

  CFX_WideString wsText;
  CFDE_XMLNode* pChild = m_pChild;
  while (pChild) {
    switch (pChild->GetType()) {
      case FDE_XMLNODE_Text:
        wsText += static_cast<CFDE_XMLText*>(pChild)->GetText();
        break;
      default:
        break;
    }
    pChild = pChild->m_pNext;
  }
  pClone->SetTextData(wsText);
  return pClone;
}

CFX_WideString CFDE_XMLElement::GetLocalTagName() const {
  FX_STRSIZE iFind = GetName().Find(L':', 0);
  if (iFind < 0)
    return GetName();
  return GetName().Right(GetName().GetLength() - iFind - 1);
}

CFX_WideString CFDE_XMLElement::GetNamespacePrefix() const {
  FX_STRSIZE iFind = GetName().Find(L':', 0);
  if (iFind < 0)
    return CFX_WideString();
  return GetName().Left(iFind);
}

CFX_WideString CFDE_XMLElement::GetNamespaceURI() const {
  CFX_WideString wsAttri(L"xmlns");
  CFX_WideString wsPrefix = GetNamespacePrefix();
  if (wsPrefix.GetLength() > 0) {
    wsAttri += L":";
    wsAttri += wsPrefix;
  }

  auto* pNode = static_cast<const CFDE_XMLNode*>(this);
  while (pNode) {
    if (pNode->GetType() != FDE_XMLNODE_Element)
      break;

    auto* pElement = static_cast<const CFDE_XMLElement*>(pNode);
    if (!pElement->HasAttribute(wsAttri)) {
      pNode = pNode->GetNodeItem(CFDE_XMLNode::Parent);
      continue;
    }
    return pElement->GetString(wsAttri);
  }
  return CFX_WideString();
}

CFX_WideString CFDE_XMLElement::GetTextData() const {
  CFX_WideTextBuf buffer;
  CFDE_XMLNode* pChild = m_pChild;
  while (pChild) {
    switch (pChild->GetType()) {
      case FDE_XMLNODE_Text:
      case FDE_XMLNODE_CharData:
        buffer << static_cast<CFDE_XMLText*>(pChild)->GetText();
        break;
      default:
        break;
    }
    pChild = pChild->m_pNext;
  }
  return buffer.MakeString();
}

void CFDE_XMLElement::SetTextData(const CFX_WideString& wsText) {
  if (wsText.GetLength() < 1)
    return;
  InsertChildNode(new CFDE_XMLText(wsText));
}
