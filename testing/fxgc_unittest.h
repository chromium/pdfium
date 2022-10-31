// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_FXGC_UNITTEST_H_
#define TESTING_FXGC_UNITTEST_H_

#include "fxjs/gc/heap.h"
#include "testing/gtest/include/gtest/gtest.h"

class FXGCUnitTest : public ::testing::Test {
 public:
  FXGCUnitTest();
  ~FXGCUnitTest() override;

  // testing::Test:
  void SetUp() override;
  void TearDown() override;

  cppgc::Heap* heap() const { return heap_.get(); }
  void ForceGCAndPump();
  void Pump();

 private:
  FXGCScopedHeap heap_;
};

#endif  // TESTING_FXGC_UNITTEST_H_
