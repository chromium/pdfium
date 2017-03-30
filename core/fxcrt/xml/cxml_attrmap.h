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

  const CFX_WideString* Lookup(const CFX_ByteString& space,
                               const CFX_ByteString& name) const;
  int GetSize() const;
  CXML_AttrItem& GetAt(int index) const;

  void SetAt(const CFX_ByteString& space,
             const CFX_ByteString& name,
             const CFX_WideString& value);

  std::unique_ptr<std::vector<CXML_AttrItem>> m_pMap;
};

#endif  // CORE_FXCRT_XML_CXML_ATTRMAP_H_
