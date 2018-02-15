// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_nodeowner.h"

#include <utility>

#include "third_party/base/stl_util.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_NodeOwner::CXFA_NodeOwner() = default;

CXFA_NodeOwner::~CXFA_NodeOwner() = default;

void CXFA_NodeOwner::ReleaseXMLNodesIfNeeded() {
  // Because we don't know what order we'll free the nodes we may end up
  // destroying the XML tree before nodes have been cleaned up that point into
  // it. This will cause the ProbeForLowSeverityLifetimeIssue to fire.
  //
  // This doesn't happen in the destructor because of the ownership semantics
  // between the CXFA_Document and CXFA_SimpleParser. It has to happen before
  // the simple parser is destroyed, but the document has to live longer then
  // the simple parser.
  for (auto& it : nodes_)
    it->ReleaseXMLNodeIfUnowned();
}

CXFA_Node* CXFA_NodeOwner::AddOwnedNode(std::unique_ptr<CXFA_Node> node) {
  if (!node)
    return nullptr;

  CXFA_Node* ret = node.get();
  nodes_.insert(std::move(node));
  return ret;
}

void CXFA_NodeOwner::FreeOwnedNode(CXFA_Node* node) {
  if (!node)
    return;

  pdfium::FakeUniquePtr<CXFA_Node> search(node);
  auto it = nodes_.find(search);
  assert(it != nodes_.end());
  nodes_.erase(it);
}
