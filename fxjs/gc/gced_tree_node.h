// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FXJS_GC_GCED_TREE_NODE_H_
#define FXJS_GC_GCED_TREE_NODE_H_

#include "core/fxcrt/tree_node.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"

namespace fxjs {

// For DOM/XML-ish trees, where references outside the tree are Persistent<>.
template <typename T>
class GCedTreeNode : public cppgc::GarbageCollected<GCedTreeNode<T>>,
                     public fxcrt::TreeNodeBase<T> {
 public:
  virtual void Trace(cppgc::Visitor* visitor) const {
    visitor->Trace(parent_);
    visitor->Trace(first_child_);
    visitor->Trace(last_child_);
    visitor->Trace(next_sibling_);
    visitor->Trace(prev_sibling_);
  }

 protected:
  GCedTreeNode() = default;
  GCedTreeNode(const GCedTreeNode& that) = delete;
  GCedTreeNode& operator=(const GCedTreeNode& that) = delete;

 private:
  friend class fxcrt::TreeNodeBase<T>;

  cppgc::Member<T> parent_;
  cppgc::Member<T> first_child_;
  cppgc::Member<T> last_child_;
  cppgc::Member<T> next_sibling_;
  cppgc::Member<T> prev_sibling_;
};

}  // namespace fxjs

using fxjs::GCedTreeNode;

#endif  // FXJS_GC_GCED_TREE_NODE_H_
