// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CXML_ATTRMAP_H_
#define CORE_FXCRT_XML_CXML_ATTRMAP_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/xml/cxml_attritem.h"

class CXML_AttrMap {
 public:
  CXML_AttrMap();
  ~CXML_AttrMap();

  const WideString* Lookup(const ByteString& space,
                           const ByteString& name) const;
  int GetSize() const;
  CXML_AttrItem& GetAt(int index) const;

  void SetAt(const ByteString& space,
             const ByteString& name,
             const WideString& value);

  std::unique_ptr<std::vector<CXML_AttrItem>> m_pMap;
};

#endif  // CORE_FXCRT_XML_CXML_ATTRMAP_H_
