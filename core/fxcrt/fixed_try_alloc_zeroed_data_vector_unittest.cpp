// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fixed_try_alloc_zeroed_data_vector.h"

#include <limits>
#include <utility>

#include "core/fxcrt/fixed_uninit_data_vector.h"
#include "core/fxcrt/fixed_zeroed_data_vector.h"
#include "core/fxcrt/span_util.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/containers/span.h"

TEST(FixedTryAllocZeroedDataVector, NoData) {
  FixedTryAllocZeroedDataVector<int> vec;
  EXPECT_EQ(0u, vec.size());
  EXPECT_TRUE(vec.empty());
  EXPECT_TRUE(vec.span().empty());
  EXPECT_TRUE(vec.writable_span().empty());
}

TEST(FixedTryAllocZeroedDataVector, WithData) {
  FixedTryAllocZeroedDataVector<int> vec(4);
  EXPECT_FALSE(vec.empty());
  EXPECT_EQ(4u, vec.size());
  EXPECT_EQ(4u, vec.span().size());
  EXPECT_EQ(4u, vec.writable_span().size());
  EXPECT_THAT(vec.span(), testing::ElementsAre(0, 0, 0, 0));

  constexpr int kData[] = {1, 2, 3, 4};
  fxcrt::spancpy(vec.writable_span(), pdfium::make_span(kData));
  EXPECT_THAT(vec.span(), testing::ElementsAre(1, 2, 3, 4));
}

TEST(FixedTryAllocZeroedDataVector, AllocFailure) {
  constexpr size_t kCloseToMaxByteAlloc =
      std::numeric_limits<size_t>::max() - 100;
  FixedTryAllocZeroedDataVector<int> vec(kCloseToMaxByteAlloc);
  EXPECT_TRUE(vec.empty());
  EXPECT_EQ(0u, vec.size());
  EXPECT_EQ(0u, vec.span().size());
  EXPECT_EQ(0u, vec.writable_span().size());
}

TEST(FixedTryAllocZeroedDataVector, Move) {
  FixedTryAllocZeroedDataVector<int> vec(4);
  constexpr int kData[] = {1, 2, 3, 4};
  ASSERT_EQ(4u, vec.writable_span().size());
  fxcrt::spancpy(vec.writable_span(), pdfium::make_span(kData));
  const int* const original_data_ptr = vec.span().data();

  FixedTryAllocZeroedDataVector<int> vec2(std::move(vec));
  EXPECT_FALSE(vec2.empty());
  EXPECT_EQ(4u, vec2.size());
  EXPECT_EQ(4u, vec2.span().size());
  EXPECT_EQ(4u, vec2.writable_span().size());
  EXPECT_THAT(vec2.span(), testing::ElementsAre(1, 2, 3, 4));
  EXPECT_EQ(vec2.span().data(), original_data_ptr);

  EXPECT_EQ(0u, vec.size());
  EXPECT_TRUE(vec.empty());
  EXPECT_TRUE(vec.span().empty());
  EXPECT_TRUE(vec.writable_span().empty());

  vec = std::move(vec2);
  EXPECT_FALSE(vec.empty());
  EXPECT_EQ(4u, vec.size());
  EXPECT_EQ(4u, vec.span().size());
  EXPECT_EQ(4u, vec.writable_span().size());
  EXPECT_THAT(vec.span(), testing::ElementsAre(1, 2, 3, 4));
  EXPECT_EQ(vec.span().data(), original_data_ptr);

  EXPECT_EQ(0u, vec2.size());
  EXPECT_TRUE(vec2.empty());
  EXPECT_TRUE(vec2.span().empty());
  EXPECT_TRUE(vec2.writable_span().empty());
}

TEST(FixedTryAllocZeroedDataVector, AssignFromFixedZeroedDataVector) {
  FixedTryAllocZeroedDataVector<int> vec;

  FixedZeroedDataVector<int> vec2(4);
  constexpr int kData[] = {1, 2, 3, 4};
  ASSERT_EQ(4u, vec2.writable_span().size());
  fxcrt::spancpy(vec2.writable_span(), pdfium::make_span(kData));

  vec = std::move(vec2);
  EXPECT_TRUE(vec2.empty());
  EXPECT_EQ(4u, vec.span().size());
  EXPECT_THAT(vec.span(), testing::ElementsAre(1, 2, 3, 4));
}

TEST(FixedTryAllocZeroedDataVector, AssignFromFixedUninitDataVector) {
  FixedTryAllocZeroedDataVector<int> vec;

  FixedUninitDataVector<int> vec2(4);
  constexpr int kData[] = {1, 2, 3, 4};
  ASSERT_EQ(4u, vec2.writable_span().size());
  fxcrt::spancpy(vec2.writable_span(), pdfium::make_span(kData));

  vec = std::move(vec2);
  EXPECT_TRUE(vec2.empty());
  EXPECT_EQ(4u, vec.span().size());
  EXPECT_THAT(vec.span(), testing::ElementsAre(1, 2, 3, 4));
}
