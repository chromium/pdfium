// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CXML_ATTRITEM_H_
#define CORE_FXCRT_XML_CXML_ATTRITEM_H_

#include "core/fxcrt/fx_string.h"

class CXML_AttrItem {
 public:
  bool Matches(const ByteString& space, const ByteString& name) const;

  ByteString m_QSpaceName;
  ByteString m_AttrName;
  WideString m_Value;
};

#endif  // CORE_FXCRT_XML_CXML_ATTRITEM_H_
