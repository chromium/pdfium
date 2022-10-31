// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/small_buffer.h"

#include <algorithm>

#include "testing/gtest/include/gtest/gtest.h"

namespace fxcrt {

TEST(SmallBuffer, Empty) {
  SmallBuffer<int, 4> buffer(0);
  EXPECT_EQ(buffer.data(), buffer.fixed_for_test());
  EXPECT_EQ(buffer.begin(), buffer.end());
}

TEST(SmallBuffer, NoFixed) {
  SmallBuffer<int, 0> buffer(4);
  EXPECT_EQ(buffer.data(), buffer.dynamic_for_test());
  std::fill(buffer.begin(), buffer.end(), 42);
  int* ptr = buffer.data();
  EXPECT_EQ(42, ptr[0]);
  EXPECT_EQ(42, ptr[1]);
  EXPECT_EQ(42, ptr[2]);
  EXPECT_EQ(42, ptr[3]);
}

TEST(SmallBuffer, NoFixedEmpty) {
  SmallBuffer<int, 0> buffer(0);
  EXPECT_EQ(buffer.data(), buffer.fixed_for_test());
  EXPECT_EQ(buffer.begin(), buffer.end());
}

TEST(SmallBuffer, Fixed) {
  SmallBuffer<int, 4> buffer(2);
  EXPECT_EQ(buffer.data(), buffer.fixed_for_test());
  std::fill(buffer.begin(), buffer.end(), 42);
  int* ptr = buffer.data();
  EXPECT_EQ(42, ptr[0]);
  EXPECT_EQ(42, ptr[1]);
}

TEST(SmallBuffer, Dynamic) {
  SmallBuffer<int, 2> buffer(4);
  EXPECT_EQ(buffer.data(), buffer.dynamic_for_test());
  std::fill(buffer.begin(), buffer.end(), 42);
  int* ptr = buffer.data();
  EXPECT_EQ(42, ptr[0]);
  EXPECT_EQ(42, ptr[1]);
  EXPECT_EQ(42, ptr[2]);
  EXPECT_EQ(42, ptr[3]);
}

}  // namespace fxcrt
