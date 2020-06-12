// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/gc/heap.h"

#include "third_party/base/ptr_util.h"

namespace {

size_t g_platform_ref_count = 0;
v8::Platform* g_platform = nullptr;

}  // namespace

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
    return g_platform->GetForegroundTaskRunner(nullptr);
  }

  std::unique_ptr<cppgc::JobHandle> PostJob(
      cppgc::TaskPriority priority,
      std::unique_ptr<cppgc::JobTask> job_task) override {
    return g_platform->PostJob(priority, std::move(job_task));
  }
};

void FXGC_Initialize(v8::Platform* platform) {
  if (platform) {
    g_platform = platform;
    cppgc::InitializeProcess(platform->GetPageAllocator());
  }
}

void FXGC_Release() {
  if (g_platform && g_platform_ref_count == 0) {
    cppgc::ShutdownProcess();
    g_platform = nullptr;
  }
}

std::unique_ptr<cppgc::Heap> FXGC_CreateHeap() {
  ++g_platform_ref_count;
  return cppgc::Heap::Create(std::make_shared<CFXGC_Platform>());
}

void FXGC_ReleaseHeap(std::unique_ptr<cppgc::Heap> heap) {
  --g_platform_ref_count;
  // |heap| destroyed when it goes out of scope.
}
