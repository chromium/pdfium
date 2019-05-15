// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_RETAINED_TREE_NODE_H_
#define CORE_FXCRT_RETAINED_TREE_NODE_H_

#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/tree_node.h"
#include "third_party/base/logging.h"

namespace fxcrt {

// For DOM/XML-ish trees, where references outside the tree are RetainPtr<T>,
// and the parent node also "retains" its children but doesn't always have
// a direct pointer to them.
template <typename T>
class RetainedTreeNode : public TreeNode<T> {
 public:
  template <typename U, typename... Args>
  friend RetainPtr<U> pdfium::MakeRetain(Args&&... args);

  void AppendFirstChild(const RetainPtr<T>& child) {
    TreeNode<T>::AppendFirstChild(child.Get());
  }

  void AppendLastChild(const RetainPtr<T>& child) {
    TreeNode<T>::AppendLastChild(child.Get());
  }

  void InsertBefore(const RetainPtr<T>& child, T* other) {
    TreeNode<T>::InsertBefore(child.Get(), other);
  }

  void InsertAfter(const RetainPtr<T>& child, T* other) {
    TreeNode<T>::InsertAfter(child.Get(), other);
  }

  void RemoveChild(const RetainPtr<T>& child) {
    TreeNode<T>::RemoveChild(child.Get());
  }

  void RemoveSelfIfParented() {
    if (T* parent = TreeNode<T>::GetParent()) {
      parent->TreeNode<T>::RemoveChild(
          pdfium::WrapRetain(static_cast<T*>(this)).Get());
    }
  }

 protected:
  RetainedTreeNode() = default;
  ~RetainedTreeNode() override {
    while (auto* pChild = TreeNode<T>::GetFirstChild())
      RemoveChild(pdfium::WrapRetain(pChild));
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
    if (--m_nRefCount == 0 && !TreeNode<T>::GetParent())
      delete this;
  }

  intptr_t m_nRefCount = 0;
};

}  // namespace fxcrt

using fxcrt::RetainedTreeNode;

#endif  // CORE_FXCRT_RETAINED_TREE_NODE_H_
