// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FXJS_GC_GCED_TREE_NODE_H_
#define FXJS_GC_GCED_TREE_NODE_H_

#include "core/fxcrt/tree_node.h"
#include "third_party/base/logging.h"
#include "v8/include/cppgc/allocation.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"

namespace fxjs {

// For DOM/XML-ish trees, where references outside the tree are RetainPtr<T>,
// and the parent node also "retains" its children but doesn't always have
// a direct pointer to them.
template <typename T>
class GCedTreeNode : public TreeNode<T, cppgc::Member<T>>,
                     public cppgc::GarbageCollected<GCedTreeNode<T>> {
 public:
  using TreeNode<T, cppgc::Member<T>>::RemoveChild;

  virtual void Trace(cppgc::Visitor* visitor) const {
    visitor->Trace(TreeNode<T, cppgc::Member<T>>::RawParent());
    visitor->Trace(TreeNode<T, cppgc::Member<T>>::RawFirstChild());
    visitor->Trace(TreeNode<T, cppgc::Member<T>>::RawLastChild());
    visitor->Trace(TreeNode<T, cppgc::Member<T>>::RawNextSibling());
    visitor->Trace(TreeNode<T, cppgc::Member<T>>::RawPrevSibling());
  }

 protected:
  GCedTreeNode() = default;
  GCedTreeNode(const GCedTreeNode& that) = delete;
  GCedTreeNode& operator=(const GCedTreeNode& that) = delete;
};

}  // namespace fxjs

using fxjs::GCedTreeNode;

#endif  // FXJS_GC_GCED_TREE_NODE_H_
