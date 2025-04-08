// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_attachnodelist.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_AttachNodeList::CXFA_AttachNodeList(CXFA_Document* pDocument,
                                         CXFA_Node* pAttachNode)
    : CXFA_TreeList(pDocument), attach_node_(pAttachNode) {}

CXFA_AttachNodeList::~CXFA_AttachNodeList() = default;

void CXFA_AttachNodeList::Trace(cppgc::Visitor* visitor) const {
  CXFA_TreeList::Trace(visitor);
  visitor->Trace(attach_node_);
}

size_t CXFA_AttachNodeList::GetLength() {
  return attach_node_->CountChildren(
      XFA_Element::Unknown,
      attach_node_->GetElementType() == XFA_Element::Subform);
}

bool CXFA_AttachNodeList::Append(CXFA_Node* pNode) {
  if (pNode->IsAncestorOf(attach_node_)) {
    return false;
  }

  CXFA_Node* pParent = pNode->GetParent();
  if (pParent) {
    pParent->RemoveChildAndNotify(pNode, true);
  }

  attach_node_->InsertChildAndNotify(pNode, nullptr);
  return true;
}

bool CXFA_AttachNodeList::Insert(CXFA_Node* pNewNode, CXFA_Node* pBeforeNode) {
  if (pNewNode->IsAncestorOf(attach_node_)) {
    return false;
  }

  if (pBeforeNode && pBeforeNode->GetParent() != attach_node_) {
    return false;
  }

  CXFA_Node* pParent = pNewNode->GetParent();
  if (pParent) {
    pParent->RemoveChildAndNotify(pNewNode, true);
  }

  attach_node_->InsertChildAndNotify(pNewNode, pBeforeNode);
  return true;
}

void CXFA_AttachNodeList::Remove(CXFA_Node* pNode) {
  attach_node_->RemoveChildAndNotify(pNode, true);
}

CXFA_Node* CXFA_AttachNodeList::Item(size_t index) {
  return attach_node_->GetChild<CXFA_Node>(
      index, XFA_Element::Unknown,
      attach_node_->GetElementType() == XFA_Element::Subform);
}
