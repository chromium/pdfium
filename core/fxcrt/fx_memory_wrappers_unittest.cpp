// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_memory_wrappers.h"

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "build/build_config.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

struct OnlyNumbers {
  int x;
  int y;
};

}  // namespace

FX_DATA_PARTITION_EXCEPTION(OnlyNumbers);

TEST(fxcrt, FxFreeDeleter) {
  std::unique_ptr<int, FxFreeDeleter> empty(nullptr);
  std::unique_ptr<int, FxFreeDeleter> thing(FX_Alloc(int, 1));
  std::unique_ptr<int, FxFreeDeleter> several(FX_Alloc(int, 100));
  EXPECT_FALSE(empty);
  EXPECT_TRUE(thing);
  EXPECT_TRUE(several);
}

TEST(fxcrt, FxAllocAllocator) {
  // Let ASAN sanity check some simple operations.
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

TEST(fxcrt, FxStringAllocator) {
  // Let ASAN sanity check some simple operations.
  std::basic_ostringstream<char, std::char_traits<char>,
                           FxStringAllocator<char>>
      str;
  str << 'B';
  str << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  str << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  str << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
  str << 42.0f;
}

TEST(fxcrt, FxAllocAllocatorStructException) {
  std::vector<OnlyNumbers, FxAllocAllocator<OnlyNumbers>> vec;
  vec.push_back({42, 73});
  EXPECT_EQ(vec.back().x, 42);
  EXPECT_EQ(vec.back().y, 73);
}
