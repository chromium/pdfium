// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_extension.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(fxcrt, FXSYS_HexCharToInt) {
  EXPECT_EQ(10, FXSYS_HexCharToInt('a'));
  EXPECT_EQ(10, FXSYS_HexCharToInt('A'));
  EXPECT_EQ(7, FXSYS_HexCharToInt('7'));
  EXPECT_EQ(0, FXSYS_HexCharToInt('i'));
}

TEST(fxcrt, FXSYS_DecimalCharToInt) {
  EXPECT_EQ(7, FXSYS_DecimalCharToInt('7'));
  EXPECT_EQ(0, FXSYS_DecimalCharToInt('a'));
  EXPECT_EQ(7, FXSYS_DecimalCharToInt(L'7'));
  EXPECT_EQ(0, FXSYS_DecimalCharToInt(L'a'));
}

TEST(fxcrt, FXSYS_isDecimalDigit) {
  EXPECT_TRUE(FXSYS_isDecimalDigit('7'));
  EXPECT_TRUE(FXSYS_isDecimalDigit(L'7'));
  EXPECT_FALSE(FXSYS_isDecimalDigit('a'));
  EXPECT_FALSE(FXSYS_isDecimalDigit(L'a'));
}

TEST(fxcrt, FX_HashCode_Ascii) {
  EXPECT_EQ(0u, FX_HashCode_GetA("", false));
  EXPECT_EQ(65u, FX_HashCode_GetA("A", false));
  EXPECT_EQ(97u, FX_HashCode_GetA("A", true));
  EXPECT_EQ(31 * 65u + 66u, FX_HashCode_GetA("AB", false));
}

TEST(fxcrt, FX_HashCode_Wide) {
  EXPECT_EQ(0u, FX_HashCode_GetW(L"", false));
  EXPECT_EQ(65u, FX_HashCode_GetW(L"A", false));
  EXPECT_EQ(97u, FX_HashCode_GetW(L"A", true));
  EXPECT_EQ(1313 * 65u + 66u, FX_HashCode_GetW(L"AB", false));
}
