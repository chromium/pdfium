// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmlelement.h"

#include <utility>

#include "core/fxcrt/cfx_widetextbuf.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/xml/cfx_xmlchardata.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

CFX_XMLElement::CFX_XMLElement(const WideString& wsTag)
    : CFX_XMLAttributeNode(wsTag) {}

CFX_XMLElement::~CFX_XMLElement() {}

FX_XMLNODETYPE CFX_XMLElement::GetType() const {
  return FX_XMLNODE_Element;
}

std::unique_ptr<CFX_XMLNode> CFX_XMLElement::Clone() {
  auto pClone = pdfium::MakeUnique<CFX_XMLElement>(GetName());
  pClone->SetAttributes(GetAttributes());

  WideString wsText;
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
  return std::move(pClone);
}

WideString CFX_XMLElement::GetLocalTagName() const {
  auto pos = GetName().Find(L':');
  return pos.has_value()
             ? GetName().Right(GetName().GetLength() - pos.value() - 1)
             : GetName();
}

WideString CFX_XMLElement::GetNamespacePrefix() const {
  auto pos = GetName().Find(L':');
  return pos.has_value() ? GetName().Left(pos.value()) : WideString();
}

WideString CFX_XMLElement::GetNamespaceURI() const {
  WideString wsAttri(L"xmlns");
  WideString wsPrefix = GetNamespacePrefix();
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
  return WideString();
}

WideString CFX_XMLElement::GetTextData() const {
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

void CFX_XMLElement::SetTextData(const WideString& wsText) {
  if (wsText.GetLength() < 1)
    return;
  InsertChildNode(new CFX_XMLText(wsText));
}
