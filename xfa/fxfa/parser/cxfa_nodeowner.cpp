// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_nodeowner.h"

#include <utility>

#include "xfa/fxfa/parser/cxfa_list.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_NodeOwner::CXFA_NodeOwner() = default;

CXFA_NodeOwner::~CXFA_NodeOwner() = default;

CXFA_Node* CXFA_NodeOwner::AddOwnedNode(std::unique_ptr<CXFA_Node> node) {
  if (!node)
    return nullptr;

  CXFA_Node* ret = node.get();
  nodes_.push_back(std::move(node));
  return ret;
}

CXFA_List* CXFA_NodeOwner::AddOwnedList(std::unique_ptr<CXFA_List> list) {
  ASSERT(list);
  CXFA_List* ret = list.get();
  lists_.push_back(std::move(list));
  return ret;
}
