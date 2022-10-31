// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_textnode.h"

#include "fxjs/xfa/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CJX_TextNode::CJX_TextNode(CXFA_Node* node) : CJX_Node(node) {}

CJX_TextNode::~CJX_TextNode() = default;

bool CJX_TextNode::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

void CJX_TextNode::defaultValue(v8::Isolate* pIsolate,
                                v8::Local<v8::Value>* pValue,
                                bool bSetting,
                                XFA_Attribute attr) {
  ScriptSomDefaultValue(pIsolate, pValue, bSetting, attr);
}

void CJX_TextNode::value(v8::Isolate* pIsolate,
                         v8::Local<v8::Value>* pValue,
                         bool bSetting,
                         XFA_Attribute attr) {
  ScriptSomDefaultValue(pIsolate, pValue, bSetting, attr);
}
