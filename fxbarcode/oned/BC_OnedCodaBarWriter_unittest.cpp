// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxbarcode/oned/BC_OnedCodaBarWriter.h"

#include <string.h>

#include "core/fxcrt/data_vector.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

TEST(OnedCodaBarWriterTest, Encode) {
  CBC_OnedCodaBarWriter writer;

  static const char kExpected1[] =
      "# ##  #  # "  // A Start
      "#  #  # ##";  // B End
  DataVector<uint8_t> encoded = writer.Encode("");
  ASSERT_EQ(strlen(kExpected1), encoded.size());
  for (size_t i = 0; i < strlen(kExpected1); i++)
    EXPECT_EQ(kExpected1[i] != ' ', !!encoded[i]) << i;

  static const char kExpected2[] =
      "# ##  #  # "  // A Start
      "# # ##  # "   // 1
      "# #  # ## "   // 2
      "##  # # # "   // 3
      "#  #  # ##";  // B End
  encoded = writer.Encode("123");
  ASSERT_EQ(strlen(kExpected2), encoded.size());
  for (size_t i = 0; i < strlen(kExpected2); i++)
    EXPECT_EQ(kExpected2[i] != ' ', !!encoded[i]) << i;

  static const char kExpected3[] =
      "# ##  #  # "  // A Start
      "# #  ## # "   // -
      "# ##  # # "   // $
      "## ## ## # "  // .
      "## ## # ## "  // /
      "## # ## ## "  // :
      "# ## ## ## "  // +
      "#  #  # ##";  // B End
  encoded = writer.Encode("-$./:+");
  ASSERT_EQ(strlen(kExpected3), encoded.size());
  for (size_t i = 0; i < strlen(kExpected3); i++)
    EXPECT_EQ(kExpected3[i] != ' ', !!encoded[i]) << i;

  static const char kExpected4[] =
      "# ##  #  # "  // A Start
      "# ## #  # "   // 4
      "## # #  # "   // 5
      "#  # # ## "   // 6
      "## ## ## # "  // .
      "## #  # # "   // 9
      "#  ## # # "   // 8
      "#  # ## # "   // 7
      "## #  # # "   // 9
      "#  ## # # "   // 8
      "#  # ## # "   // 7
      "## #  # # "   // 9
      "#  ## # # "   // 8
      "#  # ## # "   // 7
      "## ## # ## "  // /
      "# # #  ## "   // 0
      "# # #  ## "   // 0
      "# # ##  # "   // 1
      "#  #  # ##";  // B End
  encoded = writer.Encode("456.987987987/001");
  ASSERT_EQ(strlen(kExpected4), encoded.size());
  for (size_t i = 0; i < strlen(kExpected4); i++)
    EXPECT_EQ(kExpected4[i] != ' ', !!encoded[i]) << i;
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

  writer.SetStartChar('N');
  writer.SetEndChar('*');

  static const char kExpected[] =
      "#  #  # ## "  // N (same as B) Start
      "## #  # # "   // 9
      "#  ## # # "   // 8
      "#  # ## # "   // 7
      "# #  #  ##";  // * (same as C) End
  DataVector<uint8_t> encoded = writer.Encode("987");
  ASSERT_EQ(strlen(kExpected), encoded.size());
  for (size_t i = 0; i < strlen(kExpected); i++)
    EXPECT_EQ(kExpected[i] != ' ', !!encoded[i]) << i;
}

}  // namespace
