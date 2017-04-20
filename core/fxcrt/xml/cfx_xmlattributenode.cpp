// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmlattributenode.h"

#include "core/fxcrt/fx_extension.h"

CFX_XMLAttributeNode::CFX_XMLAttributeNode(const CFX_WideString& name)
    : CFX_XMLNode(), name_(name) {
  ASSERT(name_.GetLength() > 0);
}

CFX_XMLAttributeNode::~CFX_XMLAttributeNode() {}

bool CFX_XMLAttributeNode::HasAttribute(const CFX_WideString& name) const {
  return attrs_.find(name) != attrs_.end();
}

CFX_WideString CFX_XMLAttributeNode::GetString(
    const CFX_WideString& name) const {
  auto it = attrs_.find(name);
  return it != attrs_.end() ? it->second : CFX_WideString();
}

void CFX_XMLAttributeNode::SetString(const CFX_WideString& name,
                                     const CFX_WideString& value) {
  attrs_[name] = value;
}

void CFX_XMLAttributeNode::RemoveAttribute(const CFX_WideString& name) {
  attrs_.erase(name);
}
