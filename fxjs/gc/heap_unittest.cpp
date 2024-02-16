// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/gc/heap.h"

#include <memory>
#include <set>

#include "core/fxcrt/containers/contains.h"
#include "testing/fxgc_unittest.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/v8_test_environment.h"
#include "v8/include/cppgc/allocation.h"
#include "v8/include/cppgc/persistent.h"

namespace {

class PseudoCollectible : public cppgc::GarbageCollected<PseudoCollectible> {
 public:
  static void ClearCounts() {
    s_live_.clear();
    s_dead_.clear();
  }
  static size_t LiveCount() { return s_live_.size(); }
  static size_t DeadCount() { return s_dead_.size(); }

  PseudoCollectible() { s_live_.insert(this); }
  virtual ~PseudoCollectible() {
    s_live_.erase(this);
    s_dead_.insert(this);
  }

  bool IsLive() const { return pdfium::Contains(s_live_, this); }

  virtual void Trace(cppgc::Visitor* visitor) const {}

 private:
  static std::set<const PseudoCollectible*> s_live_;
  static std::set<const PseudoCollectible*> s_dead_;
};

std::set<const PseudoCollectible*> PseudoCollectible::s_live_;
std::set<const PseudoCollectible*> PseudoCollectible::s_dead_;

class CollectibleHolder {
 public:
  explicit CollectibleHolder(PseudoCollectible* holdee) : holdee_(holdee) {}
  ~CollectibleHolder() = default;

  PseudoCollectible* holdee() const { return holdee_; }

 private:
  cppgc::Persistent<PseudoCollectible> holdee_;
};

class Bloater : public cppgc::GarbageCollected<Bloater> {
 public:
  void Trace(cppgc::Visitor* visitor) const {}
  uint8_t bloat_[65536];
};

}  // namespace

class HeapUnitTest : public FXGCUnitTest {
 public:
  HeapUnitTest() = default;
  ~HeapUnitTest() override = default;

  // FXGCUnitTest:
  void TearDown() override {
    PseudoCollectible::ClearCounts();
    FXGCUnitTest::TearDown();
  }
};

TEST_F(HeapUnitTest, SeveralHeaps) {
  FXGCScopedHeap heap1 = FXGC_CreateHeap();
  EXPECT_TRUE(heap1);

  FXGCScopedHeap heap2 = FXGC_CreateHeap();
  EXPECT_TRUE(heap2);

  FXGCScopedHeap heap3 = FXGC_CreateHeap();
  EXPECT_TRUE(heap3);

  // Test manually destroying the heap.
  heap3.reset();
  EXPECT_FALSE(heap3);
  heap3.reset();
  EXPECT_FALSE(heap3);
}

TEST_F(HeapUnitTest, NoReferences) {
  FXGCScopedHeap heap1 = FXGC_CreateHeap();
  ASSERT_TRUE(heap1);
  {
    auto holder = std::make_unique<CollectibleHolder>(
        cppgc::MakeGarbageCollected<PseudoCollectible>(
            heap1->GetAllocationHandle()));

    EXPECT_TRUE(holder->holdee()->IsLive());
    EXPECT_EQ(1u, PseudoCollectible::LiveCount());
    EXPECT_EQ(0u, PseudoCollectible::DeadCount());
  }
  FXGC_ForceGarbageCollection(heap1.get());
  EXPECT_EQ(0u, PseudoCollectible::LiveCount());
  EXPECT_EQ(1u, PseudoCollectible::DeadCount());
}

TEST_F(HeapUnitTest, HasReferences) {
  FXGCScopedHeap heap1 = FXGC_CreateHeap();
  ASSERT_TRUE(heap1);
  {
    auto holder = std::make_unique<CollectibleHolder>(
        cppgc::MakeGarbageCollected<PseudoCollectible>(
            heap1->GetAllocationHandle()));

    EXPECT_TRUE(holder->holdee()->IsLive());
    EXPECT_EQ(1u, PseudoCollectible::LiveCount());
    EXPECT_EQ(0u, PseudoCollectible::DeadCount());

    FXGC_ForceGarbageCollection(heap1.get());
    EXPECT_TRUE(holder->holdee()->IsLive());
    EXPECT_EQ(1u, PseudoCollectible::LiveCount());
    EXPECT_EQ(0u, PseudoCollectible::DeadCount());
  }
}

// TODO(tsepez): enable when CPPGC fixes this segv.
TEST_F(HeapUnitTest, DISABLED_DeleteHeapHasReferences) {
  FXGCScopedHeap heap1 = FXGC_CreateHeap();
  ASSERT_TRUE(heap1);
  {
    auto holder = std::make_unique<CollectibleHolder>(
        cppgc::MakeGarbageCollected<PseudoCollectible>(
            heap1->GetAllocationHandle()));

    EXPECT_TRUE(holder->holdee()->IsLive());
    EXPECT_EQ(1u, PseudoCollectible::LiveCount());
    EXPECT_EQ(0u, PseudoCollectible::DeadCount());

    heap1.reset();

    // Maybe someday magically nulled by heap destruction.
    EXPECT_FALSE(holder->holdee());
    EXPECT_EQ(1u, PseudoCollectible::LiveCount());
    EXPECT_EQ(0u, PseudoCollectible::DeadCount());
  }
}

TEST_F(HeapUnitTest, DeleteHeapNoReferences) {
  FXGCScopedHeap heap1 = FXGC_CreateHeap();
  ASSERT_TRUE(heap1);
  {
    auto holder = std::make_unique<CollectibleHolder>(
        cppgc::MakeGarbageCollected<PseudoCollectible>(
            heap1->GetAllocationHandle()));

    EXPECT_TRUE(holder->holdee()->IsLive());
    EXPECT_EQ(1u, PseudoCollectible::LiveCount());
    EXPECT_EQ(0u, PseudoCollectible::DeadCount());
  }
  heap1.reset();
  EXPECT_EQ(0u, PseudoCollectible::LiveCount());
  EXPECT_EQ(1u, PseudoCollectible::DeadCount());
}

TEST_F(HeapUnitTest, Bloat) {
  ASSERT_TRUE(heap());
  for (int i = 0; i < 100000; ++i) {
    cppgc::MakeGarbageCollected<Bloater>(heap()->GetAllocationHandle());
    Pump();  // Do not force GC, must happen implicitly when space required.
  }
}
