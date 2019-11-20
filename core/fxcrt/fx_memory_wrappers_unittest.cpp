// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_memory_wrappers.h"

#include <memory>
#include <vector>

#include "build/build_config.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(fxcrt, FxFreeDeleter) {
  std::unique_ptr<int, FxFreeDeleter> empty(nullptr);
  std::unique_ptr<int, FxFreeDeleter> thing(FX_Alloc(int, 1));
  std::unique_ptr<int, FxFreeDeleter> several(FX_Alloc(int, 100));
  EXPECT_FALSE(empty);
  EXPECT_TRUE(thing);
  EXPECT_TRUE(several);
}

TEST(fxcrt, FxAllocAllocator) {
  std::vector<int, FxAllocAllocator<int>> vec;
  vec.push_back(42);
  vec.reserve(100);
  vec.resize(20);
  vec[11] = 42;

  std::vector<int, FxAllocAllocator<int>> vec2 = vec;
  vec = std::move(vec2);
  vec2.resize(0);
  vec2.push_back(42);
}
