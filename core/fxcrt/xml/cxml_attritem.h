// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CXML_ATTRITEM_H_
#define CORE_FXCRT_XML_CXML_ATTRITEM_H_

#include "core/fxcrt/fx_string.h"

class CXML_AttrItem {
 public:
  bool Matches(const CFX_ByteString& space, const CFX_ByteString& name) const;

  CFX_ByteString m_QSpaceName;
  CFX_ByteString m_AttrName;
  CFX_WideString m_Value;
};

#endif  // CORE_FXCRT_XML_CXML_ATTRITEM_H_
