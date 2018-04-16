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
    : CFX_XMLNode(), name_(wsTag) {
  ASSERT(!name_.IsEmpty());
}

CFX_XMLElement::~CFX_XMLElement() = default;

FX_XMLNODETYPE CFX_XMLElement::GetType() const {
  return FX_XMLNODE_Element;
}

std::unique_ptr<CFX_XMLNode> CFX_XMLElement::Clone() {
  auto pClone = pdfium::MakeUnique<CFX_XMLElement>(name_);
  pClone->attrs_ = attrs_;

  WideString wsText;
  for (CFX_XMLNode* pChild = GetFirstChild(); pChild;
       pChild = pChild->GetNextSibling()) {
    if (pChild->GetType() == FX_XMLNODE_Text)
      wsText += static_cast<CFX_XMLText*>(pChild)->GetText();
  }
  pClone->SetTextData(wsText);
  return std::move(pClone);
}

WideString CFX_XMLElement::GetLocalTagName() const {
  auto pos = name_.Find(L':');
  return pos.has_value() ? name_.Right(name_.GetLength() - pos.value() - 1)
                         : name_;
}

WideString CFX_XMLElement::GetNamespacePrefix() const {
  auto pos = name_.Find(L':');
  return pos.has_value() ? name_.Left(pos.value()) : WideString();
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
      pNode = pNode->GetParent();
      continue;
    }
    return pElement->GetAttribute(wsAttri);
  }
  return WideString();
}

WideString CFX_XMLElement::GetTextData() const {
  CFX_WideTextBuf buffer;

  for (CFX_XMLNode* pChild = GetFirstChild(); pChild;
       pChild = pChild->GetNextSibling()) {
    if (pChild->GetType() == FX_XMLNODE_Text ||
        pChild->GetType() == FX_XMLNODE_CharData) {
      buffer << static_cast<CFX_XMLText*>(pChild)->GetText();
    }
  }
  return buffer.MakeString();
}

void CFX_XMLElement::SetTextData(const WideString& wsText) {
  if (wsText.GetLength() < 1)
    return;
  AppendChild(new CFX_XMLText(wsText));
}

void CFX_XMLElement::Save(
    const RetainPtr<CFX_SeekableStreamProxy>& pXMLStream) {
  pXMLStream->WriteString(L"<");
  pXMLStream->WriteString(name_.AsStringView());

  for (auto it : attrs_) {
    pXMLStream->WriteString(
        AttributeToString(it.first, it.second).AsStringView());
  }

  if (!GetFirstChild()) {
    pXMLStream->WriteString(L" />");
    return;
  }

  pXMLStream->WriteString(L">");

  for (CFX_XMLNode* pChild = GetFirstChild(); pChild;
       pChild = pChild->GetNextSibling()) {
    pChild->Save(pXMLStream);
  }
  pXMLStream->WriteString(L"</");
  pXMLStream->WriteString(name_.AsStringView());
  pXMLStream->WriteString(L"\n>");
}

CFX_XMLElement* CFX_XMLElement::GetFirstChildNamed(
    const WideStringView& name) const {
  return GetNthChildNamed(name, 0);
}

CFX_XMLElement* CFX_XMLElement::GetNthChildNamed(const WideStringView& name,
                                                 size_t idx) const {
  for (auto* child = GetFirstChild(); child; child = child->GetNextSibling()) {
    if (child->GetType() != FX_XMLNODE_Element)
      continue;

    CFX_XMLElement* elem = static_cast<CFX_XMLElement*>(child);
    if (elem->name_ != name)
      continue;
    if (idx == 0)
      return elem;

    --idx;
  }
  return nullptr;
}

bool CFX_XMLElement::HasAttribute(const WideString& name) const {
  return attrs_.find(name) != attrs_.end();
}

WideString CFX_XMLElement::GetAttribute(const WideString& name) const {
  auto it = attrs_.find(name);
  return it != attrs_.end() ? it->second : L"";
}

void CFX_XMLElement::SetAttribute(const WideString& name,
                                  const WideString& value) {
  attrs_[name] = value;
}

void CFX_XMLElement::RemoveAttribute(const WideString& name) {
  attrs_.erase(name);
}

WideString CFX_XMLElement::AttributeToString(const WideString& name,
                                             const WideString& value) {
  WideString ret = L" ";
  ret += name;
  ret += L"=\"";
  ret += EncodeEntities(value);
  ret += L"\"";
  return ret;
}
