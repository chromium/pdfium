// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/retained_tree_node.h"

#include "core/fxcrt/observable.h"
#include "core/fxcrt/retain_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace fxcrt {
namespace {

class ObservableRetainedTreeNodeForTest
    : public RetainedTreeNode<ObservableRetainedTreeNodeForTest>,
      public Observable<ObservableRetainedTreeNodeForTest> {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

 private:
  ObservableRetainedTreeNodeForTest() = default;
};

void AddClutterToFront(
    const RetainPtr<ObservableRetainedTreeNodeForTest>& parent) {
  for (int i = 0; i < 4; ++i) {
    parent->AppendFirstChild(
        pdfium::MakeRetain<ObservableRetainedTreeNodeForTest>());
  }
}

void AddClutterToBack(
    const RetainPtr<ObservableRetainedTreeNodeForTest>& parent) {
  for (int i = 0; i < 4; ++i) {
    parent->AppendLastChild(
        pdfium::MakeRetain<ObservableRetainedTreeNodeForTest>());
  }
}

}  // namespace

TEST(RetainedTreeNode, NoParent) {
  ObservableRetainedTreeNodeForTest::ObservedPtr watcher;
  {
    RetainPtr<ObservableRetainedTreeNodeForTest> ptr =
        pdfium::MakeRetain<ObservableRetainedTreeNodeForTest>();
    EXPECT_FALSE(ptr->HasChild(ptr.Get()));
    watcher = ObservableRetainedTreeNodeForTest::ObservedPtr(ptr.Get());
    EXPECT_TRUE(watcher.Get());
  }
  EXPECT_FALSE(watcher.Get());
}

TEST(RetainedTreeNode, FirstHasParent) {
  ObservableRetainedTreeNodeForTest::ObservedPtr watcher;
  RetainPtr<ObservableRetainedTreeNodeForTest> parent =
      pdfium::MakeRetain<ObservableRetainedTreeNodeForTest>();
  {
    RetainPtr<ObservableRetainedTreeNodeForTest> ptr =
        pdfium::MakeRetain<ObservableRetainedTreeNodeForTest>();
    watcher = ObservableRetainedTreeNodeForTest::ObservedPtr(ptr.Get());
    parent->AppendFirstChild(ptr);
    EXPECT_FALSE(parent->HasChild(parent.Get()));
    EXPECT_TRUE(parent->HasChild(ptr.Get()));
    EXPECT_TRUE(watcher.Get());
  }
  EXPECT_TRUE(watcher.Get());
  parent->RemoveChild(pdfium::WrapRetain(watcher.Get()));
  EXPECT_FALSE(watcher.Get());
  // Now add some clutter.
  {
    RetainPtr<ObservableRetainedTreeNodeForTest> ptr =
        pdfium::MakeRetain<ObservableRetainedTreeNodeForTest>();
    watcher = ObservableRetainedTreeNodeForTest::ObservedPtr(ptr.Get());
    parent->AppendFirstChild(ptr);
    AddClutterToFront(parent);
    AddClutterToBack(parent);
    EXPECT_TRUE(watcher.Get());
  }
  EXPECT_TRUE(watcher.Get());
  parent->RemoveChild(pdfium::WrapRetain(watcher.Get()));
  EXPECT_FALSE(watcher.Get());
}

TEST(RetainedTreeNode, LastHasParent) {
  ObservableRetainedTreeNodeForTest::ObservedPtr watcher;
  RetainPtr<ObservableRetainedTreeNodeForTest> parent =
      pdfium::MakeRetain<ObservableRetainedTreeNodeForTest>();
  {
    RetainPtr<ObservableRetainedTreeNodeForTest> ptr =
        pdfium::MakeRetain<ObservableRetainedTreeNodeForTest>();
    watcher = ObservableRetainedTreeNodeForTest::ObservedPtr(ptr.Get());
    parent->AppendLastChild(ptr);
    EXPECT_FALSE(parent->HasChild(parent.Get()));
    EXPECT_TRUE(parent->HasChild(ptr.Get()));
    EXPECT_TRUE(watcher.Get());
  }
  {
    // Now add some clutter.
    RetainPtr<ObservableRetainedTreeNodeForTest> ptr =
        pdfium::MakeRetain<ObservableRetainedTreeNodeForTest>();
    watcher = ObservableRetainedTreeNodeForTest::ObservedPtr(ptr.Get());
    parent->AppendLastChild(ptr);
    AddClutterToFront(parent);
    AddClutterToBack(parent);
    EXPECT_TRUE(watcher.Get());
  }
  EXPECT_TRUE(watcher.Get());
  parent->RemoveChild(pdfium::WrapRetain(watcher.Get()));
  EXPECT_FALSE(watcher.Get());
}

