// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/gc/gced_tree_node_mixin.h"

#include <map>

#include "core/fxcrt/observed_ptr.h"
#include "fxjs/gc/heap.h"
#include "testing/fxgc_unittest.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/v8_test_environment.h"
#include "third_party/base/stl_util.h"
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
  static cppgc::Persistent<ObservableGCedTreeNodeMixinForTest> s_root;

  GCedTreeNodeMixinUnitTest() = default;
  ~GCedTreeNodeMixinUnitTest() override = default;

  // FXGCUnitTest:
  void TearDown() override {
    s_root = nullptr;  // Can't (yet) outlive |heap_|.
    FXGCUnitTest::TearDown();
  }

  ObservableGCedTreeNodeMixinForTest* CreateNode() {
    return cppgc::MakeGarbageCollected<ObservableGCedTreeNodeMixinForTest>(
        heap()->GetAllocationHandle());
  }

  void ForceGCAndPump() {
    FXGC_ForceGarbageCollection(heap());
    V8TestEnvironment::PumpPlatformMessageLoop(isolate());
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
  FXGCScopedHeap heap_;
};

cppgc::Persistent<ObservableGCedTreeNodeMixinForTest>
    GCedTreeNodeMixinUnitTest::s_root;

TEST_F(GCedTreeNodeMixinUnitTest, OneRefence) {
  s_root = CreateNode();
  ObservedPtr<ObservableGCedTreeNodeMixinForTest> watcher(s_root);
  ForceGCAndPump();
  EXPECT_TRUE(watcher);
}

TEST_F(GCedTreeNodeMixinUnitTest, NoReferences) {
  ObservedPtr<ObservableGCedTreeNodeMixinForTest> watcher(CreateNode());
  ForceGCAndPump();
  EXPECT_FALSE(watcher);
}

TEST_F(GCedTreeNodeMixinUnitTest, FirstHasParent) {
  s_root = CreateNode();
  ObservedPtr<ObservableGCedTreeNodeMixinForTest> watcher(CreateNode());
  s_root->AppendFirstChild(watcher.Get());
  ForceGCAndPump();
  ASSERT_TRUE(s_root);
  EXPECT_TRUE(watcher);
  s_root->RemoveChild(watcher.Get());
  ForceGCAndPump();
  ASSERT_TRUE(s_root);
  EXPECT_FALSE(watcher);

  // Now add some clutter.
  watcher.Reset(CreateNode());
  s_root->AppendFirstChild(watcher.Get());
  AddClutterToFront(s_root);
  AddClutterToBack(s_root);
  ForceGCAndPump();
  ASSERT_TRUE(s_root);
  EXPECT_TRUE(watcher);
  s_root->RemoveChild(watcher.Get());
  ForceGCAndPump();
  EXPECT_TRUE(s_root);
  EXPECT_FALSE(watcher);
}

TEST_F(GCedTreeNodeMixinUnitTest, RemoveSelf) {
  s_root = CreateNode();
  ObservedPtr<ObservableGCedTreeNodeMixinForTest> watcher(CreateNode());
  s_root->AppendFirstChild(watcher.Get());
  ForceGCAndPump();
  EXPECT_TRUE(s_root);
  ASSERT_TRUE(watcher);
  watcher->RemoveSelfIfParented();
  ForceGCAndPump();
  EXPECT_TRUE(s_root);
  EXPECT_FALSE(watcher);
}

TEST_F(GCedTreeNodeMixinUnitTest, InsertBeforeAfter) {
  s_root = CreateNode();
  AddClutterToFront(s_root);
  ObservedPtr<ObservableGCedTreeNodeMixinForTest> watcher(CreateNode());
  s_root->AppendFirstChild(watcher.Get());
  s_root->InsertBefore(s_root->GetFirstChild(), s_root->GetLastChild());
  s_root->InsertAfter(s_root->GetLastChild(), s_root->GetFirstChild());
  ForceGCAndPump();
  ASSERT_TRUE(s_root);
  EXPECT_TRUE(watcher);
  s_root->RemoveChild(watcher.Get());
  ForceGCAndPump();
  EXPECT_TRUE(s_root);
  EXPECT_FALSE(watcher);
}

TEST_F(GCedTreeNodeMixinUnitTest, AsMapKey) {
  std::map<cppgc::Persistent<ObservableGCedTreeNodeMixinForTest>, int> score;
  ObservableGCedTreeNodeMixinForTest* node = CreateNode();
  score[node] = 100;
  EXPECT_EQ(100, score[node]);
}
