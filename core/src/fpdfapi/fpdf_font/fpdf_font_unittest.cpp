// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"

#include "font_int.h"

TEST(fpdf_font, StringToCode) {
  EXPECT_EQ(0, CPDF_ToUnicodeMap::StringToCode(""));
  EXPECT_EQ(194, CPDF_ToUnicodeMap::StringToCode("<c2"));
  EXPECT_EQ(162, CPDF_ToUnicodeMap::StringToCode("<A2"));
  EXPECT_EQ(2802, CPDF_ToUnicodeMap::StringToCode("<Af2"));
  EXPECT_EQ(12, CPDF_ToUnicodeMap::StringToCode("12"));
  EXPECT_EQ(128, CPDF_ToUnicodeMap::StringToCode("128"));
}

TEST(fpdf_font, StringToWideString) {
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString(""));
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString("1234"));

  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString("<c2"));

  CFX_WideString res = L"\xc2ab";
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2ab"));
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2abab"));
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2ab 1234"));

  res += L"\xfaab";
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2abFaAb"));
}
