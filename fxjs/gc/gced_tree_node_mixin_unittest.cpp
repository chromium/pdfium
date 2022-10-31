// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/gc/gced_tree_node_mixin.h"

#include <map>

#include "core/fxcrt/observed_ptr.h"
#include "fxjs/gc/heap.h"
#include "testing/fxgc_unittest.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/v8_test_environment.h"
#include "v8/include/cppgc/allocation.h"
#include "v8/include/cppgc/persistent.h"

namespace {

class ObservableGCedTreeNodeMixinForTest
    : public cppgc::GarbageCollected<ObservableGCedTreeNodeMixinForTest>,
      public GCedTreeNodeMixin<ObservableGCedTreeNodeMixinForTest>,
      public Observable {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;

  // GCedTreeNodeMixin:
  void Trace(cppgc::Visitor* visitor) const override {
    GCedTreeNodeMixin<ObservableGCedTreeNodeMixinForTest>::Trace(visitor);
  }

 private:
  ObservableGCedTreeNodeMixinForTest() = default;
};

}  // namespace

class GCedTreeNodeMixinUnitTest : public FXGCUnitTest {
 public:
  GCedTreeNodeMixinUnitTest() = default;
  ~GCedTreeNodeMixinUnitTest() override = default;

  // FXGCUnitTest:
  void TearDown() override {
    root_ = nullptr;  // Can't (yet) outlive |FXGCUnitTest::heap_|.
    FXGCUnitTest::TearDown();
  }

  ObservableGCedTreeNodeMixinForTest* root() const { return root_; }
  void CreateRoot() { root_ = CreateNode(); }

  ObservableGCedTreeNodeMixinForTest* CreateNode() {
    return cppgc::MakeGarbageCollected<ObservableGCedTreeNodeMixinForTest>(
        heap()->GetAllocationHandle());
  }

  void AddClutterToFront(ObservableGCedTreeNodeMixinForTest* parent) {
    for (int i = 0; i < 4; ++i) {
      parent->AppendFirstChild(
          cppgc::MakeGarbageCollected<ObservableGCedTreeNodeMixinForTest>(
              heap()->GetAllocationHandle()));
    }
  }

  void AddClutterToBack(ObservableGCedTreeNodeMixinForTest* parent) {
    for (int i = 0; i < 4; ++i) {
      parent->AppendLastChild(
          cppgc::MakeGarbageCollected<ObservableGCedTreeNodeMixinForTest>(
              heap()->GetAllocationHandle()));
    }
  }

 private:
  cppgc::Persistent<ObservableGCedTreeNodeMixinForTest> root_;
};

TEST_F(GCedTreeNodeMixinUnitTest, OneRefence) {
  CreateRoot();
  ObservedPtr<ObservableGCedTreeNodeMixinForTest> watcher(root());
  ForceGCAndPump();
  EXPECT_TRUE(watcher);
}

TEST_F(GCedTreeNodeMixinUnitTest, NoReferences) {
  ObservedPtr<ObservableGCedTreeNodeMixinForTest> watcher(CreateNode());
  ForceGCAndPump();
  EXPECT_FALSE(watcher);
}

TEST_F(GCedTreeNodeMixinUnitTest, FirstHasParent) {
  CreateRoot();
  ObservedPtr<ObservableGCedTreeNodeMixinForTest> watcher(CreateNode());
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

TEST_F(GCedTreeNodeMixinUnitTest, RemoveSelf) {
  CreateRoot();
  ObservedPtr<ObservableGCedTreeNodeMixinForTest> watcher(CreateNode());
  root()->AppendFirstChild(watcher.Get());
  ForceGCAndPump();
  EXPECT_TRUE(root());
  ASSERT_TRUE(watcher);
  watcher->RemoveSelfIfParented();
  ForceGCAndPump();
  EXPECT_TRUE(root());
  EXPECT_FALSE(watcher);
}

TEST_F(GCedTreeNodeMixinUnitTest, InsertBeforeAfter) {
  CreateRoot();
  AddClutterToFront(root());
  ObservedPtr<ObservableGCedTreeNodeMixinForTest> watcher(CreateNode());
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

TEST_F(GCedTreeNodeMixinUnitTest, AsMapKey) {
  std::map<cppgc::Persistent<ObservableGCedTreeNodeMixinForTest>, int> score;
  ObservableGCedTreeNodeMixinForTest* node = CreateNode();
  score[node] = 100;
  EXPECT_EQ(100, score[node]);
}
