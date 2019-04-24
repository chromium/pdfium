// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_TREE_NODE_H_
#define CORE_FXCRT_TREE_NODE_H_

#include "core/fxcrt/fx_system.h"
#include "third_party/base/logging.h"

namespace fxcrt {

// Implements the usual DOM/XML-ish trees.
template <typename T>
class TreeNode {
 public:
  TreeNode() = default;
  virtual ~TreeNode() = default;

  T* GetParent() const { return m_pParent; }
  T* GetFirstChild() const { return m_pFirstChild; }
  T* GetLastChild() const { return m_pLastChild; }
  T* GetNextSibling() const { return m_pNextSibling; }
  T* GetPrevSibling() const { return m_pPrevSibling; }

  void AppendFirstChild(T* child) {
    BecomeParent(child);
    if (m_pFirstChild) {
      m_pFirstChild->m_pPrevSibling = child;
      child->m_pNextSibling = m_pFirstChild;
      m_pFirstChild = child;
    } else {
      m_pFirstChild = child;
      m_pLastChild = child;
    }
  }

  void AppendLastChild(T* child) {
    BecomeParent(child);
    if (m_pLastChild) {
      m_pLastChild->m_pNextSibling = child;
      child->m_pPrevSibling = m_pLastChild;
      m_pLastChild = child;
    } else {
      m_pFirstChild = child;
      m_pLastChild = child;
    }
  }

  void InsertBefore(T* child, T* other) {
    if (!other) {
      AppendLastChild(child);
      return;
    }
    CHECK(other->m_pParent == this);
    BecomeParent(child);
    child->m_pNextSibling = other;
    child->m_pPrevSibling = other->m_pPrevSibling;
    if (other->m_pPrevSibling)
      other->m_pPrevSibling->m_pNextSibling = child;
    else
      m_pFirstChild = child;
    other->m_pPrevSibling = child;
  }

  void InsertAfter(T* child, T* other) {
    if (!other) {
      AppendFirstChild(child);
      return;
    }
    CHECK(other->m_pParent == this);
    BecomeParent(child);
    child->m_pNextSibling = other->m_pNextSibling;
    child->m_pPrevSibling = other;
    if (other->m_pNextSibling)
      other->m_pNextSibling->m_pPrevSibling = child;
    else
      m_pLastChild = child;
    other->m_pNextSibling = child;
  }

  void RemoveChild(T* child) {
    CHECK(child->m_pParent == this);
    if (child->m_pNextSibling)
      child->m_pNextSibling->m_pPrevSibling = child->m_pPrevSibling;
    else
      m_pLastChild = child->m_pPrevSibling;

    if (child->m_pPrevSibling)
      child->m_pPrevSibling->m_pNextSibling = child->m_pNextSibling;
    else
      m_pFirstChild = child->m_pNextSibling;

    child->m_pParent = nullptr;
    child->m_pPrevSibling = nullptr;
    child->m_pNextSibling = nullptr;
  }

 private:
  // Child left in state where sibling members need subsequent adjustment.
  void BecomeParent(T* child) {
    if (child->m_pParent)
      child->m_pParent->TreeNode<T>::RemoveChild(child);
    child->m_pParent = static_cast<T*>(this);
    ASSERT(!child->m_pNextSibling);
    ASSERT(!child->m_pPrevSibling);
  }

  T* m_pParent = nullptr;       // Raw, intra-tree pointer.
  T* m_pFirstChild = nullptr;   // Raw, intra-tree pointer.
  T* m_pLastChild = nullptr;    // Raw, intra-tree pointer.
  T* m_pNextSibling = nullptr;  // Raw, intra-tree pointer
  T* m_pPrevSibling = nullptr;  // Raw, intra-tree pointer
};

}  // namespace fxcrt

using fxcrt::TreeNode;

#endif  // CORE_FXCRT_TREE_NODE_H_
