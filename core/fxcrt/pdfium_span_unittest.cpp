// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <vector>

#include "core/fxcrt/raw_span.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/unowned_ptr.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::AnyOf;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;

// Tests PDFium-modifications to base::span. The name of this file is
// chosen to avoid collisions with base's span_unittest.cc

TEST(PdfiumSpan, EmptySpan) {
  int stuff[] = {1, 2, 3};
  pdfium::span<int> null_span;
  pdfium::span<int> stuff_span(stuff);
  EXPECT_TRUE(null_span.empty());
  EXPECT_FALSE(stuff_span.empty());
  {
    pdfium::span<int> empty_first_span = stuff_span.first(0u);
    pdfium::span<int> empty_last_span = stuff_span.last(0u);
    pdfium::span<int> empty_sub_span1 = stuff_span.subspan(0u, 0u);
    pdfium::span<int> empty_sub_span2 = stuff_span.subspan(3u, 0u);
    pdfium::span<int> empty_sub_span3 = stuff_span.subspan(3u);
    EXPECT_TRUE(empty_first_span.empty());
    EXPECT_TRUE(empty_last_span.empty());
    EXPECT_TRUE(empty_sub_span1.empty());
    EXPECT_TRUE(empty_sub_span2.empty());
    EXPECT_TRUE(empty_sub_span3.empty());
  }
  {
    pdfium::span<int> empty_first_span = stuff_span.first<0>();
    pdfium::span<int> empty_last_span = stuff_span.last<0>();
    pdfium::span<int> empty_sub_span1 = stuff_span.subspan<0, 0>();
    pdfium::span<int> empty_sub_span2 = stuff_span.subspan<3, 0>();
    pdfium::span<int> empty_sub_span3 = stuff_span.subspan<3>();
    EXPECT_TRUE(empty_first_span.empty());
    EXPECT_TRUE(empty_last_span.empty());
    EXPECT_TRUE(empty_sub_span1.empty());
    EXPECT_TRUE(empty_sub_span2.empty());
    EXPECT_TRUE(empty_sub_span3.empty());
  }
}

TEST(PdfiumSpan, ValidSpan) {
  int stuff[] = {1, 2, 3};
  pdfium::span<int> stuff_span(stuff);
  EXPECT_FALSE(stuff_span.empty());
  {
    pdfium::span<int> first_span = stuff_span.first(2u);
    pdfium::span<int> last_span = stuff_span.last(2u);
    pdfium::span<int> sub_span1 = stuff_span.subspan(1u, 1u);
    pdfium::span<int> sub_span2 = stuff_span.subspan(2u, 1u);
    pdfium::span<int> sub_span3 = stuff_span.subspan(1u);
    EXPECT_THAT(first_span, ElementsAre(1, 2));
    EXPECT_THAT(last_span, ElementsAre(2, 3));
    EXPECT_THAT(sub_span1, ElementsAre(2));
    EXPECT_THAT(sub_span2, ElementsAre(3));
    EXPECT_THAT(sub_span3, ElementsAre(2, 3));
  }
  {
    pdfium::span<int> first_span = stuff_span.first<2>();
    pdfium::span<int> last_span = stuff_span.last<2>();
    pdfium::span<int> sub_span1 = stuff_span.subspan<1, 1>();
    pdfium::span<int> sub_span2 = stuff_span.subspan<2, 1>();
    pdfium::span<int> sub_span3 = stuff_span.subspan<1>();
    EXPECT_THAT(first_span, ElementsAre(1, 2));
    EXPECT_THAT(last_span, ElementsAre(2, 3));
    EXPECT_THAT(sub_span1, ElementsAre(2));
    EXPECT_THAT(sub_span2, ElementsAre(3));
    EXPECT_THAT(sub_span3, ElementsAre(2, 3));
  }
}

// Custom implementation of front()/back().
TEST(PdfiumSpan, FrontBack) {
  int one[] = {1};
  int stuff[] = {1, 2, 3};
  pdfium::span<int> one_span(one);
  pdfium::span<int> stuff_span(stuff);
  EXPECT_EQ(one_span.front(), 1);
  EXPECT_EQ(one_span.back(), 1);
  EXPECT_EQ(stuff_span.front(), 1);
  EXPECT_EQ(stuff_span.back(), 3);
}

TEST(PdfiumSpan, AsByteSpan) {
  int32_t c_style[] = {1, 2};
  const int32_t const_c_style[] = {3, 4};
  std::array<int32_t, 2> cxx_style = {1, 2};
  std::array<const int32_t, 2> const_cxx_style = {3, 4};

  // Don't assume a specific endian architecture.
  EXPECT_THAT(pdfium::as_byte_span(c_style),
              AnyOf(ElementsAre(0, 0, 0, 1, 0, 0, 0, 2),
                    ElementsAre(1, 0, 0, 0, 2, 0, 0, 0)));
  EXPECT_THAT(pdfium::as_writable_byte_span(c_style),
              AnyOf(ElementsAre(0, 0, 0, 1, 0, 0, 0, 2),
                    ElementsAre(1, 0, 0, 0, 2, 0, 0, 0)));
  EXPECT_THAT(pdfium::as_byte_span(cxx_style),
              AnyOf(ElementsAre(0, 0, 0, 1, 0, 0, 0, 2),
                    ElementsAre(1, 0, 0, 0, 2, 0, 0, 0)));
  EXPECT_THAT(pdfium::as_writable_byte_span(cxx_style),
              AnyOf(ElementsAre(0, 0, 0, 1, 0, 0, 0, 2),
                    ElementsAre(1, 0, 0, 0, 2, 0, 0, 0)));
  EXPECT_THAT(pdfium::as_byte_span(const_c_style),
              AnyOf(ElementsAre(0, 0, 0, 3, 0, 0, 0, 4),
                    ElementsAre(3, 0, 0, 0, 4, 0, 0, 0)));
  EXPECT_THAT(pdfium::as_byte_span(const_cxx_style),
              AnyOf(ElementsAre(0, 0, 0, 3, 0, 0, 0, 4),
                    ElementsAre(3, 0, 0, 0, 4, 0, 0, 0)));
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
