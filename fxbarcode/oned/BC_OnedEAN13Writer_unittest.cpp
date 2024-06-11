// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxbarcode/oned/BC_OnedEAN13Writer.h"

#include <string.h>

#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/data_vector.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

TEST(OnedEAN13WriterTest, Encode) {
  CBC_OnedEAN13Writer writer;
  writer.InitEANWriter();

  // EAN-13 barcodes encode 13-digit numbers into 95 modules in a unidimensional
  // disposition.
  EXPECT_TRUE(writer.Encode("").empty());
  EXPECT_TRUE(writer.Encode("123").empty());
  EXPECT_TRUE(writer.Encode("123456789012").empty());
  EXPECT_TRUE(writer.Encode("12345678901234").empty());

  static const char kExpected1[] =
      "# #"  // Start
      // 1 implicit by LLGLGG in next 6 digits
      "  #  ##"  // 2 L
      " #### #"  // 3 L
      "  ### #"  // 4 G
      " ##   #"  // 5 L
      "    # #"  // 6 G
      "  #   #"  // 7 G
      " # # "    // Middle
      "#  #   "  // 8 R
      "### #  "  // 9 R
      "###  # "  // 0 R
      "##  ## "  // 1 R
      "## ##  "  // 2 R
      "#  #   "  // 8 R
      "# #";     // End
  DataVector<uint8_t> encoded = writer.Encode("1234567890128");
  for (size_t i = 0; i < strlen(kExpected1); i++) {
    UNSAFE_TODO(EXPECT_EQ(kExpected1[i] != ' ', !!encoded[i])) << i;
  }
  static const char kExpected2[] =
      "# #"  // Start
      // 7 implicit by LGLGLG in next 6 digits
      " ### ##"  // 7 L
      "  #   #"  // 7 G
      " # ####"  // 6 L
      "    # #"  // 6 G
      " # ####"  // 6 L
      " ###  #"  // 5 G
      " # # "    // Middle
      "#  ### "  // 5 R
      "#  ### "  // 5 R
      "# ###  "  // 4 R
      "# ###  "  // 4 R
      "# ###  "  // 4 R
      "###  # "  // 0 R
      "# #";     // End
  encoded = writer.Encode("7776665554440");
  ASSERT_EQ(strlen(kExpected2), encoded.size());
  for (size_t i = 0; i < strlen(kExpected2); i++) {
    UNSAFE_TODO(EXPECT_EQ(kExpected2[i] != ' ', !!encoded[i])) << i;
  }
}

TEST(OnedEAN13WriterTest, Checksum) {
  CBC_OnedEAN13Writer writer;
  writer.InitEANWriter();
  EXPECT_EQ(0, writer.CalcChecksum(""));
  EXPECT_EQ(6, writer.CalcChecksum("123"));
  EXPECT_EQ(8, writer.CalcChecksum("123456789012"));
  EXPECT_EQ(0, writer.CalcChecksum("777666555444"));
}

}  // namespace
