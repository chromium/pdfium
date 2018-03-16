// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmlnode.h"

#include <vector>

#include "core/fxcrt/xml/cfx_xmlchardata.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlinstruction.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "third_party/base/stl_util.h"

CFX_XMLNode::CFX_XMLNode() = default;

CFX_XMLNode::~CFX_XMLNode() {
  DeleteChildren();
}

FX_XMLNODETYPE CFX_XMLNode::GetType() const {
  return FX_XMLNODE_Unknown;
}

void CFX_XMLNode::DeleteChildren() {
  CFX_XMLNode* pChild = first_child_;
  first_child_ = nullptr;
  last_child_ = nullptr;

  while (pChild) {
    CFX_XMLNode* pNext = pChild->next_sibling_;
    delete pChild;
    pChild = pNext;
  }
}

void CFX_XMLNode::AppendChild(CFX_XMLNode* pNode) {
  InsertChildNode(pNode, -1);
}

void CFX_XMLNode::InsertChildNode(CFX_XMLNode* pNode, int32_t index) {
  ASSERT(!pNode->parent_);

  pNode->parent_ = this;
  if (!first_child_) {
    ASSERT(!last_child_);

    first_child_ = pNode;
    last_child_ = pNode;
    pNode->prev_sibling_ = nullptr;
    pNode->next_sibling_ = nullptr;
    return;
  }

  if (index == 0) {
    pNode->next_sibling_ = first_child_;
    pNode->prev_sibling_ = nullptr;
    first_child_->prev_sibling_ = pNode;
    first_child_ = pNode;
    return;
  }

  int32_t iCount = 0;
  CFX_XMLNode* pFind = first_child_;
  while (++iCount != index && pFind->next_sibling_)
    pFind = pFind->next_sibling_;

  pNode->prev_sibling_ = pFind;
  pNode->next_sibling_ = pFind->next_sibling_;
  if (pFind->next_sibling_)
    pFind->next_sibling_->prev_sibling_ = pNode;

  pFind->next_sibling_ = pNode;
  if (pFind == last_child_)
    last_child_ = pNode;
}

void CFX_XMLNode::RemoveChildNode(CFX_XMLNode* pNode) {
  ASSERT(first_child_ && pNode);

  if (first_child_ == pNode)
    first_child_ = pNode->next_sibling_;
  else
    pNode->prev_sibling_->next_sibling_ = pNode->next_sibling_;

  if (last_child_ == pNode)
    last_child_ = pNode->prev_sibling_;

  if (pNode->next_sibling_)
    pNode->next_sibling_->prev_sibling_ = pNode->prev_sibling_;

  pNode->parent_ = nullptr;
  pNode->next_sibling_ = nullptr;
  pNode->prev_sibling_ = nullptr;
}

CFX_XMLNode* CFX_XMLNode::GetRoot() {
  CFX_XMLNode* pParent = this;
  while (pParent->parent_)
    pParent = pParent->parent_;

  return pParent;
}

std::unique_ptr<CFX_XMLNode> CFX_XMLNode::Clone() {
  return nullptr;
}

void CFX_XMLNode::Save(const RetainPtr<CFX_SeekableStreamProxy>& pXMLStream) {}

WideString CFX_XMLNode::EncodeEntities(const WideString& value) {
  WideString ret = value;
  ret.Replace(L"&", L"&amp;");
  ret.Replace(L"<", L"&lt;");
  ret.Replace(L">", L"&gt;");
  ret.Replace(L"\'", L"&apos;");
  ret.Replace(L"\"", L"&quot;");
  return ret;
}

WideString CFX_XMLNode::AttributeToString(const WideString& name,
                                          const WideString& value) {
  WideString ret = L" ";
  ret += name;
  ret += L"=\"";
  ret += EncodeEntities(value);
  ret += L"\"";
  return ret;
}
