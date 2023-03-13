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
    visitor->Trace(m_pParent);
    visitor->Trace(m_pFirstChild);
    visitor->Trace(m_pLastChild);
    visitor->Trace(m_pNextSibling);
    visitor->Trace(m_pPrevSibling);
  }

 protected:
  GCedTreeNode() = default;
  GCedTreeNode(const GCedTreeNode& that) = delete;
  GCedTreeNode& operator=(const GCedTreeNode& that) = delete;

 private:
  friend class fxcrt::TreeNodeBase<T>;

  cppgc::Member<T> m_pParent;
  cppgc::Member<T> m_pFirstChild;
  cppgc::Member<T> m_pLastChild;
  cppgc::Member<T> m_pNextSibling;
  cppgc::Member<T> m_pPrevSibling;
};

}  // namespace fxjs

using fxjs::GCedTreeNode;

#endif  // FXJS_GC_GCED_TREE_NODE_H_
