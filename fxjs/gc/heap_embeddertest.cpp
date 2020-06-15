// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/gc/heap.h"

#include <memory>

#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST_F(EmbedderTest, SeveralHeaps) {
  FXGCScopedHeap heap1 = FXGC_CreateHeap();
  EXPECT_TRUE(heap1);

  FXGCScopedHeap heap2 = FXGC_CreateHeap();
  EXPECT_TRUE(heap2);

  FXGCScopedHeap heap3 = FXGC_CreateHeap();
  EXPECT_TRUE(heap2);
}
