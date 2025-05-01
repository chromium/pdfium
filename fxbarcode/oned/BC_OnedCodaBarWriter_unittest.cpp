// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxbarcode/oned/BC_OnedCodaBarWriter.h"

#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/span.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

TEST(OnedCodaBarWriterTest, Encode) {
  CBC_OnedCodaBarWriter writer;

  static constexpr auto kExpected1 = pdfium::span_from_cstring(
      "# ##  #  # "   // A Start
      "#  #  # ##");  // B End
  DataVector<uint8_t> encoded = writer.Encode("");
  ASSERT_EQ(kExpected1.size(), encoded.size());
  for (size_t i = 0; i < kExpected1.size(); i++) {
    EXPECT_EQ(kExpected1[i] != ' ', !!encoded[i]) << i;
  }
  static constexpr auto kExpected2 = pdfium::span_from_cstring(
      "# ##  #  # "   // A Start
      "# # ##  # "    // 1
      "# #  # ## "    // 2
      "##  # # # "    // 3
      "#  #  # ##");  // B End
  encoded = writer.Encode("123");
  ASSERT_EQ(kExpected2.size(), encoded.size());
  for (size_t i = 0; i < kExpected2.size(); i++) {
    EXPECT_EQ(kExpected2[i] != ' ', !!encoded[i]) << i;
  }
  static constexpr auto kExpected3 = pdfium::span_from_cstring(
      "# ##  #  # "   // A Start
      "# #  ## # "    // -
      "# ##  # # "    // $
      "## ## ## # "   // .
      "## ## # ## "   // /
      "## # ## ## "   // :
      "# ## ## ## "   // +
      "#  #  # ##");  // B End
  encoded = writer.Encode("-$./:+");
  ASSERT_EQ(kExpected3.size(), encoded.size());
  for (size_t i = 0; i < kExpected3.size(); i++) {
    EXPECT_EQ(kExpected3[i] != ' ', !!encoded[i]) << i;
  }
  static constexpr auto kExpected4 = pdfium::span_from_cstring(
      "# ##  #  # "   // A Start
      "# ## #  # "    // 4
      "## # #  # "    // 5
      "#  # # ## "    // 6
      "## ## ## # "   // .
      "## #  # # "    // 9
      "#  ## # # "    // 8
      "#  # ## # "    // 7
      "## #  # # "    // 9
      "#  ## # # "    // 8
      "#  # ## # "    // 7
      "## #  # # "    // 9
      "#  ## # # "    // 8
      "#  # ## # "    // 7
      "## ## # ## "   // /
      "# # #  ## "    // 0
      "# # #  ## "    // 0
      "# # ##  # "    // 1
      "#  #  # ##");  // B End
  encoded = writer.Encode("456.987987987/001");
  ASSERT_EQ(kExpected4.size(), encoded.size());
  for (size_t i = 0; i < kExpected4.size(); i++) {
    EXPECT_EQ(kExpected4[i] != ' ', !!encoded[i]) << i;
  }
}

TEST(OnedCodaBarWriterTest, SetDelimiters) {
  CBC_OnedCodaBarWriter writer;

  EXPECT_TRUE(writer.SetStartChar('A'));
  EXPECT_TRUE(writer.SetStartChar('B'));
  EXPECT_TRUE(writer.SetStartChar('C'));
  EXPECT_TRUE(writer.SetStartChar('D'));
  EXPECT_TRUE(writer.SetStartChar('E'));
  EXPECT_TRUE(writer.SetStartChar('N'));
  EXPECT_TRUE(writer.SetStartChar('T'));
  EXPECT_TRUE(writer.SetStartChar('*'));
  EXPECT_FALSE(writer.SetStartChar('V'));
  EXPECT_FALSE(writer.SetStartChar('0'));
  EXPECT_FALSE(writer.SetStartChar('\0'));
  EXPECT_FALSE(writer.SetStartChar('@'));

  EXPECT_TRUE(writer.SetEndChar('A'));
  EXPECT_TRUE(writer.SetEndChar('B'));
  EXPECT_TRUE(writer.SetEndChar('C'));
  EXPECT_TRUE(writer.SetEndChar('D'));
  EXPECT_TRUE(writer.SetEndChar('E'));
  EXPECT_TRUE(writer.SetEndChar('N'));
  EXPECT_TRUE(writer.SetEndChar('T'));
  EXPECT_TRUE(writer.SetEndChar('*'));
  EXPECT_FALSE(writer.SetEndChar('V'));
  EXPECT_FALSE(writer.SetEndChar('0'));
  EXPECT_FALSE(writer.SetEndChar('\0'));
  EXPECT_FALSE(writer.SetEndChar('@'));

  EXPECT_TRUE(writer.SetStartChar('N'));
  EXPECT_TRUE(writer.SetEndChar('*'));

  static constexpr auto kExpected = pdfium::span_from_cstring(
      "#  #  # ## "   // N (same as B) Start
      "## #  # # "    // 9
      "#  ## # # "    // 8
      "#  # ## # "    // 7
      "# #  #  ##");  // * (same as C) End
  DataVector<uint8_t> encoded = writer.Encode("987");
  ASSERT_EQ(kExpected.size(), encoded.size());
  for (size_t i = 0; i < kExpected.size(); i++) {
    EXPECT_EQ(kExpected[i] != ' ', !!encoded[i]) << i;
  }
}

}  // namespace
