// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_arraynodelist.h"

#include <utility>
#include <vector>

#include "fxjs/gc/container_trace.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_ArrayNodeList::CXFA_ArrayNodeList(CXFA_Document* pDocument)
    : CXFA_TreeList(pDocument) {}

CXFA_ArrayNodeList::~CXFA_ArrayNodeList() = default;

void CXFA_ArrayNodeList::Trace(cppgc::Visitor* visitor) const {
  CXFA_TreeList::Trace(visitor);
  ContainerTrace(visitor, array_);
}

void CXFA_ArrayNodeList::SetArrayNodeList(
    const std::vector<CXFA_Node*>& srcArray) {
  if (!srcArray.empty()) {
    array_ =
        std::vector<cppgc::Member<CXFA_Node>>(srcArray.begin(), srcArray.end());
  }
}

size_t CXFA_ArrayNodeList::GetLength() {
  return array_.size();
}

bool CXFA_ArrayNodeList::Append(CXFA_Node* pNode) {
  array_.push_back(pNode);
  return true;
}

bool CXFA_ArrayNodeList::Insert(CXFA_Node* pNewNode, CXFA_Node* pBeforeNode) {
  if (!pBeforeNode) {
    array_.push_back(pNewNode);
    return true;
  }

  auto it = std::ranges::find(array_, pBeforeNode);
  if (it == array_.end()) {
    return false;
  }

  array_.insert(it, pNewNode);
  return true;
}

void CXFA_ArrayNodeList::Remove(CXFA_Node* pNode) {
  auto it = std::ranges::find(array_, pNode);
  if (it != array_.end()) {
    array_.erase(it);
  }
}

CXFA_Node* CXFA_ArrayNodeList::Item(size_t index) {
  return index < array_.size() ? array_[index] : nullptr;
}
