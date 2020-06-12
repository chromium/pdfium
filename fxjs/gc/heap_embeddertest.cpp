// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/gc/heap.h"

#include <memory>

#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class HeapEmbedderTest : public EmbedderTest {};

TEST(HeapEmbedderTest, SeveralHeaps) {
  FXGC_Initialize(EmbedderTestEnvironment::GetInstance()->platform());

  std::unique_ptr<cppgc::Heap> heap1 = FXGC_CreateHeap();
  EXPECT_TRUE(heap1);

  std::unique_ptr<cppgc::Heap> heap2 = FXGC_CreateHeap();
  EXPECT_TRUE(heap2);

  std::unique_ptr<cppgc::Heap> heap3 = FXGC_CreateHeap();
  EXPECT_TRUE(heap2);

  FXGC_ReleaseHeap(std::move(heap1));
  FXGC_ReleaseHeap(std::move(heap2));
  FXGC_ReleaseHeap(std::move(heap3));

  FXGC_Release();
}
