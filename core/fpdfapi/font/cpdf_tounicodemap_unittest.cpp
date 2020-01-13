// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/font/cpdf_tounicodemap.h"

#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(cpdf_tounicodemap, StringToCode) {
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<0001>"), testing::Optional(1u));
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<c2>"), testing::Optional(194u));
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<A2>"), testing::Optional(162u));
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<Af2>"),
              testing::Optional(2802u));
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<FFFFFFFF>"),
              testing::Optional(4294967295u));

  // Integer overflow
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<100000000>").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<1abcdFFFF>").has_value());

  // Invalid string
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<>").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("12").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<12").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("12>").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<1-7>").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("00AB").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<00NN>").has_value());
}

TEST(cpdf_tounicodemap, StringToWideString) {
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString(""));
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString("1234"));
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString("<c2"));
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString("<c2D2"));
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString("c2ab>"));

  WideString res = L"\xc2ab";
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2ab>"));
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2abab>"));
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2ab 1234>"));

  res += L"\xfaab";
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2abFaAb>"));
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2abFaAb12>"));
}
