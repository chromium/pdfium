// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_RETAINED_TREE_NODE_H_
#define CORE_FXCRT_RETAINED_TREE_NODE_H_

#include "core/fxcrt/retain_ptr.h"
#include "third_party/base/logging.h"

namespace fxcrt {

// For DOM/XML-ish trees, where references outside the tree are RetainPtr<T>,
// and the parent node also "retains" its children but doesn't always have
// a direct pointer to them.
template <typename T>
class RetainedTreeNode {
 public:
  template <typename U, typename... Args>
  friend RetainPtr<U> pdfium::MakeRetain(Args&&... args);

  T* GetParent() const { return m_pParent; }
  T* GetFirstChild() const { return m_pFirstChild; }
  T* GetLastChild() const { return m_pLastChild; }
  T* GetNextSibling() const { return m_pNextSibling; }
  T* GetPrevSibling() const { return m_pPrevSibling; }

  void AppendFirstChild(const RetainPtr<T>& child) {
    BecomeParent(child);
    if (m_pFirstChild) {
      m_pFirstChild->m_pPrevSibling = child.Get();
      child->m_pNextSibling = m_pFirstChild;
      m_pFirstChild = child.Get();
    } else {
      m_pFirstChild = child.Get();
      m_pLastChild = child.Get();
    }
  }

  void AppendLastChild(const RetainPtr<T>& child) {
    BecomeParent(child);
    if (m_pLastChild) {
      m_pLastChild->m_pNextSibling = child.Get();
      child->m_pPrevSibling = m_pLastChild;
      m_pLastChild = child.Get();
    } else {
      m_pFirstChild = child.Get();
      m_pLastChild = child.Get();
    }
  }

  void InsertBefore(const RetainPtr<T>& child, T* other) {
    if (!other) {
      AppendLastChild(child);
      return;
    }
    CHECK(other->m_pParent == this);
    BecomeParent(child);
    child->m_pNextSibling = other;
    child->m_pPrevSibling = other->m_pPrevSibling;
    if (other->m_pPrevSibling)
      other->m_pPrevSibling->m_pNextSibling = child.Get();
    else
      m_pFirstChild = child.Get();
    other->m_pPrevSibling = child.Get();
  }

  void InsertAfter(const RetainPtr<T>& child, T* other) {
    if (!other) {
      AppendFirstChild(child);
      return;
    }
    CHECK(other->m_pParent == this);
    BecomeParent(child);
    child->m_pNextSibling = other->m_pNextSibling;
    child->m_pPrevSibling = other;
    if (other->m_pNextSibling)
      other->m_pNextSibling->m_pPrevSibling = child.Get();
    else
      m_pLastChild = child.Get();
    other->m_pNextSibling = child.Get();
  }

  void RemoveChild(const RetainPtr<T>& child) {
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

 protected:
  RetainedTreeNode() = default;
  virtual ~RetainedTreeNode() {
    while (m_pFirstChild)
      RemoveChild(pdfium::WrapRetain(m_pFirstChild));
  }

 private:
  template <typename U>
  friend struct ReleaseDeleter;

  template <typename U>
  friend class RetainPtr;

  RetainedTreeNode(const RetainedTreeNode& that) = delete;
  RetainedTreeNode& operator=(const RetainedTreeNode& that) = delete;

  void Retain() { ++m_nRefCount; }
  void Release() {
    ASSERT(m_nRefCount > 0);
    if (--m_nRefCount == 0 && !m_pParent)
      delete this;
  }

  // Child left in state where sibling members need subsequent adjustment.
  void BecomeParent(const RetainPtr<T>& child) {
    if (child->m_pParent)
      child->m_pParent->RemoveChild(child);
    child->m_pParent = static_cast<T*>(this);
    ASSERT(!child->m_pNextSibling);
    ASSERT(!child->m_pPrevSibling);
  }

  intptr_t m_nRefCount = 0;
  T* m_pParent = nullptr;       // Raw, intra-tree pointer.
  T* m_pFirstChild = nullptr;   // Raw, intra-tree pointer.
  T* m_pLastChild = nullptr;    // Raw, intra-tree pointer.
  T* m_pNextSibling = nullptr;  // Raw, intra-tree pointer
  T* m_pPrevSibling = nullptr;  // Raw, intra-tree pointer
};

}  // namespace fxcrt

using fxcrt::RetainedTreeNode;

#endif  // CORE_FXCRT_RETAINED_TREE_NODE_H_
