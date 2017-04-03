// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/xml/cfde_xmltext.h"

CFDE_XMLText::CFDE_XMLText(const CFX_WideString& wsText)
    : CFDE_XMLNode(), m_wsText(wsText) {}

CFDE_XMLText::~CFDE_XMLText() {}

FDE_XMLNODETYPE CFDE_XMLText::GetType() const {
  return FDE_XMLNODE_Text;
}

CFDE_XMLNode* CFDE_XMLText::Clone(bool bRecursive) {
  CFDE_XMLText* pClone = new CFDE_XMLText(m_wsText);
  return pClone;
}
