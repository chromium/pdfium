// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FXJS_GC_GCED_TREE_NODE_MIXIN_H_
#define FXJS_GC_GCED_TREE_NODE_MIXIN_H_

#include "core/fxcrt/tree_node.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"

namespace fxjs {

// For DOM/XML-ish trees, where references outside the tree are Persistent<>,
// usable by classes that are already garbage collected themselves.
template <typename T>
class GCedTreeNodeMixin : public cppgc::GarbageCollectedMixin,
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
  GCedTreeNodeMixin() = default;
  GCedTreeNodeMixin(const GCedTreeNodeMixin& that) = delete;
  GCedTreeNodeMixin& operator=(const GCedTreeNodeMixin& that) = delete;

 private:
  friend class fxcrt::TreeNodeBase<T>;

  cppgc::Member<T> m_pParent;
  cppgc::Member<T> m_pFirstChild;
  cppgc::Member<T> m_pLastChild;
  cppgc::Member<T> m_pNextSibling;
  cppgc::Member<T> m_pPrevSibling;
};

}  // namespace fxjs

using fxjs::GCedTreeNodeMixin;

#endif  // FXJS_GC_GCED_TREE_NODE_MIXIN_H_
