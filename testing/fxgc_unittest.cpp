// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/fxgc_unittest.h"

#include "fxjs/gc/heap.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/v8_test_environment.h"

FXGCUnitTest::FXGCUnitTest() = default;

FXGCUnitTest::~FXGCUnitTest() = default;

void FXGCUnitTest::SetUp() {
  FXV8UnitTest::SetUp();
  FXGC_Initialize(V8TestEnvironment::GetInstance()->platform());
  heap_ = FXGC_CreateHeap();
  ASSERT_TRUE(heap_);
}

void FXGCUnitTest::TearDown() {
  heap_.reset();
  FXGC_Release();
  FXV8UnitTest::TearDown();
}
