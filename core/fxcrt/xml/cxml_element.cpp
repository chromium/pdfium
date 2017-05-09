// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cxml_element.h"

#include "core/fxcrt/xml/cxml_content.h"
#include "core/fxcrt/xml/cxml_parser.h"

// static
std::unique_ptr<CXML_Element> CXML_Element::Parse(const void* pBuffer,
                                                  size_t size) {
  CXML_Parser parser;
  if (!parser.Init(static_cast<const uint8_t*>(pBuffer), size))
    return nullptr;
  return parser.ParseElement(nullptr, false);
}

CXML_Element::CXML_Element(const CXML_Element* pParent,
                           const CFX_ByteStringC& qSpace,
                           const CFX_ByteStringC& tagname)
    : m_pParent(pParent), m_QSpaceName(qSpace), m_TagName(tagname) {}

CXML_Element::~CXML_Element() {}

CXML_Element* CXML_Element::AsElement() {
  return this;
}

const CXML_Element* CXML_Element::AsElement() const {
  return this;
}

CFX_ByteString CXML_Element::GetTagName(bool bQualified) const {
  if (!bQualified || m_QSpaceName.IsEmpty()) {
    return m_TagName;
  }
  CFX_ByteString bsTag = m_QSpaceName;
  bsTag += ":";
  bsTag += m_TagName;
  return bsTag;
}

CFX_ByteString CXML_Element::GetNamespace(bool bQualified) const {
  return bQualified ? m_QSpaceName : GetNamespaceURI(m_QSpaceName);
}

CFX_ByteString CXML_Element::GetNamespaceURI(
    const CFX_ByteString& qName) const {
  const CFX_WideString* pwsSpace;
  const CXML_Element* pElement = this;
  do {
    if (qName.IsEmpty())
      pwsSpace = pElement->m_AttrMap.Lookup("", "xmlns");
    else
      pwsSpace = pElement->m_AttrMap.Lookup("xmlns", qName);
    if (pwsSpace)
      break;

    pElement = pElement->GetParent();
  } while (pElement);
  return pwsSpace ? pwsSpace->UTF8Encode() : CFX_ByteString();
}

void CXML_Element::GetAttrByIndex(int index,
                                  CFX_ByteString* space,
                                  CFX_ByteString* name,
                                  CFX_WideString* value) const {
  if (index < 0 || index >= m_AttrMap.GetSize())
    return;

  CXML_AttrItem& item = m_AttrMap.GetAt(index);
  *space = item.m_QSpaceName;
  *name = item.m_AttrName;
  *value = item.m_Value;
}

bool CXML_Element::HasAttr(const CFX_ByteStringC& name) const {
  CFX_ByteStringC bsSpace;
  CFX_ByteStringC bsName;
  FX_XML_SplitQualifiedName(name, bsSpace, bsName);
  return !!m_AttrMap.Lookup(CFX_ByteString(bsSpace), CFX_ByteString(bsName));
}

bool CXML_Element::GetAttrValue(const CFX_ByteStringC& name,
                                CFX_WideString& attribute) const {
  CFX_ByteStringC bsSpace;
  CFX_ByteStringC bsName;
  FX_XML_SplitQualifiedName(name, bsSpace, bsName);
  return GetAttrValue(bsSpace, bsName, attribute);
}

bool CXML_Element::GetAttrValue(const CFX_ByteStringC& space,
                                const CFX_ByteStringC& name,
                                CFX_WideString& attribute) const {
  const CFX_WideString* pValue =
      m_AttrMap.Lookup(CFX_ByteString(space), CFX_ByteString(name));
  if (!pValue)
    return false;

  attribute = *pValue;
  return true;
}

bool CXML_Element::GetAttrInteger(const CFX_ByteStringC& name,
                                  int& attribute) const {
  CFX_ByteStringC bsSpace;
  CFX_ByteStringC bsName;
  FX_XML_SplitQualifiedName(name, bsSpace, bsName);
  const CFX_WideString* pwsValue =
      m_AttrMap.Lookup(CFX_ByteString(bsSpace), CFX_ByteString(bsName));
  if (!pwsValue)
    return false;

  attribute = pwsValue->GetInteger();
  return true;
}

bool CXML_Element::GetAttrInteger(const CFX_ByteStringC& space,
                                  const CFX_ByteStringC& name,
                                  int& attribute) const {
  const CFX_WideString* pwsValue =
      m_AttrMap.Lookup(CFX_ByteString(space), CFX_ByteString(name));
  if (!pwsValue)
    return false;

  attribute = pwsValue->GetInteger();
  return true;
}

bool CXML_Element::GetAttrFloat(const CFX_ByteStringC& name,
                                float& attribute) const {
  CFX_ByteStringC bsSpace;
  CFX_ByteStringC bsName;
  FX_XML_SplitQualifiedName(name, bsSpace, bsName);
  return GetAttrFloat(bsSpace, bsName, attribute);
}

bool CXML_Element::GetAttrFloat(const CFX_ByteStringC& space,
                                const CFX_ByteStringC& name,
                                float& attribute) const {
  const CFX_WideString* pValue =
      m_AttrMap.Lookup(CFX_ByteString(space), CFX_ByteString(name));
  if (!pValue)
    return false;

  attribute = pValue->GetFloat();
  return true;
}

uint32_t CXML_Element::CountElements(const CFX_ByteStringC& space,
                                     const CFX_ByteStringC& tag) const {
  int count = 0;
  for (const auto& pChild : m_Children) {
    const CXML_Element* pKid = pChild->AsElement();
    if (pKid && pKid->m_TagName == tag &&
        (space.IsEmpty() || pKid->m_QSpaceName == space)) {
      count++;
    }
  }
  return count;
}

CXML_Object* CXML_Element::GetChild(uint32_t index) const {
  return index < m_Children.size() ? m_Children[index].get() : nullptr;
}

CXML_Element* CXML_Element::GetElement(const CFX_ByteStringC& space,
                                       const CFX_ByteStringC& tag,
                                       int nth) const {
  if (nth < 0)
    return nullptr;

  for (const auto& pChild : m_Children) {
    CXML_Element* pKid = pChild->AsElement();
    if (pKid && pKid->m_TagName == tag &&
        (space.IsEmpty() || pKid->m_QSpaceName == space)) {
      if (nth-- == 0)
        return pKid;
    }
  }
  return nullptr;
}

uint32_t CXML_Element::FindElement(CXML_Element* pElement) const {
  int index = 0;
  for (const auto& pChild : m_Children) {
    CXML_Element* pKid = pChild->AsElement();
    if (pKid && pKid == pElement)
      return index;

    ++index;
  }
  return 0xFFFFFFFF;
}

void CXML_Element::SetTag(const CFX_ByteStringC& qTagName) {
  ASSERT(!qTagName.IsEmpty());
  CFX_ByteStringC bsSpace;
  CFX_ByteStringC bsName;
  FX_XML_SplitQualifiedName(qTagName, bsSpace, bsName);
  m_QSpaceName = bsSpace;
  m_TagName = bsName;
}
