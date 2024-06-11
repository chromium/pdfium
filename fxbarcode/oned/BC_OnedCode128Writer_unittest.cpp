// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxbarcode/oned/BC_OnedCode128Writer.h"

#include <iterator>

#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/span.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::ElementsAreArray;

namespace {

struct TestCase {
  pdfium::span<const int32_t> pattern_span() const {
    return UNSAFE_TODO(pdfium::make_span(patterns, num_patterns));
  }

  const char* input;
  int32_t checksum;
  int32_t patterns[7];
  size_t num_patterns;
};

TEST(OnedCode128WriterTest, Encode128B) {
  static const TestCase kTestCases[] = {
      {"", 104, {104}, 1},
      {"a", 169, {104, 65}, 2},
      {"1", 121, {104, 17}, 2},
      {"a1", 203, {104, 65, 17}, 3},
      {"ab", 301, {104, 65, 66}, 3},
      {"12", 157, {104, 17, 18}, 3},
      {"abc", 502, {104, 65, 66, 67}, 4},
      {"123", 214, {104, 17, 18, 19}, 4},
      {"abc123", 774, {104, 65, 66, 67, 17, 18, 19}, 7},
      {"ABC123", 582, {104, 33, 34, 35, 17, 18, 19}, 7},
      {"321ABC", 722, {104, 19, 18, 17, 33, 34, 35}, 7},
      {"XYZ", 448, {104, 56, 57, 58}, 4},
  };
  for (const auto& test_case : kTestCases) {
    std::vector<int32_t> patterns;
    int32_t checksum =
        CBC_OnedCode128Writer::Encode128B(test_case.input, &patterns);
    EXPECT_EQ(test_case.checksum, checksum);
    EXPECT_THAT(patterns, ElementsAreArray(test_case.pattern_span()));
  }
}

TEST(OnedCode128WriterTest, Encode128C) {
  static const TestCase kTestCases[] = {
      {"", 105, {105}, 1},
      {"a", 202, {105, 97}, 2},
      {"1", 106, {105, 1}, 2},
      {"a1", 204, {105, 97, 1}, 3},
      {"ab", 398, {105, 97, 98}, 3},
      {"12", 117, {105, 12}, 2},
      {"abc", 695, {105, 97, 98, 99}, 4},
      {"123", 123, {105, 12, 3}, 3},
      {"abc123", 758, {105, 97, 98, 99, 12, 3}, 6},
      {"ABC123", 566, {105, 65, 66, 67, 12, 3}, 6},
      {"321ABC", 933, {105, 32, 1, 65, 66, 67}, 6},
      {"XYZ", 641, {105, 88, 89, 90}, 4},
  };
  for (const auto& test_case : kTestCases) {
    std::vector<int32_t> patterns;
    int32_t checksum =
        CBC_OnedCode128Writer::Encode128C(test_case.input, &patterns);
    EXPECT_EQ(test_case.checksum, checksum);
    EXPECT_THAT(patterns, ElementsAreArray(test_case.pattern_span()));
  }
}

TEST(OnedCode128WriterTest, CheckContentValidity) {
  {
    CBC_OnedCode128Writer writer(BC_TYPE::kCode128B);
    EXPECT_TRUE(writer.CheckContentValidity(L"foo"));
    EXPECT_TRUE(writer.CheckContentValidity(L"xyz"));
    EXPECT_FALSE(writer.CheckContentValidity(L""));
    EXPECT_FALSE(writer.CheckContentValidity(L"\""));
    EXPECT_FALSE(writer.CheckContentValidity(L"f\x10oo"));
    EXPECT_FALSE(writer.CheckContentValidity(L"bar\x7F"));
    EXPECT_FALSE(writer.CheckContentValidity(L"qux\x88"));
  }
  {
    CBC_OnedCode128Writer writer(BC_TYPE::kCode128C);
    EXPECT_TRUE(writer.CheckContentValidity(L"foo"));
    EXPECT_TRUE(writer.CheckContentValidity(L"xyz"));
    EXPECT_FALSE(writer.CheckContentValidity(L""));
    EXPECT_FALSE(writer.CheckContentValidity(L"\""));
    EXPECT_FALSE(writer.CheckContentValidity(L"f\x10oo"));
    EXPECT_FALSE(writer.CheckContentValidity(L"bar\x7F"));
    EXPECT_FALSE(writer.CheckContentValidity(L"qux\x88"));
  }
}

TEST(OnedCode128WriterTest, FilterContents) {
  {
    CBC_OnedCode128Writer writer(BC_TYPE::kCode128B);
    EXPECT_EQ(L"", writer.FilterContents(L""));
    EXPECT_EQ(L"foo", writer.FilterContents(L"foo\x10"));
    EXPECT_EQ(L"fool", writer.FilterContents(L"foo\x10l"));
    EXPECT_EQ(L"foo", writer.FilterContents(L"foo\x10\x7F"));
    EXPECT_EQ(L"foo", writer.FilterContents(L"foo\x10\x7F\x88"));
    EXPECT_EQ(L"bar", writer.FilterContents(L"bar\x10\x7F\x88"));
  }
  {
    CBC_OnedCode128Writer writer(BC_TYPE::kCode128C);
    EXPECT_EQ(L"", writer.FilterContents(L""));
    EXPECT_EQ(L"f", writer.FilterContents(L"foo\x10"));
    EXPECT_EQ(L"f", writer.FilterContents(L"foo\x10l"));
    EXPECT_EQ(L"f", writer.FilterContents(L"foo\x10\x7F"));
    EXPECT_EQ(L"f", writer.FilterContents(L"foo\x10\x7F\x88"));
    EXPECT_EQ(L"ba", writer.FilterContents(L"bar\x10\x7F\x88"));
  }
}

}  // namespace
