// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/gc/heap.h"

#include <utility>

#include "core/fxcrt/fx_system.h"
#include "third_party/base/check.h"
#include "v8/include/cppgc/heap.h"

namespace {

size_t g_platform_ref_count = 0;
v8::Platform* g_platform = nullptr;
v8::Isolate* g_isolate = nullptr;

}  // namespace

// Taken from v8/samples/cppgc/cppgc-for-v8-embedders.cc.
// Adaptper that makes the global v8::Platform compatible with a
// cppgc::Platform.
class CFXGC_Platform final : public cppgc::Platform {
 public:
  CFXGC_Platform() = default;
  ~CFXGC_Platform() override = default;

  cppgc::PageAllocator* GetPageAllocator() override {
    return g_platform->GetPageAllocator();
  }

  double MonotonicallyIncreasingTime() override {
    return g_platform->MonotonicallyIncreasingTime();
  }

  std::shared_ptr<cppgc::TaskRunner> GetForegroundTaskRunner() override {
    // V8's default platform creates a new task runner when passed the
    // v8::Isolate pointer the first time. For non-default platforms this will
    // require getting the appropriate task runner.
    return g_platform->GetForegroundTaskRunner(g_isolate);
  }

  std::unique_ptr<cppgc::JobHandle> PostJob(
      cppgc::TaskPriority priority,
      std::unique_ptr<cppgc::JobTask> job_task) override {
    return g_platform->PostJob(priority, std::move(job_task));
  }
};

void FXGC_Initialize(v8::Platform* platform, v8::Isolate* isolate) {
  if (platform) {
    DCHECK(!g_platform);
    g_platform = platform;
    g_isolate = isolate;
  }
}

void FXGC_Release() {
  if (g_platform && g_platform_ref_count == 0) {
    g_platform = nullptr;
    g_isolate = nullptr;
  }
}

FXGCScopedHeap FXGC_CreateHeap() {
  // If XFA is included at compile-time, but JS is disabled at run-time,
  // we may still attempt to build a CPDFXFA_Context which will want a
  // heap. But we can't make one because JS is disabled.
  // TODO(tsepez): Stop the context from even being created.
  if (!g_platform)
    return nullptr;

  ++g_platform_ref_count;
  auto heap = cppgc::Heap::Create(
      std::make_shared<CFXGC_Platform>(),
      cppgc::Heap::HeapOptions{
          {},
          cppgc::Heap::StackSupport::kNoConservativeStackScan,
          cppgc::Heap::MarkingType::kAtomic,
          cppgc::Heap::SweepingType::kIncrementalAndConcurrent,
          {}});
  return FXGCScopedHeap(heap.release());
}

void FXGC_ForceGarbageCollection(cppgc::Heap* heap) {
  heap->ForceGarbageCollectionSlow("FXGC", "ForceGarbageCollection",
                                   cppgc::Heap::StackState::kNoHeapPointers);
}

void FXGCHeapDeleter::operator()(cppgc::Heap* heap) {
  DCHECK(heap);
  DCHECK(g_platform_ref_count > 0);
  --g_platform_ref_count;

  FXGC_ForceGarbageCollection(heap);
  delete heap;
}
