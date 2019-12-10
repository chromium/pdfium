// Copyright 2018 PDFium Authors. All rights reserved.
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
}

TEST(fxnumber, FromStringUnsigned) {
  {
    FX_Number number("");
    EXPECT_TRUE(number.IsInteger());
    EXPECT_FALSE(number.IsSigned());
  }
  {
    FX_Number number("0");
    EXPECT_TRUE(number.IsInteger());
    EXPECT_FALSE(number.IsSigned());
  }
  {
    FX_Number number("10");
    EXPECT_TRUE(number.IsInteger());
    EXPECT_FALSE(number.IsSigned());
  }
  {
    FX_Number number("4294967295");
    EXPECT_TRUE(number.IsInteger());
    EXPECT_FALSE(number.IsSigned());
  }
  {
    // Value overflows.
    FX_Number number("4223423494965252");
    EXPECT_TRUE(number.IsInteger());
    EXPECT_FALSE(number.IsSigned());
  }
  {
    // No explicit sign will allow the number to go negative if we retrieve
    // it as a signed value. This is needed for things like the encryption
    // Permissions flag (Table 3.20 PDF 1.7 spec)
    FX_Number number("4294965252");
    EXPECT_EQ(-2044, number.GetSigned());
  }
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
