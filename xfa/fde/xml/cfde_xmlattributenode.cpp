// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/xml/cfde_xmlattributenode.h"

#include "core/fxcrt/fx_ext.h"

CFDE_XMLAttributeNode::CFDE_XMLAttributeNode(const CFX_WideString& name)
    : CFDE_XMLNode(), name_(name) {
  ASSERT(name_.GetLength() > 0);
}

CFDE_XMLAttributeNode::~CFDE_XMLAttributeNode() {}

bool CFDE_XMLAttributeNode::HasAttribute(const CFX_WideString& name) const {
  return attrs_.find(name) != attrs_.end();
}

CFX_WideString CFDE_XMLAttributeNode::GetString(
    const CFX_WideString& name) const {
  auto it = attrs_.find(name);
  return it != attrs_.end() ? it->second : CFX_WideString();
}

void CFDE_XMLAttributeNode::SetString(const CFX_WideString& name,
                                      const CFX_WideString& value) {
  attrs_[name] = value;
}

void CFDE_XMLAttributeNode::RemoveAttribute(const CFX_WideString& name) {
  attrs_.erase(name);
}
