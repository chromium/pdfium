// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FXJS_GC_FXGC_UNITTEST_H_
#define FXJS_GC_FXGC_UNITTEST_H_

#include "fxjs/fxv8_unittest.h"
#include "fxjs/gc/heap.h"

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

#endif  // FXJS_GC_FXGC_UNITTEST_H_
