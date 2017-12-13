// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cxml_element.h"

#include "core/fxcrt/xml/cxml_content.h"
#include "core/fxcrt/xml/cxml_parser.h"

namespace {

void SplitQualifiedName(const ByteStringView& bsFullName,
                        ByteStringView* bsSpace,
                        ByteStringView* bsName) {
  if (bsFullName.IsEmpty())
    return;

  auto iStart = bsFullName.Find(':');
  if (iStart.has_value()) {
    *bsSpace = bsFullName.Left(iStart.value());
    *bsName = bsFullName.Right(bsFullName.GetLength() - (iStart.value() + 1));
  } else {
    *bsName = bsFullName;
  }
}

}  // namespace

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
  const CXML_Element* pElement = this;
  do {
    const WideString* pwsSpace;
    if (qName.IsEmpty())
      pwsSpace = pElement->Lookup("", "xmlns");
    else
      pwsSpace = pElement->Lookup("xmlns", qName);
    if (pwsSpace)
      return pwsSpace->UTF8Encode();

    pElement = pElement->GetParent();
  } while (pElement);
  return ByteString();
}

void CXML_Element::GetAttrByIndex(size_t index,
                                  ByteString* space,
                                  ByteString* name,
                                  WideString* value) const {
  if (index >= m_AttrMap.size())
    return;

  const CXML_AttrItem& item = m_AttrMap[index];
  *space = item.m_QSpaceName;
  *name = item.m_AttrName;
  *value = item.m_Value;
}

WideString CXML_Element::GetAttrValue(const ByteStringView& name) const {
  ByteStringView bsSpace;
  ByteStringView bsName;
  SplitQualifiedName(name, &bsSpace, &bsName);

  WideString attr;
  const WideString* pValue = Lookup(ByteString(bsSpace), ByteString(bsName));
  if (pValue)
    attr = *pValue;
  return attr;
}

int CXML_Element::GetAttrInteger(const ByteStringView& name) const {
  ByteStringView bsSpace;
  ByteStringView bsName;
  SplitQualifiedName(name, &bsSpace, &bsName);

  const WideString* pwsValue = Lookup(ByteString(bsSpace), ByteString(bsName));
  return pwsValue ? pwsValue->GetInteger() : 0;
}

size_t CXML_Element::CountElements(const ByteStringView& space,
                                   const ByteStringView& tag) const {
  size_t count = 0;
  for (const auto& pChild : m_Children) {
    const CXML_Element* pKid = pChild->AsElement();
    if (MatchesElement(pKid, space, tag))
      count++;
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
    if (MatchesElement(pKid, space, tag)) {
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
  for (CXML_AttrItem& item : m_AttrMap) {
    if (item.Matches(space, name)) {
      item.m_Value = value;
      return;
    }
  }
  m_AttrMap.push_back({space, name, WideString(value)});
}

// static
bool CXML_Element::MatchesElement(const CXML_Element* pKid,
                                  const ByteStringView& space,
                                  const ByteStringView& tag) {
  return pKid && pKid->m_TagName == tag &&
         (space.IsEmpty() || pKid->m_QSpaceName == space);
}

const WideString* CXML_Element::Lookup(const ByteString& space,
                                       const ByteString& name) const {
  for (const CXML_AttrItem& item : m_AttrMap) {
    if (item.Matches(space, name))
      return &item.m_Value;
  }
  return nullptr;
}
