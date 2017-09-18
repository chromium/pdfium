// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmltext.h"

#include "third_party/base/ptr_util.h"

CFX_XMLText::CFX_XMLText(const WideString& wsText)
    : CFX_XMLNode(), m_wsText(wsText) {}

CFX_XMLText::~CFX_XMLText() {}

FX_XMLNODETYPE CFX_XMLText::GetType() const {
  return FX_XMLNODE_Text;
}

std::unique_ptr<CFX_XMLNode> CFX_XMLText::Clone() {
  return pdfium::MakeUnique<CFX_XMLText>(m_wsText);
}
