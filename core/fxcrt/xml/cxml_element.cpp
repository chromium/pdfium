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
                           const ByteStringView& qSpace,
                           const ByteStringView& tagname)
    : m_pParent(pParent), m_QSpaceName(qSpace), m_TagName(tagname) {}

CXML_Element::~CXML_Element() {}

CXML_Element* CXML_Element::AsElement() {
  return this;
}

const CXML_Element* CXML_Element::AsElement() const {
  return this;
}

ByteString CXML_Element::GetTagName() const {
  return m_TagName;
}

ByteString CXML_Element::GetNamespaceURI(const ByteString& qName) const {
  const WideString* pwsSpace;
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
  return pwsSpace ? pwsSpace->UTF8Encode() : ByteString();
}

void CXML_Element::GetAttrByIndex(size_t index,
                                  ByteString* space,
                                  ByteString* name,
                                  WideString* value) const {
  if (index >= static_cast<size_t>(m_AttrMap.GetSize()))
    return;

  CXML_AttrItem& item = m_AttrMap.GetAt(index);
  *space = item.m_QSpaceName;
  *name = item.m_AttrName;
  *value = item.m_Value;
}

WideString CXML_Element::GetAttrValue(const ByteStringView& name) const {
  ByteStringView bsSpace;
  ByteStringView bsName;
  FX_XML_SplitQualifiedName(name, bsSpace, bsName);

  WideString attr;
  const WideString* pValue =
      m_AttrMap.Lookup(ByteString(bsSpace), ByteString(bsName));
  if (pValue)
    attr = *pValue;
  return attr;
}

int CXML_Element::GetAttrInteger(const ByteStringView& name) const {
  ByteStringView bsSpace;
  ByteStringView bsName;
  FX_XML_SplitQualifiedName(name, bsSpace, bsName);

  const WideString* pwsValue =
      m_AttrMap.Lookup(ByteString(bsSpace), ByteString(bsName));
  return pwsValue ? pwsValue->GetInteger() : 0;
}

size_t CXML_Element::CountElements(const ByteStringView& space,
                                   const ByteStringView& tag) const {
  size_t count = 0;
  for (const auto& pChild : m_Children) {
    const CXML_Element* pKid = pChild->AsElement();
    if (pKid && pKid->m_TagName == tag &&
        (space.IsEmpty() || pKid->m_QSpaceName == space)) {
      count++;
    }
  }
  return count;
}

CXML_Object* CXML_Element::GetChild(size_t index) const {
  return index < m_Children.size() ? m_Children[index].get() : nullptr;
}

CXML_Element* CXML_Element::GetElement(const ByteStringView& space,
                                       const ByteStringView& tag,
                                       size_t nth) const {
  for (const auto& pChild : m_Children) {
    CXML_Element* pKid = pChild->AsElement();
    if (pKid && pKid->m_TagName == tag &&
        (space.IsEmpty() || pKid->m_QSpaceName == space)) {
      if (nth == 0)
        return pKid;
      --nth;
    }
  }
  return nullptr;
}

void CXML_Element::SetAttribute(const ByteString& space,
                                const ByteString& name,
                                const WideString& value) {
  m_AttrMap.SetAt(space, name, value);
}
