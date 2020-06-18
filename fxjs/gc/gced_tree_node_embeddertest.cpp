// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/gc/gced_tree_node.h"

#include "core/fxcrt/observed_ptr.h"
#include "fxjs/gc/heap.h"
#include "testing/gced_embeddertest.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/stl_util.h"
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

class GCedTreeNodeEmbedderTest : public GCedEmbedderTest {
 public:
  static cppgc::Persistent<ObservableGCedTreeNodeForTest> s_root;

  void SetUp() override {
    GCedEmbedderTest::SetUp();
    heap_ = FXGC_CreateHeap();
  }

  void TearDown() override {
    s_root = nullptr;  // Can't (yet) outlive |heap_|.
    heap_.reset();
    GCedEmbedderTest::TearDown();
  }

  cppgc::Heap* heap() const { return heap_.get(); }
  ObservableGCedTreeNodeForTest* CreateNode() {
    return cppgc::MakeGarbageCollected<ObservableGCedTreeNodeForTest>(
        heap_.get());
  }

  void ForceGCAndPump() {
    heap()->ForceGarbageCollectionSlow(
        "GCedTreeNodeEmbedderTest", "test",
        cppgc::Heap::StackState::kNoHeapPointers);
    PumpPlatformMessageLoop();
  }

  void AddClutterToFront(ObservableGCedTreeNodeForTest* parent) {
    for (int i = 0; i < 4; ++i) {
      parent->AppendFirstChild(
          cppgc::MakeGarbageCollected<ObservableGCedTreeNodeForTest>(
              heap_.get()));
    }
  }

  void AddClutterToBack(ObservableGCedTreeNodeForTest* parent) {
    for (int i = 0; i < 4; ++i) {
      parent->AppendLastChild(
          cppgc::MakeGarbageCollected<ObservableGCedTreeNodeForTest>(
              heap_.get()));
    }
  }

 private:
  FXGCScopedHeap heap_;
};

cppgc::Persistent<ObservableGCedTreeNodeForTest>
    GCedTreeNodeEmbedderTest::s_root;

TEST_F(GCedTreeNodeEmbedderTest, OneRefence) {
  s_root = CreateNode();
  ObservedPtr<ObservableGCedTreeNodeForTest> watcher(s_root);
  ForceGCAndPump();
  EXPECT_TRUE(watcher);
}

TEST_F(GCedTreeNodeEmbedderTest, NoReferences) {
  ObservedPtr<ObservableGCedTreeNodeForTest> watcher(CreateNode());
  ForceGCAndPump();
  EXPECT_FALSE(watcher);
}

TEST_F(GCedTreeNodeEmbedderTest, FirstHasParent) {
  s_root = CreateNode();
  ObservedPtr<ObservableGCedTreeNodeForTest> watcher(CreateNode());
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

TEST_F(GCedTreeNodeEmbedderTest, RemoveSelf) {
  s_root = CreateNode();
  ObservedPtr<ObservableGCedTreeNodeForTest> watcher(CreateNode());
  s_root->AppendFirstChild(watcher.Get());
  ForceGCAndPump();
  EXPECT_TRUE(s_root);
  ASSERT_TRUE(watcher);
  watcher->RemoveSelfIfParented();
  ForceGCAndPump();
  EXPECT_TRUE(s_root);
  EXPECT_FALSE(watcher);
}

TEST_F(GCedTreeNodeEmbedderTest, InsertBeforeAfter) {
  s_root = CreateNode();
  AddClutterToFront(s_root);
  ObservedPtr<ObservableGCedTreeNodeForTest> watcher(CreateNode());
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
