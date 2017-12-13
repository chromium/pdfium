// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CXML_ELEMENT_H_
#define CORE_FXCRT_XML_CXML_ELEMENT_H_

#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/xml/cxml_attrmap.h"
#include "core/fxcrt/xml/cxml_object.h"

class CXML_Element : public CXML_Object {
 public:
  static std::unique_ptr<CXML_Element> Parse(const void* pBuffer, size_t size);

  CXML_Element(const CXML_Element* pParent,
               const ByteStringView& qSpace,
               const ByteStringView& tagname);
  ~CXML_Element() override;

  // CXML_Object:
  CXML_Element* AsElement() override;
  const CXML_Element* AsElement() const override;

  ByteString GetTagName(bool bQualified = false) const;
  ByteString GetNamespaceURI(const ByteString& qName) const;
  const CXML_Element* GetParent() const { return m_pParent.Get(); }
  uint32_t CountAttrs() const { return m_AttrMap.GetSize(); }
  void GetAttrByIndex(int index,
                      ByteString* space,
                      ByteString* name,
                      WideString* value) const;
  WideString GetAttrValue(const ByteStringView& name) const;

  int GetAttrInteger(const ByteStringView& name) const;

  void AppendChild(std::unique_ptr<CXML_Object> child) {
    m_Children.push_back(std::move(child));
  }

  uint32_t CountChildren() const { return m_Children.size(); }
  uint32_t CountElements(const ByteStringView& space,
                         const ByteStringView& tag) const;
  CXML_Object* GetChild(uint32_t index) const;
  CXML_Element* GetElement(const ByteStringView& space,
                           const ByteStringView& tag,
                           int nth) const;

  void SetAttribute(const ByteString& space,
                    const ByteString& name,
                    const WideString& value);

 private:
  UnownedPtr<const CXML_Element> const m_pParent;
  ByteString m_QSpaceName;
  ByteString m_TagName;
  CXML_AttrMap m_AttrMap;
  std::vector<std::unique_ptr<CXML_Object>> m_Children;
};

#endif  // CORE_FXCRT_XML_CXML_ELEMENT_H_
