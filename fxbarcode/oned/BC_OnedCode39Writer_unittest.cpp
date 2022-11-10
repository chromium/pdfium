// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxbarcode/oned/BC_OnedCode39Writer.h"

#include <string.h>

#include "core/fxcrt/data_vector.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

TEST(OnedCode39WriterTest, SetWideNarrowRatio) {
  // Code 39 barcodes encode strings of any size into modules in a
  // unidimensional disposition.
  // Each module is either: a narrow bar, a narrow space, a wide
  // bar, or a wide space. Accepted wide-to-narrow ratios are between 2 and 3.
  // This writer in particular only takes integer ratios, so it's either 2 or 3.
  CBC_OnedCode39Writer writer;
  EXPECT_FALSE(writer.SetWideNarrowRatio(0));
  EXPECT_FALSE(writer.SetWideNarrowRatio(1));
  EXPECT_TRUE(writer.SetWideNarrowRatio(2));
  EXPECT_TRUE(writer.SetWideNarrowRatio(3));
  EXPECT_FALSE(writer.SetWideNarrowRatio(4));
  EXPECT_FALSE(writer.SetWideNarrowRatio(100));

  writer.SetWideNarrowRatio(3);

  static const char kExpected1[] =
      "#   # ### ### # "  // * Start
      "# ### ### #   # "  // P
      "# # ###   # ### "  // D
      "# ### ###   # # "  // F
      "# ### #   ### # "  // I
      "###   # # # ### "  // U
      "### ### # #   # "  // M
      "#   # ### ### #";  // * End
  DataVector<uint8_t> encoded = writer.Encode("PDFIUM");
  ASSERT_EQ(strlen(kExpected1), encoded.size());
  for (size_t i = 0; i < strlen(kExpected1); i++)
    EXPECT_EQ(kExpected1[i] != ' ', !!encoded[i]) << i;

  writer.SetWideNarrowRatio(2);

  static const char kExpected2[] =
      "#  # ## ## # "  // * Start
      "# ## ## #  # "  // P
      "# # ##  # ## "  // D
      "# ## ##  # # "  // F
      "# ## #  ## # "  // I
      "##  # # # ## "  // U
      "## ## # #  # "  // M
      "#  # ## ## #";  // * End
  encoded = writer.Encode("PDFIUM");
  ASSERT_EQ(strlen(kExpected2), encoded.size());
  for (size_t i = 0; i < strlen(kExpected2); i++)
    EXPECT_EQ(kExpected2[i] != ' ', !!encoded[i]) << i;
}

TEST(OnedCode39WriterTest, Encode) {
  CBC_OnedCode39Writer writer;

  static const char kExpected1[] =
      "#   # ### ### # "  // * Start
      "#   # ### ### #";  // * End
  DataVector<uint8_t> encoded = writer.Encode("");
  ASSERT_EQ(strlen(kExpected1), encoded.size());
  for (size_t i = 0; i < strlen(kExpected1); i++)
    EXPECT_EQ(kExpected1[i] != ' ', !!encoded[i]) << i;

  static const char kExpected2[] =
      "#   # ### ### # "  // * Start
      "### #   # # ### "  // 1
      "# ###   # # ### "  // 2
      "### ###   # # # "  // 3
      "#   # ### ### #";  // * End
  encoded = writer.Encode("123");
  ASSERT_EQ(strlen(kExpected2), encoded.size());
  for (size_t i = 0; i < strlen(kExpected2); i++)
    EXPECT_EQ(kExpected2[i] != ' ', !!encoded[i]) << i;

  static const char kExpected3[] =
      "#   # ### ### # "  // * Start
      "# ### ### #   # "  // P
      "# # ###   # ### "  // D
      "# ### ###   # # "  // F
      "# ### #   ### # "  // I
      "###   # # # ### "  // U
      "### ### # #   # "  // M
      "#   # ### ### #";  // * End
  encoded = writer.Encode("PDFIUM");
  ASSERT_EQ(strlen(kExpected3), encoded.size());
  for (size_t i = 0; i < strlen(kExpected3); i++)
    EXPECT_EQ(kExpected3[i] != ' ', !!encoded[i]) << i;

  static const char kExpected4[] =
      "#   # ### ### # "  // * Start
      "### # #   # ### "  // A
      "#   ### # ### # "  // Space
      "#   # # ### ### "  // -
      "#   #   #   # # "  // $
      "# #   #   #   # "  // %
      "###   # # ### # "  // .
      "#   #   # #   # "  // /
      "#   # #   #   # "  // +
      "#   ### ### # # "  // Z
      "#   # ### ### #";  // * End
  encoded = writer.Encode("A -$%./+Z");
  ASSERT_EQ(strlen(kExpected4), encoded.size());
  for (size_t i = 0; i < strlen(kExpected4); i++)
    EXPECT_EQ(kExpected4[i] != ' ', !!encoded[i]) << i;
}

TEST(OnedCode39WriterTest, Checksum) {
  CBC_OnedCode39Writer writer;
  writer.SetCalcChecksum(true);

  static const char kExpected1[] =
      "#   # ### ### # "  // * Start
      "### #   # # ### "  // 1 (1)
      "# ###   # # ### "  // 2 (2)
      "### ###   # # # "  // 3 (3)
      "# ###   ### # # "  // 6 (6 = (1 + 2 + 3) % 43)
      "#   # ### ### #";  // * End
  DataVector<uint8_t> encoded = writer.Encode("123");
  ASSERT_EQ(strlen(kExpected1), encoded.size());
  for (size_t i = 0; i < strlen(kExpected1); i++)
    EXPECT_EQ(kExpected1[i] != ' ', !!encoded[i]) << i;

  static const char kExpected2[] =
      "#   # ### ### # "  // * Start
      "# ### ### #   # "  // P (25)
      "# # ###   # ### "  // D (13)
      "# ### ###   # # "  // F (15)
      "# ### #   ### # "  // I (18)
      "###   # # # ### "  // U (30)
      "### ### # #   # "  // M (22)
      "###   # # ### # "  // . (37 = (25 + 13 + 15 + 18 + 30 + 22) % 43)
      "#   # ### ### #";  // * End
  encoded = writer.Encode("PDFIUM");
  ASSERT_EQ(strlen(kExpected2), encoded.size());
  for (size_t i = 0; i < strlen(kExpected2); i++)
    EXPECT_EQ(kExpected2[i] != ' ', !!encoded[i]) << i;
}

}  // namespace
