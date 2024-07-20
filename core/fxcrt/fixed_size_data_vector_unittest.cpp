// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fixed_size_data_vector.h"

#include <limits>
#include <numeric>
#include <utility>

#include "core/fxcrt/span.h"
#include "core/fxcrt/stl_util.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(FixedSizedDataVector, NoData) {
  FixedSizeDataVector<int> vec;
  EXPECT_EQ(0u, vec.size());
  EXPECT_TRUE(vec.empty());
  EXPECT_TRUE(vec.span().empty());
}

TEST(FixedSizeDataVector, UninitData) {
  auto vec = FixedSizeDataVector<int>::Uninit(4);
  EXPECT_FALSE(vec.empty());
  ASSERT_EQ(4u, vec.size());
  ASSERT_EQ(4u, vec.span().size());

  constexpr int kData[] = {1, 2, 3, 4};
  fxcrt::Copy(kData, vec.span());
  EXPECT_THAT(vec.span(), testing::ElementsAre(1, 2, 3, 4));
}

TEST(FixedSizeDataVector, TryUninitData) {
  auto vec = FixedSizeDataVector<int>::TryUninit(4);
  EXPECT_FALSE(vec.empty());
  ASSERT_EQ(4u, vec.size());
  ASSERT_EQ(4u, vec.span().size());

  constexpr int kData[] = {1, 2, 3, 4};
  fxcrt::Copy(kData, vec.span());
  EXPECT_THAT(vec.span(), testing::ElementsAre(1, 2, 3, 4));
}

TEST(FixedSizeDataVector, ZeroedData) {
  auto vec = FixedSizeDataVector<int>::Zeroed(4);
  EXPECT_FALSE(vec.empty());
  EXPECT_EQ(4u, vec.size());
  EXPECT_EQ(4u, vec.span().size());
  EXPECT_THAT(vec.span(), testing::ElementsAre(0, 0, 0, 0));

  constexpr int kData[] = {1, 2, 3, 4};
  fxcrt::Copy(kData, vec.span());
  EXPECT_THAT(vec.span(), testing::ElementsAre(1, 2, 3, 4));
}

TEST(FixedSizeDataVector, TryZeroedData) {
  auto vec = FixedSizeDataVector<int>::TryZeroed(4);
  EXPECT_FALSE(vec.empty());
  ASSERT_EQ(4u, vec.size());
  ASSERT_EQ(4u, vec.span().size());
  EXPECT_THAT(vec.span(), testing::ElementsAre(0, 0, 0, 0));

  constexpr int kData[] = {1, 2, 3, 4};
  fxcrt::Copy(kData, vec.span());
  EXPECT_THAT(vec.span(), testing::ElementsAre(1, 2, 3, 4));
}

TEST(FixedSizeDataVector, TryAllocFailures) {
  constexpr size_t kCloseToMaxByteAlloc =
      std::numeric_limits<size_t>::max() - 100;
  auto vec = FixedSizeDataVector<int>::TryZeroed(kCloseToMaxByteAlloc);
  EXPECT_TRUE(vec.empty());
  EXPECT_EQ(0u, vec.size());
  EXPECT_EQ(0u, vec.span().size());

  vec = FixedSizeDataVector<int>::TryUninit(kCloseToMaxByteAlloc);
  EXPECT_TRUE(vec.empty());
  EXPECT_EQ(0u, vec.size());
  EXPECT_EQ(0u, vec.span().size());
}

TEST(FixedSizeDataVector, MoveConstruct) {
  constexpr int kData[] = {1, 2, 3, 4};
  auto vec = FixedSizeDataVector<int>::Uninit(4);
  ASSERT_EQ(4u, vec.span().size());
  fxcrt::Copy(kData, vec.span());
  const int* const original_data_ptr = vec.span().data();

  FixedSizeDataVector<int> vec2(std::move(vec));
  EXPECT_FALSE(vec2.empty());
  EXPECT_EQ(4u, vec2.size());
  EXPECT_EQ(4u, vec2.span().size());
  EXPECT_THAT(vec2.span(), testing::ElementsAre(1, 2, 3, 4));
  EXPECT_EQ(vec2.span().data(), original_data_ptr);

  EXPECT_EQ(0u, vec.size());
  EXPECT_TRUE(vec.empty());
  EXPECT_TRUE(vec.span().empty());

  vec = std::move(vec2);
  EXPECT_FALSE(vec.empty());
  EXPECT_EQ(4u, vec.size());
  EXPECT_EQ(4u, vec.span().size());
  EXPECT_THAT(vec.span(), testing::ElementsAre(1, 2, 3, 4));
  EXPECT_EQ(vec.span().data(), original_data_ptr);

  EXPECT_EQ(0u, vec2.size());
  EXPECT_TRUE(vec2.empty());
  EXPECT_TRUE(vec2.span().empty());
}

TEST(FixedSizeDataVector, MoveAssign) {
  auto vec = FixedSizeDataVector<int>();
  auto vec2 = FixedSizeDataVector<int>::Zeroed(4);
  constexpr int kData[] = {1, 2, 3, 4};
  ASSERT_EQ(4u, vec2.span().size());
  fxcrt::Copy(kData, vec2.span());

  vec = std::move(vec2);
  EXPECT_TRUE(vec2.empty());
  EXPECT_EQ(4u, vec.span().size());
  EXPECT_THAT(vec.span(), testing::ElementsAre(1, 2, 3, 4));
}

TEST(FixedSizeDataVector, TruncatedFrom) {
  constexpr int kData[] = {1, 2, 3, 4};
  auto vec1 = FixedSizeDataVector<int>::Uninit(4);
  fxcrt::Copy(kData, vec1.span());

  auto vec2 = FixedSizeDataVector<int>::TruncatedFrom(std::move(vec1), 3);
  EXPECT_EQ(0u, vec1.span().size());
  EXPECT_EQ(nullptr, vec1.span().data());
  EXPECT_THAT(vec2.span(), testing::ElementsAre(1, 2, 3));
}

TEST(FixedSizeDataVector, Subspan) {
  auto vec = FixedSizeDataVector<uint32_t>::Uninit(4);
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

TEST(FixedSizeDataVector, First) {
  auto vec = FixedSizeDataVector<uint32_t>::Uninit(4);
  std::iota(vec.span().begin(), vec.span().end(), 0u);

  pdfium::span<uint32_t> empty = vec.first(0);
  EXPECT_TRUE(empty.empty());

  pdfium::span<uint32_t> some = vec.first(2);
  ASSERT_EQ(some.size(), 2u);
  EXPECT_EQ(some[0], 0u);
  EXPECT_EQ(some[1], 1u);
}

TEST(FixedSizeDataVector, Last) {
  auto vec = FixedSizeDataVector<uint32_t>::Uninit(4);
  std::iota(vec.span().begin(), vec.span().end(), 0u);

  pdfium::span<uint32_t> empty = vec.first(0);
  EXPECT_TRUE(empty.empty());

  pdfium::span<uint32_t> some = vec.first(2);
  ASSERT_EQ(some.size(), 2u);
  EXPECT_EQ(some[0], 0u);
  EXPECT_EQ(some[1], 1u);
}
