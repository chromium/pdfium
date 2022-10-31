// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/gc/gced_tree_node.h"

#include <map>

#include "core/fxcrt/observed_ptr.h"
#include "fxjs/gc/heap.h"
#include "testing/fxgc_unittest.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/v8_test_environment.h"
#include "v8/include/cppgc/allocation.h"
#include "v8/include/cppgc/persistent.h"

namespace {

class ObservableGCedTreeNodeForTest
    : public GCedTreeNode<ObservableGCedTreeNodeForTest>,
      public Observable {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;

 private:
  ObservableGCedTreeNodeForTest() = default;
};

}  // namespace

class GCedTreeNodeUnitTest : public FXGCUnitTest {
 public:
  GCedTreeNodeUnitTest() = default;
  ~GCedTreeNodeUnitTest() override = default;

  // FXGCUnitTest:
  void TearDown() override {
    root_ = nullptr;  // Can't (yet) outlive |FXGCUnitTest::heap_|.
    FXGCUnitTest::TearDown();
  }

  ObservableGCedTreeNodeForTest* root() const { return root_; }
  void CreateRoot() { root_ = CreateNode(); }

  ObservableGCedTreeNodeForTest* CreateNode() {
    return cppgc::MakeGarbageCollected<ObservableGCedTreeNodeForTest>(
        heap()->GetAllocationHandle());
  }

  void AddClutterToFront(ObservableGCedTreeNodeForTest* parent) {
    for (int i = 0; i < 4; ++i) {
      parent->AppendFirstChild(
          cppgc::MakeGarbageCollected<ObservableGCedTreeNodeForTest>(
              heap()->GetAllocationHandle()));
    }
  }

  void AddClutterToBack(ObservableGCedTreeNodeForTest* parent) {
    for (int i = 0; i < 4; ++i) {
      parent->AppendLastChild(
          cppgc::MakeGarbageCollected<ObservableGCedTreeNodeForTest>(
              heap()->GetAllocationHandle()));
    }
  }

 private:
  cppgc::Persistent<ObservableGCedTreeNodeForTest> root_;
};

TEST_F(GCedTreeNodeUnitTest, OneRefence) {
  CreateRoot();
  ObservedPtr<ObservableGCedTreeNodeForTest> watcher(root());
  ForceGCAndPump();
  EXPECT_TRUE(watcher);
}

TEST_F(GCedTreeNodeUnitTest, NoReferences) {
  ObservedPtr<ObservableGCedTreeNodeForTest> watcher(CreateNode());
  ForceGCAndPump();
  EXPECT_FALSE(watcher);
}

TEST_F(GCedTreeNodeUnitTest, FirstHasParent) {
  CreateRoot();
  ObservedPtr<ObservableGCedTreeNodeForTest> watcher(CreateNode());
  root()->AppendFirstChild(watcher.Get());
  ForceGCAndPump();
  ASSERT_TRUE(root());
  EXPECT_TRUE(watcher);
  root()->RemoveChild(watcher.Get());
  ForceGCAndPump();
  ASSERT_TRUE(root());
  EXPECT_FALSE(watcher);

  // Now add some clutter.
  watcher.Reset(CreateNode());
  root()->AppendFirstChild(watcher.Get());
  AddClutterToFront(root());
  AddClutterToBack(root());
  ForceGCAndPump();
  ASSERT_TRUE(root());
  EXPECT_TRUE(watcher);
  root()->RemoveChild(watcher.Get());
  ForceGCAndPump();
  EXPECT_TRUE(root());
  EXPECT_FALSE(watcher);
}

TEST_F(GCedTreeNodeUnitTest, RemoveSelf) {
  CreateRoot();
  ObservedPtr<ObservableGCedTreeNodeForTest> watcher(CreateNode());
  root()->AppendFirstChild(watcher.Get());
  ForceGCAndPump();
  EXPECT_TRUE(root());
  ASSERT_TRUE(watcher);
  watcher->RemoveSelfIfParented();
  ForceGCAndPump();
  EXPECT_TRUE(root());
  EXPECT_FALSE(watcher);
}

TEST_F(GCedTreeNodeUnitTest, InsertBeforeAfter) {
  CreateRoot();
  AddClutterToFront(root());
  ObservedPtr<ObservableGCedTreeNodeForTest> watcher(CreateNode());
  root()->AppendFirstChild(watcher.Get());
  root()->InsertBefore(root()->GetFirstChild(), root()->GetLastChild());
  root()->InsertAfter(root()->GetLastChild(), root()->GetFirstChild());
  ForceGCAndPump();
  ASSERT_TRUE(root());
  EXPECT_TRUE(watcher);
  root()->RemoveChild(watcher.Get());
  ForceGCAndPump();
  EXPECT_TRUE(root());
  EXPECT_FALSE(watcher);
}

TEST_F(GCedTreeNodeUnitTest, AsMapKey) {
  std::map<cppgc::Persistent<ObservableGCedTreeNodeForTest>, int> score;
  ObservableGCedTreeNodeForTest* node = CreateNode();
  score[node] = 100;
  EXPECT_EQ(100, score[node]);
}
