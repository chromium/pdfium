// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>

#include "core/fxcrt/fx_number.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(fxnumber, Default) {
  FX_Number number;
  EXPECT_TRUE(number.IsInteger());
  EXPECT_FALSE(number.IsSigned());
  EXPECT_EQ(0, number.GetSigned());
  EXPECT_FLOAT_EQ(0.0f, number.GetFloat());
}

TEST(fxnumber, FromSigned) {
  FX_Number number(-128);
  EXPECT_TRUE(number.IsInteger());
  EXPECT_TRUE(number.IsSigned());
  EXPECT_EQ(-128, number.GetSigned());
  EXPECT_FLOAT_EQ(-128.0f, number.GetFloat());

  // Show that assignment works.
  FX_Number number2 = number;
  EXPECT_TRUE(number2.IsInteger());
  EXPECT_TRUE(number2.IsSigned());
  EXPECT_EQ(-128, number2.GetSigned());
  EXPECT_FLOAT_EQ(-128.0f, number2.GetFloat());
}

TEST(fxnumber, FromFloat) {
  FX_Number number(-100.001f);
  EXPECT_FALSE(number.IsInteger());
  EXPECT_TRUE(number.IsSigned());
  EXPECT_EQ(-100, number.GetSigned());
  EXPECT_FLOAT_EQ(-100.001f, number.GetFloat());

  // Show that assignment works.
  FX_Number number2 = number;
  EXPECT_FALSE(number2.IsInteger());
  EXPECT_TRUE(number2.IsSigned());
  EXPECT_EQ(-100, number2.GetSigned());
  EXPECT_FLOAT_EQ(-100.001f, number2.GetFloat());

  // Show positive saturation.
  FX_Number number3(1e17f);
  EXPECT_FALSE(number3.IsInteger());
  EXPECT_TRUE(number3.IsSigned());
  EXPECT_EQ(std::numeric_limits<int32_t>::max(), number3.GetSigned());

  // Show negative saturation.
  FX_Number number4(-1e17f);
  EXPECT_FALSE(number4.IsInteger());
  EXPECT_TRUE(number4.IsSigned());
  EXPECT_EQ(std::numeric_limits<int32_t>::min(), number4.GetSigned());
}

TEST(fxnumber, FromStringUnsigned) {
  struct TestCase {
    const char* input;
    int expected_output;
  };

  auto test_func = [](pdfium::span<const TestCase> test_cases) {
    for (const auto& test : test_cases) {
      FX_Number number(test.input);
      EXPECT_TRUE(number.IsInteger());
      EXPECT_FALSE(number.IsSigned());
      EXPECT_EQ(test.expected_output, number.GetSigned());
    }
  };

  static constexpr TestCase kNormalCases[] = {
      {"", 0},
      {"0", 0},
      {"10", 10},
  };
  test_func(kNormalCases);

  static constexpr TestCase kOverflowCases[] = {
      {"4223423494965252", 0},
      {"4294967296", 0},
      {"4294967297", 0},
      {"5000000000", 0},
  };
  test_func(kOverflowCases);

  // No explicit sign will allow the number to go negative if retrieved as a
  // signed value. This is needed for things like the encryption permissions
  // flag (Table 3.20 PDF 1.7 spec)
  static constexpr TestCase kNegativeCases[] = {
      {"4294965252", -2044},
      {"4294967247", -49},
      {"4294967248", -48},
      {"4294967292", -4},
      {"4294967295", -1},
  };
  test_func(kNegativeCases);
}

TEST(fxnumber, FromStringSigned) {
  {
    FX_Number number("-0");
    EXPECT_TRUE(number.IsInteger());
    EXPECT_TRUE(number.IsSigned());
    EXPECT_EQ(0, number.GetSigned());
  }
  {
    FX_Number number("+0");
    EXPECT_TRUE(number.IsInteger());
    EXPECT_TRUE(number.IsSigned());
    EXPECT_EQ(0, number.GetSigned());
  }
  {
    FX_Number number("-10");
    EXPECT_TRUE(number.IsInteger());
    EXPECT_TRUE(number.IsSigned());
    EXPECT_EQ(-10, number.GetSigned());
  }
  {
    FX_Number number("+10");
    EXPECT_TRUE(number.IsInteger());
    EXPECT_TRUE(number.IsSigned());
    EXPECT_EQ(10, number.GetSigned());
  }
  {
    FX_Number number("-2147483648");
    EXPECT_TRUE(number.IsInteger());
    EXPECT_TRUE(number.IsSigned());
    EXPECT_EQ(std::numeric_limits<int32_t>::min(), number.GetSigned());
  }
  {
    FX_Number number("+2147483647");
    EXPECT_TRUE(number.IsInteger());
    EXPECT_TRUE(number.IsSigned());
    EXPECT_EQ(std::numeric_limits<int32_t>::max(), number.GetSigned());
  }
  {
    // Value underflows.
    FX_Number number("-2147483649");
    EXPECT_EQ(0, number.GetSigned());
  }
  {
    // Value overflows.
    FX_Number number("+2147483648");
    EXPECT_EQ(0, number.GetSigned());
  }
}

TEST(fxnumber, FromStringFloat) {
  FX_Number number("3.24");
  EXPECT_FLOAT_EQ(3.24f, number.GetFloat());
}
