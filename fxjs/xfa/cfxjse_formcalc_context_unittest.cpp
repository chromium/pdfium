// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/xfa/cfxjse_formcalc_context.h"

#include "core/fxcrt/bytestring.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(FormCalcContextTest, GenerateSomExpression) {
  ByteString result =
      CFXJSE_FormCalcContext::GenerateSomExpression("", 0, 0, /*bIsStar=*/true);
  EXPECT_EQ(result, "[*]");

  result = CFXJSE_FormCalcContext::GenerateSomExpression("foo", 0, 0,
                                                         /*bIsStar=*/true);
  EXPECT_EQ(result, "foo[*]");

  result = CFXJSE_FormCalcContext::GenerateSomExpression("foo", 0, 0,
                                                         /*bIsStar=*/false);
  EXPECT_EQ(result, "foo");

  result = CFXJSE_FormCalcContext::GenerateSomExpression("fu", 1, 0,
                                                         /*bIsStar=*/false);
  EXPECT_EQ(result, "fu[0]");

  result = CFXJSE_FormCalcContext::GenerateSomExpression("food", 1, 99,
                                                         /*bIsStar=*/false);
  EXPECT_EQ(result, "food[99]");

  result = CFXJSE_FormCalcContext::GenerateSomExpression("foot", 1, -65,
                                                         /*bIsStar=*/false);
  EXPECT_EQ(result, "foot[-65]");

  result = CFXJSE_FormCalcContext::GenerateSomExpression("football", 2, 0,
                                                         /*bIsStar=*/false);
  EXPECT_EQ(result, "football[0]");

  result = CFXJSE_FormCalcContext::GenerateSomExpression("foosball", 2, 123,
                                                         /*bIsStar=*/false);
  EXPECT_EQ(result, "foosball[+123]");

  result = CFXJSE_FormCalcContext::GenerateSomExpression("bar", 2, -654,
                                                         /*bIsStar=*/false);
  EXPECT_EQ(result, "bar[-654]");

  result = CFXJSE_FormCalcContext::GenerateSomExpression("barb", 2, 2147483647,
                                                         /*bIsStar=*/false);
  EXPECT_EQ(result, "barb[+2147483647]");

  result = CFXJSE_FormCalcContext::GenerateSomExpression(
      "bart", 2, -2147483648, /*bIsStar=*/false);
  EXPECT_EQ(result, "bart[-0]");

  result = CFXJSE_FormCalcContext::GenerateSomExpression("bark", 3, 0,
                                                         /*bIsStar=*/false);
  EXPECT_EQ(result, "bark[0]");

  result = CFXJSE_FormCalcContext::GenerateSomExpression("bard", 3, 357,
                                                         /*bIsStar=*/false);
  EXPECT_EQ(result, "bard[-357]");

  result = CFXJSE_FormCalcContext::GenerateSomExpression("bars", 3, -9876,
                                                         /*bIsStar=*/false);
  EXPECT_EQ(result, "bars[9876]");

  result = CFXJSE_FormCalcContext::GenerateSomExpression("cars", 3, 2147483647,
                                                         /*bIsStar=*/false);
  EXPECT_EQ(result, "cars[-2147483647]");

  result = CFXJSE_FormCalcContext::GenerateSomExpression(
      "mars", 3, -2147483648, /*bIsStar=*/false);
  EXPECT_EQ(result, "mars[0]");
}
