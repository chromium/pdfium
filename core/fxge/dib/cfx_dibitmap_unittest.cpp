// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/dib/cfx_dibitmap.h"

#include "testing/gtest/include/gtest/gtest.h"

TEST(CFX_DIBitmap, Create) {
  auto pBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  EXPECT_FALSE(pBitmap->Create(400, 300, FXDIB_Invalid));

  pBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  EXPECT_TRUE(pBitmap->Create(400, 300, FXDIB_1bppRgb));
}

TEST(CFX_DIBitmap, CalculatePitchAndSizeGood) {
  // Simple case with no provided pitch.
  Optional<CFX_DIBitmap::PitchAndSize> result =
      CFX_DIBitmap::CalculatePitchAndSize(100, 200, FXDIB_Argb, 0);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(400u, result.value().pitch);
  EXPECT_EQ(80000u, result.value().size);

  // Simple case with no provided pitch and different format.
  result = CFX_DIBitmap::CalculatePitchAndSize(100, 200, FXDIB_8bppRgb, 0);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(100u, result.value().pitch);
  EXPECT_EQ(20000u, result.value().size);

  // Simple case with provided pitch.
  result = CFX_DIBitmap::CalculatePitchAndSize(100, 200, FXDIB_Argb, 400);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(400u, result.value().pitch);
  EXPECT_EQ(80000u, result.value().size);

  // Simple case with provided pitch, but pitch does not match width * bpp.
  result = CFX_DIBitmap::CalculatePitchAndSize(100, 200, FXDIB_Argb, 355);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(355u, result.value().pitch);
  EXPECT_EQ(71000u, result.value().size);
}

TEST(CFX_DIBitmap, CalculatePitchAndSizeBad) {
  // Bad width / height.
  static const CFX_Size kBadDimensions[] = {
      {0, 0},   {-1, -1}, {-1, 0},   {0, -1},
      {0, 200}, {100, 0}, {-1, 200}, {100, -1},
  };
  for (const auto& dimension : kBadDimensions) {
    EXPECT_FALSE(CFX_DIBitmap::CalculatePitchAndSize(
        dimension.width, dimension.height, FXDIB_Argb, 0));
    EXPECT_FALSE(CFX_DIBitmap::CalculatePitchAndSize(
        dimension.width, dimension.height, FXDIB_Argb, 1));
  }

  // Bad format.
  EXPECT_FALSE(CFX_DIBitmap::CalculatePitchAndSize(100, 200, FXDIB_Invalid, 0));
  EXPECT_FALSE(
      CFX_DIBitmap::CalculatePitchAndSize(100, 200, FXDIB_Invalid, 800));

  // Overflow cases with calculated pitch.
  EXPECT_FALSE(
      CFX_DIBitmap::CalculatePitchAndSize(1073747000, 1, FXDIB_Argb, 0));
  EXPECT_FALSE(
      CFX_DIBitmap::CalculatePitchAndSize(1048576, 1024, FXDIB_Argb, 0));
  EXPECT_FALSE(
      CFX_DIBitmap::CalculatePitchAndSize(4194304, 1024, FXDIB_8bppRgb, 0));

  // Overflow cases with provided pitch.
  EXPECT_FALSE(CFX_DIBitmap::CalculatePitchAndSize(2147484000u, 1, FXDIB_Argb,
                                                   2147484000u));
  EXPECT_FALSE(
      CFX_DIBitmap::CalculatePitchAndSize(1048576, 1024, FXDIB_Argb, 4194304));
  EXPECT_FALSE(CFX_DIBitmap::CalculatePitchAndSize(4194304, 1024, FXDIB_8bppRgb,
                                                   4194304));
}

TEST(CFX_DIBitmap, CalculatePitchAndSizeBoundary) {
  // Test boundary condition for pitch overflow.
  Optional<CFX_DIBitmap::PitchAndSize> result =
      CFX_DIBitmap::CalculatePitchAndSize(536870908, 4, FXDIB_8bppRgb, 0);
  ASSERT_TRUE(result);
  EXPECT_EQ(536870908u, result.value().pitch);
  EXPECT_EQ(2147483632u, result.value().size);
  EXPECT_FALSE(
      CFX_DIBitmap::CalculatePitchAndSize(536870909, 4, FXDIB_8bppRgb, 0));

  // Test boundary condition for size overflow.
  result = CFX_DIBitmap::CalculatePitchAndSize(68174084, 63, FXDIB_8bppRgb, 0);
  ASSERT_TRUE(result);
  EXPECT_EQ(68174084u, result.value().pitch);
  EXPECT_EQ(4294967292u, result.value().size);
  EXPECT_FALSE(
      CFX_DIBitmap::CalculatePitchAndSize(68174085, 63, FXDIB_8bppRgb, 0));
}