TEST(RetainedTreeNode, GrandChildCleanedUp) {
  ObservableRetainedTreeNodeForTest::ObservedPtr watcher;
  RetainPtr<ObservableRetainedTreeNodeForTest> grandparent =
      pdfium::MakeRetain<ObservableRetainedTreeNodeForTest>();
  {
    RetainPtr<ObservableRetainedTreeNodeForTest> parent =
        pdfium::MakeRetain<ObservableRetainedTreeNodeForTest>();
    grandparent->AppendFirstChild(parent);
    {
      RetainPtr<ObservableRetainedTreeNodeForTest> ptr =
          pdfium::MakeRetain<ObservableRetainedTreeNodeForTest>();
      watcher = ObservableRetainedTreeNodeForTest::ObservedPtr(ptr.Get());
      parent->AppendFirstChild(ptr);
      EXPECT_TRUE(watcher.Get());
    }
    grandparent->RemoveChild(parent);
    EXPECT_TRUE(watcher.Get());
  }
  EXPECT_FALSE(watcher.Get());
}

TEST(RetainedTreeNode, RemoveSelf) {
  ObservableRetainedTreeNodeForTest::ObservedPtr watcher;
  auto parent = pdfium::MakeRetain<ObservableRetainedTreeNodeForTest>();
  {
    auto child = pdfium::MakeRetain<ObservableRetainedTreeNodeForTest>();
    watcher = ObservableRetainedTreeNodeForTest::ObservedPtr(child.Get());
    parent->AppendFirstChild(child);
  }
  EXPECT_TRUE(watcher.Get());
  watcher->RemoveSelfIfParented();
  EXPECT_FALSE(watcher.Get());
}

TEST(RetainedTreeNode, InsertBeforeAfter) {
  ObservableRetainedTreeNodeForTest::ObservedPtr watcher;
  RetainPtr<ObservableRetainedTreeNodeForTest> parent =
      pdfium::MakeRetain<ObservableRetainedTreeNodeForTest>();

  AddClutterToFront(parent);
  {
    RetainPtr<ObservableRetainedTreeNodeForTest> ptr =
        pdfium::MakeRetain<ObservableRetainedTreeNodeForTest>();
    watcher = ObservableRetainedTreeNodeForTest::ObservedPtr(ptr.Get());
    parent->AppendFirstChild(ptr);
    parent->InsertBefore(pdfium::WrapRetain(parent->GetFirstChild()),
                         parent->GetLastChild());
    parent->InsertAfter(pdfium::WrapRetain(parent->GetLastChild()),
                        parent->GetFirstChild());
    EXPECT_TRUE(watcher.Get());
  }
  parent->RemoveChild(pdfium::WrapRetain(watcher.Get()));
  EXPECT_FALSE(watcher.Get());
}

TEST(RetainedTreeNode, Traversal) {
  RetainPtr<ObservableRetainedTreeNodeForTest> parent =
      pdfium::MakeRetain<ObservableRetainedTreeNodeForTest>();

  AddClutterToFront(parent);
  int count = 0;
  for (ObservableRetainedTreeNodeForTest* pNode = parent->GetFirstChild();
       pNode; pNode = pNode->GetNextSibling()) {
    ++count;
  }
  EXPECT_EQ(4, count);
  count = 0;
  for (ObservableRetainedTreeNodeForTest* pNode = parent->GetLastChild(); pNode;
       pNode = pNode->GetPrevSibling()) {
    ++count;
  }
  EXPECT_EQ(4, count);
}

}  // namespace fxcrt
