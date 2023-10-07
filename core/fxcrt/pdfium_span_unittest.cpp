// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "core/fxcrt/unowned_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/containers/span.h"

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

TEST(PdfiumSpanDeathTest, EmptySpanIndex) {
  pdfium::span<int> empty_span;
  EXPECT_DEATH(empty_span[0] += 1, ".*");
}

TEST(PdfiumSpanDeathTest, EmptySpanFront) {
  pdfium::span<int> empty_span;
  EXPECT_DEATH(empty_span.front() += 1, ".*");
}

TEST(PdfiumSpanDeathTest, EmptySpanBack) {
  pdfium::span<int> empty_span;
  EXPECT_DEATH(empty_span.back() += 1, ".*");
}

#if defined(UNOWNED_PTR_DANGLING_CHECKS)
namespace {

void CreateDanglingSpan() {
  pdfium::span<int> data_span;
  {
    std::vector<int> data(4);
    data_span = pdfium::make_span(data);
  }
}

}  // namespace

TEST(PdfiumSpanDeathTest, DanglingReference) {
  EXPECT_DEATH(CreateDanglingSpan(), ".*");
}
#endif  // defined(UNOWNED_PTR_DANGLING_CHECKS)
