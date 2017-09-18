// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmlattributenode.h"

#include "core/fxcrt/fx_extension.h"

CFX_XMLAttributeNode::CFX_XMLAttributeNode(const WideString& name)
    : CFX_XMLNode(), name_(name) {
  ASSERT(name_.GetLength() > 0);
}

CFX_XMLAttributeNode::~CFX_XMLAttributeNode() {}

bool CFX_XMLAttributeNode::HasAttribute(const WideString& name) const {
  return attrs_.find(name) != attrs_.end();
}

WideString CFX_XMLAttributeNode::GetString(const WideString& name) const {
  auto it = attrs_.find(name);
  return it != attrs_.end() ? it->second : WideString();
}

void CFX_XMLAttributeNode::SetString(const WideString& name,
                                     const WideString& value) {
  attrs_[name] = value;
}

void CFX_XMLAttributeNode::RemoveAttribute(const WideString& name) {
  attrs_.erase(name);
}
