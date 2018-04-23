// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmlnode.h"

#include <algorithm>
#include <utility>

#include "core/fxcrt/xml/cfx_xmlchardata.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlinstruction.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "third_party/base/stl_util.h"

CFX_XMLNode::CFX_XMLNode() = default;

CFX_XMLNode::~CFX_XMLNode() {
  DeleteChildren();
}

void CFX_XMLNode::DeleteChildren() {
  children_.clear();
}

void CFX_XMLNode::AppendChild(std::unique_ptr<CFX_XMLNode> pNode) {
  InsertChildNode(std::move(pNode), -1);
}

void CFX_XMLNode::InsertChildNode(std::unique_ptr<CFX_XMLNode> pNode,
                                  int32_t index) {
  ASSERT(!pNode->parent_);

  pNode->parent_ = this;
  if (static_cast<size_t>(index) >= children_.size()) {
    if (!children_.empty())
      children_.back()->next_sibling_ = pNode.get();
    children_.push_back(std::move(pNode));
    return;
  }
  index = std::max(index, 0);

  pNode->next_sibling_ = children_[index].get();
  children_.insert(children_.begin() + index, std::move(pNode));
  if (index > 0)
    children_[index - 1]->next_sibling_ = children_[index].get();
}

void CFX_XMLNode::RemoveChildNode(CFX_XMLNode* pNode) {
  ASSERT(pNode);

  auto it = std::find(children_.begin(), children_.end(),
                      pdfium::FakeUniquePtr<CFX_XMLNode>(pNode));
  if (it != children_.end()) {
    if (it != children_.begin()) {
      CFX_XMLNode* prev = (*(it - 1)).get();
      prev->next_sibling_ = (*it)->next_sibling_;
    }

    children_.erase(it);
  }
}

void CFX_XMLNode::MoveChildrenTo(CFX_XMLNode* root) {
  ASSERT(root->children_.empty());
  std::swap(root->children_, children_);
}

CFX_XMLNode* CFX_XMLNode::GetRoot() {
  CFX_XMLNode* pParent = this;
  while (pParent->parent_)
    pParent = pParent->parent_.Get();

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
