// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_NODEITERATORTEMPLATE_H_
#define XFA_FXFA_PARSER_CXFA_NODEITERATORTEMPLATE_H_

#include <type_traits>
#include "core/fxcrt/unowned_ptr.h"
#include "v8/include/cppgc/macros.h"
#include "v8/include/cppgc/type-traits.h"

template <class NodeType,
          class TraverseStrategy,
          typename HolderType = std::conditional_t<
              cppgc::IsGarbageCollectedOrMixinTypeV<NodeType>,
              NodeType*,
              UnownedPtr<NodeType>>>
class CXFA_NodeIteratorTemplate {
  CPPGC_STACK_ALLOCATED();  // Allows Raw/Unowned |HolderType|.

 public:
  explicit CXFA_NodeIteratorTemplate(NodeType* pRoot)
      : root_(pRoot), current_(pRoot) {}

  NodeType* GetRoot() const { return static_cast<NodeType*>(root_); }
  NodeType* GetCurrent() const { return static_cast<NodeType*>(current_); }

  void Reset() { current_ = root_; }
  bool SetCurrent(NodeType* pNode) {
    if (!RootReachableFromNode(pNode)) {
      current_ = nullptr;
      return false;
    }
    current_ = pNode;
    return true;
  }

  NodeType* MoveToPrev() {
    if (!root_) {
      return nullptr;
    }
    if (!current_) {
      current_ = LastDescendant(static_cast<NodeType*>(root_));
      return static_cast<NodeType*>(current_);
    }
    NodeType* pSibling =
        PreviousSiblingWithinSubtree(static_cast<NodeType*>(current_));
    if (pSibling) {
      current_ = LastDescendant(pSibling);
      return static_cast<NodeType*>(current_);
    }
    NodeType* pParent = ParentWithinSubtree(static_cast<NodeType*>(current_));
    if (pParent) {
      current_ = pParent;
    }
    return pParent;
  }

  NodeType* MoveToNext() {
    if (!root_ || !current_) {
      return nullptr;
    }
    NodeType* pChild =
        TraverseStrategy::GetFirstChild(static_cast<NodeType*>(current_));
    if (pChild) {
      current_ = pChild;
      return pChild;
    }
    return SkipChildrenAndMoveToNext();
  }

  NodeType* SkipChildrenAndMoveToNext() {
    if (!root_) {
      return nullptr;
    }
    NodeType* pNode = static_cast<NodeType*>(current_);
    while (pNode) {
      NodeType* pSibling = NextSiblingWithinSubtree(pNode);
      if (pSibling) {
        current_ = pSibling;
        return pSibling;
      }
      pNode = ParentWithinSubtree(pNode);
    }
    current_ = nullptr;
    return nullptr;
  }

 private:
  bool RootReachableFromNode(NodeType* pNode) {
    return pNode && (pNode == root_ ||
                     RootReachableFromNode(TraverseStrategy::GetParent(pNode)));
  }

  NodeType* ParentWithinSubtree(NodeType* pNode) {
    return pNode && pNode != root_ ? TraverseStrategy::GetParent(pNode)
                                   : nullptr;
  }

  NodeType* NextSiblingWithinSubtree(NodeType* pNode) {
    return pNode != root_ ? TraverseStrategy::GetNextSibling(pNode) : nullptr;
  }

  NodeType* PreviousSiblingWithinSubtree(NodeType* pNode) {
    NodeType* pParent = ParentWithinSubtree(pNode);
    if (!pParent) {
      return nullptr;
    }
    NodeType* pCurrent = TraverseStrategy::GetFirstChild(pParent);
    NodeType* pPrevious = nullptr;
    while (pCurrent != pNode) {
      pPrevious = pCurrent;
      pCurrent = TraverseStrategy::GetNextSibling(pCurrent);
    }
    return pPrevious;
  }

  NodeType* LastChild(NodeType* pNode) {
    NodeType* pPrevious = nullptr;
    NodeType* pChild = TraverseStrategy::GetFirstChild(pNode);
    while (pChild) {
      pPrevious = pChild;
      pChild = NextSiblingWithinSubtree(pChild);
    }
    return pPrevious;
  }

  NodeType* LastDescendant(NodeType* pNode) {
    NodeType* pChild = LastChild(pNode);
    return pChild ? LastDescendant(pChild) : pNode;
  }

  HolderType root_;
  HolderType current_;
};

#endif  // XFA_FXFA_PARSER_CXFA_NODEITERATORTEMPLATE_H_
