// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"

#include "core/include/fxcrt/fx_ext.h"

TEST(fxcrt, FXSYS_toHexDigit) {
  EXPECT_EQ(10, FXSYS_toHexDigit('a'));
  EXPECT_EQ(10, FXSYS_toHexDigit('A'));
  EXPECT_EQ(7, FXSYS_toHexDigit('7'));
  EXPECT_EQ(0, FXSYS_toHexDigit('i'));
}

TEST(fxcrt, FXSYS_toDecimalDigit) {
  EXPECT_EQ(7, FXSYS_toDecimalDigit('7'));
  EXPECT_EQ(0, FXSYS_toDecimalDigit('a'));
}

TEST(fxcrt, FXSYS_toDecimalDigitWide) {
  EXPECT_EQ(7, FXSYS_toDecimalDigitWide(L'7'));
  EXPECT_EQ(0, FXSYS_toDecimalDigitWide(L'a'));
}
