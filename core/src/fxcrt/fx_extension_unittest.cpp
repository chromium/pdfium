// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"

#include "../../include/fxcrt/fx_ext.h"

TEST(fxcrt, HexCharToDigit) {
  EXPECT_EQ(10, HexCharToDigit('a'));
  EXPECT_EQ(10, HexCharToDigit('A'));
  EXPECT_EQ(7, HexCharToDigit('7'));
  EXPECT_EQ(0, HexCharToDigit('i'));
}
