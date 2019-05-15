// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmlnode.h"

CFX_XMLNode::CFX_XMLNode() = default;

CFX_XMLNode::~CFX_XMLNode() = default;

void CFX_XMLNode::InsertChildNode(CFX_XMLNode* pNode, int32_t index) {
  InsertBefore(pNode, GetNthChild(index));
}

CFX_XMLNode* CFX_XMLNode::GetRoot() {
  CFX_XMLNode* pParent = this;
  while (pParent->GetParent())
    pParent = pParent->GetParent();

  return pParent;
}

WideString CFX_XMLNode::EncodeEntities(const WideString& value) {
  WideString ret = value;
  ret.Replace(L"&", L"&amp;");
  ret.Replace(L"<", L"&lt;");
  ret.Replace(L">", L"&gt;");
  ret.Replace(L"\'", L"&apos;");
  ret.Replace(L"\"", L"&quot;");
  return ret;
}
