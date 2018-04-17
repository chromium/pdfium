// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmlnode.h"

#include <utility>
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
  CFX_XMLNode* child = last_child_.Get();
  // Clear last child early as it will have been deleted already.
  last_child_ = nullptr;
  while (child) {
    child = child->prev_sibling_.Get();
    if (child)
      child->next_sibling_ = nullptr;
  }
  first_child_ = nullptr;
}

void CFX_XMLNode::AppendChild(std::unique_ptr<CFX_XMLNode> pNode) {
  InsertChildNode(std::move(pNode), -1);
}

void CFX_XMLNode::InsertChildNode(std::unique_ptr<CFX_XMLNode> pNode,
                                  int32_t index) {
  ASSERT(!pNode->parent_);

  pNode->parent_ = this;
  // No existing children, add node as first child.
  if (!first_child_) {
    ASSERT(!last_child_);

    first_child_ = std::move(pNode);
    last_child_ = first_child_.get();
    first_child_->prev_sibling_ = nullptr;
    first_child_->next_sibling_ = nullptr;
    return;
  }

  if (index == 0) {
    first_child_->prev_sibling_ = pNode.get();
    pNode->next_sibling_ = std::move(first_child_);
    pNode->prev_sibling_ = nullptr;
    first_child_ = std::move(pNode);
    return;
  }

  int32_t iCount = 0;
  CFX_XMLNode* pFind = first_child_.get();
  // Note, negative indexes, and indexes after the end of the list will result
  // in appending to the list.
  while (++iCount != index && pFind->next_sibling_)
    pFind = pFind->next_sibling_.get();

  pNode->prev_sibling_ = pFind;
  if (pFind->next_sibling_)
    pFind->next_sibling_->prev_sibling_ = pNode.get();
  pNode->next_sibling_ = std::move(pFind->next_sibling_);

  pFind->next_sibling_ = std::move(pNode);
  if (pFind == last_child_.Get())
    last_child_ = pFind->next_sibling_.get();
}

void CFX_XMLNode::RemoveChildNode(CFX_XMLNode* pNode) {
  ASSERT(first_child_);
  ASSERT(pNode);

  if (first_child_.get() == pNode) {
    first_child_.release();
    first_child_ = std::move(pNode->next_sibling_);
  } else {
    CFX_XMLNode* prev = pNode->prev_sibling_.Get();
    prev->next_sibling_.release();  // Release pNode
    prev->next_sibling_ = std::move(pNode->next_sibling_);
  }

  if (last_child_.Get() == pNode)
    last_child_ = pNode->prev_sibling_.Get();

  if (pNode->next_sibling_)
    pNode->next_sibling_->prev_sibling_ = pNode->prev_sibling_;

  pNode->parent_ = nullptr;
  pNode->prev_sibling_ = nullptr;
}

CFX_XMLNode* CFX_XMLNode::GetRoot() {
  CFX_XMLNode* pParent = this;
  while (pParent->parent_)
    pParent = pParent->parent_.Get();

  return pParent;
}

std::unique_ptr<CFX_XMLNode> CFX_XMLNode::Clone() {
  return nullptr;
}

void CFX_XMLNode::Save(const RetainPtr<IFX_SeekableStream>& pXMLStream) {}

WideString CFX_XMLNode::EncodeEntities(const WideString& value) {
  WideString ret = value;
  ret.Replace(L"&", L"&amp;");
  ret.Replace(L"<", L"&lt;");
  ret.Replace(L">", L"&gt;");
  ret.Replace(L"\'", L"&apos;");
  ret.Replace(L"\"", L"&quot;");
  return ret;
}
