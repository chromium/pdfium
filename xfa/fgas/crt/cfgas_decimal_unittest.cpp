// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fgas/crt/cfgas_decimal.h"

#include <limits>

#include "testing/gtest/include/gtest/gtest.h"

TEST(CFGAS_Decimal, Empty) {
  CFGAS_Decimal empty;
  EXPECT_EQ(L"0", empty.ToWideString());
  EXPECT_EQ(0.0f, empty.ToFloat());
  EXPECT_EQ(0.0, empty.ToDouble());
}

TEST(CFGAS_Decimal, FromInt32) {
  CFGAS_Decimal big(std::numeric_limits<int32_t>::max());
  CFGAS_Decimal small(std::numeric_limits<int32_t>::min());
  EXPECT_STREQ(L"2147483647", big.ToWideString().c_str());
  EXPECT_STREQ(L"-2147483648", small.ToWideString().c_str());
}

TEST(CFGAS_Decimal, FromUint32) {
  CFGAS_Decimal big(std::numeric_limits<uint32_t>::max());
  CFGAS_Decimal small(std::numeric_limits<uint32_t>::min());
  EXPECT_STREQ(L"4294967295", big.ToWideString().c_str());
  EXPECT_STREQ(L"0", small.ToWideString().c_str());
}

TEST(CFGAS_Decimal, FromUint64) {
  CFGAS_Decimal big(std::numeric_limits<uint64_t>::max());
  CFGAS_Decimal small(std::numeric_limits<uint64_t>::min());
  EXPECT_STREQ(L"18446744073709551615", big.ToWideString().c_str());
  EXPECT_STREQ(L"0", small.ToWideString().c_str());
}

TEST(CFGAS_Decimal, FromFloat) {
  WideString big = CFGAS_Decimal(powf(2.0f, 95.0f), 0).ToWideString();
  WideString big_expected = L"39614081257132168796771975168";

  // Precision may not be the same on all platforms.
  EXPECT_EQ(big_expected.GetLength(), big.GetLength());
  EXPECT_STREQ(big_expected.First(8).c_str(), big.First(8).c_str());

  WideString tiny = CFGAS_Decimal(1e20f, 0).ToWideString();
  WideString tiny_expected = L"100000000000000000000";
  EXPECT_EQ(tiny_expected.GetLength(), tiny.GetLength());
  EXPECT_STREQ(tiny_expected.First(8).c_str(), tiny.First(8).c_str());

  WideString teeny = CFGAS_Decimal(1e14f, 4).ToWideString();
  WideString teeny_expected = L"100000000000000.0000";
  EXPECT_EQ(teeny_expected.GetLength(), teeny.GetLength());
  EXPECT_STREQ(teeny_expected.First(8).c_str(), teeny.First(8).c_str());
}

TEST(CFGAS_Decimal, FromFloatFractional) {
  WideString case1 = CFGAS_Decimal(123.456f, 10).ToWideString();
  WideString case1_expected = L"123.4560000000";

  // Precision may not be the same on all platforms.
  EXPECT_EQ(case1_expected.GetLength(), case1.GetLength());
  EXPECT_STREQ(case1_expected.First(8).c_str(), case1.First(8).c_str());
}

TEST(CFGAS_Decimal, FromString) {
  CFGAS_Decimal big(L"100000000000000000000000000");
  CFGAS_Decimal small(L"-1000000000000000000000000");
  EXPECT_STREQ(L"100000000000000000000000000", big.ToWideString().c_str());
  EXPECT_STREQ(L"-1000000000000000000000000", small.ToWideString().c_str());
}

TEST(CFGAS_Decimal, FromString28Digits) {
  CFGAS_Decimal frac(L"32109876543210.0123456890123");
  EXPECT_STREQ(L"32109876543210.0123456890123", frac.ToWideString().c_str());
}
