// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fixed_uninit_data_vector.h"

#include <numeric>
#include <utility>

#include "core/fxcrt/fixed_try_alloc_zeroed_data_vector.h"
#include "core/fxcrt/fixed_zeroed_data_vector.h"
#include "core/fxcrt/span_util.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/containers/span.h"

TEST(FixedUninitDataVector, NoData) {
  FixedUninitDataVector<int> vec;
  EXPECT_EQ(0u, vec.size());
  EXPECT_TRUE(vec.empty());
  EXPECT_TRUE(vec.span().empty());
}

TEST(FixedUninitDataVector, WithData) {
  FixedUninitDataVector<int> vec(4);
  EXPECT_FALSE(vec.empty());
  ASSERT_EQ(4u, vec.size());
  ASSERT_EQ(4u, vec.span().size());

  constexpr int kData[] = {1, 2, 3, 4};
  fxcrt::spancpy(vec.span(), pdfium::make_span(kData));
  EXPECT_THAT(vec.span(), testing::ElementsAre(1, 2, 3, 4));
}

TEST(FixedUninitDataVector, Move) {
  FixedUninitDataVector<int> vec(4);
  constexpr int kData[] = {1, 2, 3, 4};
  ASSERT_EQ(4u, vec.span().size());
  fxcrt::spancpy(vec.span(), pdfium::make_span(kData));
  const int* const original_data_ptr = vec.span().data();

  FixedUninitDataVector<int> vec2(std::move(vec));
  EXPECT_FALSE(vec2.empty());
  ASSERT_EQ(4u, vec2.size());
  ASSERT_EQ(4u, vec2.span().size());
  EXPECT_THAT(vec2.span(), testing::ElementsAre(1, 2, 3, 4));
  EXPECT_EQ(vec2.span().data(), original_data_ptr);

  EXPECT_EQ(0u, vec.size());
  EXPECT_TRUE(vec.empty());
  EXPECT_TRUE(vec.span().empty());

  vec = std::move(vec2);
  EXPECT_FALSE(vec.empty());
  ASSERT_EQ(4u, vec.size());
  ASSERT_EQ(4u, vec.span().size());
  EXPECT_THAT(vec.span(), testing::ElementsAre(1, 2, 3, 4));
  EXPECT_EQ(vec.span().data(), original_data_ptr);

  EXPECT_EQ(0u, vec2.size());
  EXPECT_TRUE(vec2.empty());
  EXPECT_TRUE(vec2.span().empty());
}

TEST(FixedUninitDataVector, AssignFromFixedZeroedDataVector) {
  FixedUninitDataVector<int> vec;

  FixedZeroedDataVector<int> vec2(4);
  constexpr int kData[] = {1, 2, 3, 4};
  ASSERT_EQ(4u, vec2.span().size());
  fxcrt::spancpy(vec2.span(), pdfium::make_span(kData));

  vec = std::move(vec2);
  EXPECT_TRUE(vec2.empty());
  ASSERT_EQ(4u, vec.span().size());
  EXPECT_THAT(vec.span(), testing::ElementsAre(1, 2, 3, 4));
}

TEST(FixedUninitDataVector, AssignFromFixedTryAllocZeroedDataVector) {
  FixedUninitDataVector<int> vec;

  FixedTryAllocZeroedDataVector<int> vec2(4);
  constexpr int kData[] = {1, 2, 3, 4};
  ASSERT_EQ(4u, vec2.span().size());
  fxcrt::spancpy(vec2.span(), pdfium::make_span(kData));

  vec = std::move(vec2);
  EXPECT_TRUE(vec2.empty());
  ASSERT_EQ(4u, vec.span().size());
  EXPECT_THAT(vec.span(), testing::ElementsAre(1, 2, 3, 4));
}

TEST(FixedUninitDataVector, Subspan) {
  FixedUninitDataVector<uint32_t> vec(4);
  std::iota(vec.span().begin(), vec.span().end(), 0u);

  pdfium::span<uint32_t> empty = vec.subspan(2, 0);
  EXPECT_TRUE(empty.empty());

  pdfium::span<uint32_t> first = vec.subspan(0, 1);
  ASSERT_EQ(first.size(), 1u);
  EXPECT_EQ(first[0], 0u);

  pdfium::span<uint32_t> mids = vec.subspan(1, 2);
  ASSERT_EQ(mids.size(), 2u);
  EXPECT_EQ(mids[0], 1u);
  EXPECT_EQ(mids[1], 2u);

  pdfium::span<uint32_t> rest = vec.subspan(3);
  ASSERT_EQ(rest.size(), 1u);
  EXPECT_EQ(rest[0], 3u);
}

TEST(FixedUninitDataVector, First) {
  FixedUninitDataVector<uint32_t> vec(4);
  std::iota(vec.span().begin(), vec.span().end(), 0u);

  pdfium::span<uint32_t> empty = vec.first(0);
  EXPECT_TRUE(empty.empty());

  pdfium::span<uint32_t> some = vec.first(2);
  ASSERT_EQ(some.size(), 2u);
  EXPECT_EQ(some[0], 0u);
  EXPECT_EQ(some[1], 1u);
}

TEST(FixedUninitDataVector, Last) {
  FixedUninitDataVector<uint32_t> vec(4);
  std::iota(vec.span().begin(), vec.span().end(), 0u);

  pdfium::span<uint32_t> empty = vec.first(0);
  EXPECT_TRUE(empty.empty());

  pdfium::span<uint32_t> some = vec.first(2);
  ASSERT_EQ(some.size(), 2u);
  EXPECT_EQ(some[0], 0u);
  EXPECT_EQ(some[1], 1u);
}
