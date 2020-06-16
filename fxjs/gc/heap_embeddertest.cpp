// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/gc/heap.h"

#include <memory>
#include <set>

#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/stl_util.h"
#include "v8/include/cppgc/allocation.h"
#include "v8/include/cppgc/persistent.h"
#include "v8/include/v8.h"

namespace {

class PseudoCollectible : public cppgc::GarbageCollected<PseudoCollectible> {
 public:
  static cppgc::Persistent<PseudoCollectible> s_persistent_;

  static void Clear() {
    s_live_.clear();
    s_dead_.clear();
    s_persistent_ = nullptr;
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
cppgc::Persistent<PseudoCollectible> PseudoCollectible::s_persistent_;

struct V8IsolateDeleter {
  inline void operator()(v8::Isolate* ptr) { ptr->Dispose(); }
};

}  // namespace

class HeapEmbedderTest : public EmbedderTest {
 public:
  void SetUp() override {
    v8::Isolate::CreateParams params;
    params.array_buffer_allocator = static_cast<v8::ArrayBuffer::Allocator*>(
        FPDF_GetArrayBufferAllocatorSharedInstance());
    isolate_.reset(v8::Isolate::New(params));
    EmbedderTest::SetExternalIsolate(isolate_.get());
    EmbedderTest::SetUp();
  }

  void TearDown() override {
    EmbedderTest::TearDown();
    isolate_.reset();
  }

  void PumpPlatformMessageLoop() {
    while (v8::platform::PumpMessageLoop(
        EmbedderTestEnvironment::GetInstance()->platform(), isolate_.get())) {
      continue;
    }
  }

 private:
  std::unique_ptr<v8::Isolate, V8IsolateDeleter> isolate_;
};

TEST_F(HeapEmbedderTest, SeveralHeaps) {
  FXGCScopedHeap heap1 = FXGC_CreateHeap();
  EXPECT_TRUE(heap1);

  FXGCScopedHeap heap2 = FXGC_CreateHeap();
  EXPECT_TRUE(heap2);

  FXGCScopedHeap heap3 = FXGC_CreateHeap();
  EXPECT_TRUE(heap2);
}

TEST_F(HeapEmbedderTest, NoReferences) {
  FXGCScopedHeap heap1 = FXGC_CreateHeap();
  ASSERT_TRUE(heap1);

  PseudoCollectible::s_persistent_ =
      cppgc::MakeGarbageCollected<PseudoCollectible>(heap1.get());
  EXPECT_TRUE(PseudoCollectible::s_persistent_->IsLive());
  EXPECT_EQ(1u, PseudoCollectible::LiveCount());
  EXPECT_EQ(0u, PseudoCollectible::DeadCount());

  PseudoCollectible::s_persistent_ = nullptr;
  heap1->ForceGarbageCollectionSlow("NoReferences", "test",
                                    cppgc::Heap::StackState::kNoHeapPointers);
  PumpPlatformMessageLoop();
  EXPECT_EQ(0u, PseudoCollectible::LiveCount());
  EXPECT_EQ(1u, PseudoCollectible::DeadCount());
  PseudoCollectible::Clear();
}

TEST_F(HeapEmbedderTest, HasReferences) {
  FXGCScopedHeap heap1 = FXGC_CreateHeap();
  ASSERT_TRUE(heap1);

  PseudoCollectible::s_persistent_ =
      cppgc::MakeGarbageCollected<PseudoCollectible>(heap1.get());
  EXPECT_TRUE(PseudoCollectible::s_persistent_->IsLive());
  EXPECT_EQ(1u, PseudoCollectible::LiveCount());
  EXPECT_EQ(0u, PseudoCollectible::DeadCount());

  heap1->ForceGarbageCollectionSlow("HasReferences", "test",
                                    cppgc::Heap::StackState::kNoHeapPointers);
  PumpPlatformMessageLoop();
  EXPECT_TRUE(PseudoCollectible::s_persistent_->IsLive());
  EXPECT_EQ(1u, PseudoCollectible::LiveCount());
  EXPECT_EQ(0u, PseudoCollectible::DeadCount());
  PseudoCollectible::Clear();
}
