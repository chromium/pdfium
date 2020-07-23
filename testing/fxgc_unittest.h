// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_FXGC_UNITTEST_H_
#define TESTING_FXGC_UNITTEST_H_

#include "fxjs/gc/heap.h"
#include "testing/fxv8_unittest.h"

class FXGCUnitTest : public FXV8UnitTest {
 public:
  FXGCUnitTest();
  ~FXGCUnitTest() override;

  // FXV8UnitTest:
  void SetUp() override;
  void TearDown() override;

  cppgc::Heap* heap() const { return heap_.get(); }

 private:
  FXGCScopedHeap heap_;
};

#endif  // TESTING_FXGC_UNITTEST_H_
