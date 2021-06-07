// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/fxgc_unittest.h"

#include "fxjs/gc/heap.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/v8_test_environment.h"
#include "v8/include/libplatform/libplatform.h"

FXGCUnitTest::FXGCUnitTest() = default;

FXGCUnitTest::~FXGCUnitTest() = default;

void FXGCUnitTest::SetUp() {
  FXV8UnitTest::SetUp();
  auto* env = V8TestEnvironment::GetInstance();
  FXGC_Initialize(env->platform(), env->isolate());
  heap_ = FXGC_CreateHeap();
  ASSERT_TRUE(heap_);
}

void FXGCUnitTest::TearDown() {
  FXGC_ForceGarbageCollection(heap_.get());
  auto* env = V8TestEnvironment::GetInstance();
  while (v8::platform::PumpMessageLoop(env->platform(), env->isolate()))
    continue;

  heap_.reset();
  FXGC_Release();
  FXV8UnitTest::TearDown();
}
