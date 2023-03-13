// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_TREE_NODE_H_
#define CORE_FXCRT_TREE_NODE_H_

#include <stdint.h>

#include "third_party/base/check.h"

namespace fxcrt {

// Implements the usual DOM/XML-ish trees allowing for a variety of
// pointer types with which to connect the nodes.

template <typename T>
class TreeNodeBase {
 public:
  TreeNodeBase() = default;
  virtual ~TreeNodeBase() = default;

  T* GetParent() const { return actual().m_pParent; }
  T* GetFirstChild() const { return actual().m_pFirstChild; }
  T* GetLastChild() const { return actual().m_pLastChild; }
  T* GetNextSibling() const { return actual().m_pNextSibling; }
  T* GetPrevSibling() const { return actual().m_pPrevSibling; }

  bool HasChild(const T* child) const {
    return child != this && child->GetParent() == this;
  }

  T* GetNthChild(int32_t n) {
    if (n < 0)
      return nullptr;
    T* result = GetFirstChild();
    while (n-- && result) {
      result = result->GetNextSibling();
    }
    return result;
  }

  void AppendFirstChild(T* child) {
    BecomeParent(child);
    if (actual().m_pFirstChild) {
      CHECK(actual().m_pLastChild);
      actual().m_pFirstChild->m_pPrevSibling = child;
      child->m_pNextSibling = actual().m_pFirstChild;
      actual().m_pFirstChild = child;
    } else {
      CHECK(!actual().m_pLastChild);
      actual().m_pFirstChild = child;
      actual().m_pLastChild = child;
    }
  }

  void AppendLastChild(T* child) {
    BecomeParent(child);
    if (actual().m_pLastChild) {
      CHECK(actual().m_pFirstChild);
      actual().m_pLastChild->m_pNextSibling = child;
      child->m_pPrevSibling = actual().m_pLastChild;
      actual().m_pLastChild = child;
    } else {
      CHECK(!actual().m_pFirstChild);
      actual().m_pFirstChild = child;
      actual().m_pLastChild = child;
    }
  }

  void InsertBefore(T* child, T* other) {
    if (!other) {
      AppendLastChild(child);
      return;
    }
    BecomeParent(child);
    CHECK(HasChild(other));
    child->m_pNextSibling = other;
    child->m_pPrevSibling = other->m_pPrevSibling;
    if (actual().m_pFirstChild == other) {
      CHECK(!other->m_pPrevSibling);
      actual().m_pFirstChild = child;
    } else {
      other->m_pPrevSibling->m_pNextSibling = child;
    }
    other->m_pPrevSibling = child;
  }

  void InsertAfter(T* child, T* other) {
    if (!other) {
      AppendFirstChild(child);
      return;
    }
    BecomeParent(child);
    CHECK(HasChild(other));
    child->m_pNextSibling = other->m_pNextSibling;
    child->m_pPrevSibling = other;
    if (actual().m_pLastChild == other) {
      CHECK(!other->m_pNextSibling);
      actual().m_pLastChild = child;
    } else {
      other->m_pNextSibling->m_pPrevSibling = child;
    }
    other->m_pNextSibling = child;
  }

  void RemoveChild(T* child) {
    CHECK(HasChild(child));
    if (actual().m_pLastChild == child) {
      CHECK(!child->m_pNextSibling);
      actual().m_pLastChild = child->m_pPrevSibling;
    } else {
      child->m_pNextSibling->m_pPrevSibling = child->m_pPrevSibling;
    }
    if (actual().m_pFirstChild == child) {
      CHECK(!child->m_pPrevSibling);
      actual().m_pFirstChild = child->m_pNextSibling;
    } else {
      child->m_pPrevSibling->m_pNextSibling = child->m_pNextSibling;
    }
    child->m_pParent = nullptr;
    child->m_pPrevSibling = nullptr;
    child->m_pNextSibling = nullptr;
  }

  void RemoveAllChildren() {
    while (T* child = GetFirstChild())
      RemoveChild(child);
  }

  void RemoveSelfIfParented() {
    if (T* parent = GetParent())
      parent->RemoveChild(static_cast<T*>(this));
  }

 private:
  inline T& actual() { return static_cast<T&>(*this); }
  inline const T& actual() const { return static_cast<const T&>(*this); }

  // Child left in state where sibling members need subsequent adjustment.
  void BecomeParent(T* child) {
    CHECK(child != this);  // Detect attempts at self-insertion.
    if (child->m_pParent)
      child->m_pParent->TreeNodeBase<T>::RemoveChild(child);
    child->m_pParent = static_cast<T*>(this);
    CHECK(!child->m_pNextSibling);
    CHECK(!child->m_pPrevSibling);
  }
};

// Tree connected using C-style pointers.
template <typename T>
class TreeNode : public TreeNodeBase<T> {
 public:
  TreeNode() = default;
  virtual ~TreeNode() = default;

 private:
  friend class TreeNodeBase<T>;

  T* m_pParent = nullptr;       // Raw, intra-tree pointer.
  T* m_pFirstChild = nullptr;   // Raw, intra-tree pointer.
  T* m_pLastChild = nullptr;    // Raw, intra-tree pointer.
  T* m_pNextSibling = nullptr;  // Raw, intra-tree pointer
  T* m_pPrevSibling = nullptr;  // Raw, intra-tree pointer
};

}  // namespace fxcrt

using fxcrt::TreeNode;

#endif  // CORE_FXCRT_TREE_NODE_H_
