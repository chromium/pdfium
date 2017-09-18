// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cxml_attrmap.h"

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

CXML_AttrMap::CXML_AttrMap() {}

CXML_AttrMap::~CXML_AttrMap() {}

const WideString* CXML_AttrMap::Lookup(const ByteString& space,
                                       const ByteString& name) const {
  if (!m_pMap)
    return nullptr;

  for (const auto& item : *m_pMap) {
    if (item.Matches(space, name))
      return &item.m_Value;
  }
  return nullptr;
}

void CXML_AttrMap::SetAt(const ByteString& space,
                         const ByteString& name,
                         const WideString& value) {
  if (!m_pMap)
    m_pMap = pdfium::MakeUnique<std::vector<CXML_AttrItem>>();

  for (CXML_AttrItem& item : *m_pMap) {
    if (item.Matches(space, name)) {
      item.m_Value = value;
      return;
    }
  }

  m_pMap->push_back({space, name, WideString(value)});
}

int CXML_AttrMap::GetSize() const {
  return m_pMap ? pdfium::CollectionSize<int>(*m_pMap) : 0;
}

CXML_AttrItem& CXML_AttrMap::GetAt(int index) const {
  return (*m_pMap)[index];
}
