// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xml_int.h"

#include "core/include/fxcrt/fx_xml.h"

void FX_XML_SplitQualifiedName(const CFX_ByteStringC& bsFullName,
                               CFX_ByteStringC& bsSpace,
                               CFX_ByteStringC& bsName) {
  if (bsFullName.IsEmpty()) {
    return;
  }
  int32_t iStart = 0;
  for (; iStart < bsFullName.GetLength(); iStart++) {
    if (bsFullName.GetAt(iStart) == ':') {
      break;
    }
  }
  if (iStart >= bsFullName.GetLength()) {
    bsName = bsFullName;
  } else {
    bsSpace = CFX_ByteStringC(bsFullName.GetCStr(), iStart);
    iStart++;
    bsName = CFX_ByteStringC(bsFullName.GetCStr() + iStart,
                             bsFullName.GetLength() - iStart);
  }
}
void CXML_Element::SetTag(const CFX_ByteStringC& qSpace,
                          const CFX_ByteStringC& tagname) {
  m_QSpaceName = qSpace;
  m_TagName = tagname;
}
void CXML_Element::SetTag(const CFX_ByteStringC& qTagName) {
  ASSERT(!qTagName.IsEmpty());
  CFX_ByteStringC bsSpace, bsName;
  FX_XML_SplitQualifiedName(qTagName, bsSpace, bsName);
  m_QSpaceName = bsSpace;
  m_TagName = bsName;
}
