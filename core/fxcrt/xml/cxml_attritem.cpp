// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cxml_attritem.h"

bool CXML_AttrItem::Matches(const ByteString& space,
                            const ByteString& name) const {
  return (space.IsEmpty() || m_QSpaceName == space) && m_AttrName == name;
}
