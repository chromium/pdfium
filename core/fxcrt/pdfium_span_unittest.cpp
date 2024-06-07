// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "core/fxcrt/raw_span.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/unowned_ptr.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;

// Tests PDFium-modifications to base::span. The name of this file is
// chosen to avoid collisions with base's span_unittest.cc

TEST(PdfiumSpan, EmptySpan) {
  int stuff[] = {1, 2, 3};
  pdfium::span<int> null_span;
  pdfium::span<int> stuff_span(stuff);
  pdfium::span<int> empty_first_span = stuff_span.first(0);
  pdfium::span<int> empty_last_span = stuff_span.last(0);
  pdfium::span<int> empty_sub_span1 = stuff_span.subspan(0, 0);
  pdfium::span<int> empty_sub_span2 = stuff_span.subspan(3, 0);
  EXPECT_TRUE(null_span.empty());
  EXPECT_TRUE(empty_first_span.empty());
  EXPECT_TRUE(empty_last_span.empty());
  EXPECT_TRUE(empty_sub_span1.empty());
  EXPECT_TRUE(empty_sub_span2.empty());
}

// Custom implementation of first()/last().
TEST(PdfiumSpan, FirstLast) {
  int one[] = {1};
  int stuff[] = {1, 2, 3};
  pdfium::span<int> one_span(one);
  pdfium::span<int> stuff_span(stuff);
  EXPECT_EQ(one_span.front(), 1);
  EXPECT_EQ(one_span.back(), 1);
  EXPECT_EQ(stuff_span.front(), 1);
  EXPECT_EQ(stuff_span.back(), 3);
}

TEST(PdfiumSpan, GMockMacroCompatibility) {
  int arr1[] = {1, 3, 5};
  int arr2[] = {1, 3, 5};
  std::vector vec1(std::begin(arr1), std::end(arr1));
  std::vector vec2(std::begin(arr2), std::end(arr2));
  pdfium::span<int, 3> static_span1(arr1);
  pdfium::span<int, 3> static_span2(arr2);
  pdfium::span<int> dynamic_span1(vec1);
  pdfium::span<int> dynamic_span2(vec2);

  EXPECT_THAT(arr1, ElementsAreArray(static_span2));
  EXPECT_THAT(arr1, ElementsAreArray(dynamic_span2));

  EXPECT_THAT(vec1, ElementsAreArray(static_span2));
  EXPECT_THAT(vec1, ElementsAreArray(dynamic_span2));

  EXPECT_THAT(static_span1, ElementsAre(1, 3, 5));
  EXPECT_THAT(static_span1, ElementsAreArray(arr2));
  EXPECT_THAT(static_span1, ElementsAreArray(static_span2));
  EXPECT_THAT(static_span1, ElementsAreArray(dynamic_span2));
  EXPECT_THAT(static_span1, ElementsAreArray(vec2));

  EXPECT_THAT(dynamic_span1, ElementsAre(1, 3, 5));
  EXPECT_THAT(dynamic_span1, ElementsAreArray(arr2));
  EXPECT_THAT(dynamic_span1, ElementsAreArray(static_span2));
  EXPECT_THAT(dynamic_span1, ElementsAreArray(dynamic_span2));
  EXPECT_THAT(dynamic_span1, ElementsAreArray(vec2));
}

TEST(PdfiumSpanDeathTest, EmptySpanIndex) {
  pdfium::span<int> empty_span;
  EXPECT_DEATH(empty_span[0] += 1, "");
}

TEST(PdfiumSpanDeathTest, EmptySpanFront) {
  pdfium::span<int> empty_span;
  EXPECT_DEATH(empty_span.front() += 1, "");
}

TEST(PdfiumSpanDeathTest, EmptySpanBack) {
  pdfium::span<int> empty_span;
  EXPECT_DEATH(empty_span.back() += 1, "");
}

#if defined(UNOWNED_PTR_DANGLING_CHECKS)
namespace {

void CreateDanglingSpan() {
  pdfium::raw_span<int> data_span;
  {
    std::vector<int> data(4);
    data_span = pdfium::make_span(data);
  }
}

}  // namespace

TEST(PdfiumSpanDeathTest, DanglingReference) {
  EXPECT_DEATH(CreateDanglingSpan(), "");
}
#endif  // defined(UNOWNED_PTR_DANGLING_CHECKS)
