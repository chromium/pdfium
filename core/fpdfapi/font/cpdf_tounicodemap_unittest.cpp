// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/font/cpdf_tounicodemap.h"

#include "testing/gtest/include/gtest/gtest.h"

TEST(cpdf_tounicodemap, StringToCode) {
  Optional<uint32_t> result = CPDF_ToUnicodeMap::StringToCode("<c2");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(194u, result.value());

  result = CPDF_ToUnicodeMap::StringToCode("<A2");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(162u, result.value());

  result = CPDF_ToUnicodeMap::StringToCode("<Af2");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(2802u, result.value());

  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("12").has_value());
}

TEST(cpdf_tounicodemap, StringToWideString) {
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString(""));
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString("1234"));

  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString("<c2"));

  WideString res = L"\xc2ab";
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2ab"));
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2abab"));
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2ab 1234"));

  res += L"\xfaab";
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2abFaAb"));
}
