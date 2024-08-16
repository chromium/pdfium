// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/dib/cfx_dibitmap.h"

#include <stdint.h>

#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/span.h"
#include "core/fxge/dib/fx_dib.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

using ::testing::ElementsAre;

}  // namespace

TEST(CFXDIBitmapTest, Create) {
  auto pBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  EXPECT_FALSE(pBitmap->Create(400, 300, FXDIB_Format::kInvalid));

  pBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  EXPECT_TRUE(pBitmap->Create(400, 300, FXDIB_Format::k1bppRgb));
}

TEST(CFXDIBitmapTest, CalculatePitchAndSizeGood) {
  // Simple case with no provided pitch.
  std::optional<CFX_DIBitmap::PitchAndSize> result =
      CFX_DIBitmap::CalculatePitchAndSize(100, 200, FXDIB_Format::kBgra, 0);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(400u, result.value().pitch);
  EXPECT_EQ(80000u, result.value().size);

  // Simple case with no provided pitch and different format.
  result =
      CFX_DIBitmap::CalculatePitchAndSize(100, 200, FXDIB_Format::k8bppRgb, 0);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(100u, result.value().pitch);
  EXPECT_EQ(20000u, result.value().size);

  // Simple case with provided pitch matching width * bpp.
  result =
      CFX_DIBitmap::CalculatePitchAndSize(100, 200, FXDIB_Format::kBgra, 400);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(400u, result.value().pitch);
  EXPECT_EQ(80000u, result.value().size);

  // Simple case with provided pitch, where pitch exceeds width * bpp.
  result =
      CFX_DIBitmap::CalculatePitchAndSize(100, 200, FXDIB_Format::kBgra, 455);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(455u, result.value().pitch);
  EXPECT_EQ(91000u, result.value().size);
}

TEST(CFXDIBitmapTest, CalculatePitchAndSizeBad) {
  // Bad width / height.
  static const CFX_Size kBadDimensions[] = {
      {0, 0},   {-1, -1}, {-1, 0},   {0, -1},
      {0, 200}, {100, 0}, {-1, 200}, {100, -1},
  };
  for (const auto& dimension : kBadDimensions) {
    EXPECT_FALSE(CFX_DIBitmap::CalculatePitchAndSize(
        dimension.width, dimension.height, FXDIB_Format::kBgra, 0));
    EXPECT_FALSE(CFX_DIBitmap::CalculatePitchAndSize(
        dimension.width, dimension.height, FXDIB_Format::kBgra, 1));
  }

  // Bad format.
  EXPECT_FALSE(
      CFX_DIBitmap::CalculatePitchAndSize(100, 200, FXDIB_Format::kInvalid, 0));
  EXPECT_FALSE(CFX_DIBitmap::CalculatePitchAndSize(
      100, 200, FXDIB_Format::kInvalid, 800));

  // Width too wide for claimed pitch.
  EXPECT_FALSE(
      CFX_DIBitmap::CalculatePitchAndSize(101, 200, FXDIB_Format::kBgra, 400));

  // Overflow cases with calculated pitch.
  EXPECT_FALSE(CFX_DIBitmap::CalculatePitchAndSize(1073747000, 1,
                                                   FXDIB_Format::kBgra, 0));
  EXPECT_FALSE(CFX_DIBitmap::CalculatePitchAndSize(1048576, 1024,
                                                   FXDIB_Format::kBgra, 0));
  EXPECT_FALSE(CFX_DIBitmap::CalculatePitchAndSize(4194304, 1024,
                                                   FXDIB_Format::k8bppRgb, 0));

  // Overflow cases with provided pitch.
  EXPECT_FALSE(CFX_DIBitmap::CalculatePitchAndSize(
      2147484000u, 1, FXDIB_Format::kBgra, 2147484000u));
  EXPECT_FALSE(CFX_DIBitmap::CalculatePitchAndSize(
      1048576, 1024, FXDIB_Format::kBgra, 4194304));
  EXPECT_FALSE(CFX_DIBitmap::CalculatePitchAndSize(
      4194304, 1024, FXDIB_Format::k8bppRgb, 4194304));
}

TEST(CFXDIBitmapTest, CalculatePitchAndSizeBoundary) {
  // Test boundary condition for pitch overflow.
  std::optional<CFX_DIBitmap::PitchAndSize> result =
      CFX_DIBitmap::CalculatePitchAndSize(536870908, 4, FXDIB_Format::k8bppRgb,
                                          0);
  ASSERT_TRUE(result);
  EXPECT_EQ(536870908u, result.value().pitch);
  EXPECT_EQ(2147483632u, result.value().size);
  EXPECT_FALSE(CFX_DIBitmap::CalculatePitchAndSize(536870909, 4,
                                                   FXDIB_Format::k8bppRgb, 0));

  // Test boundary condition for size overflow.
  result = CFX_DIBitmap::CalculatePitchAndSize(68174084, 63,
                                               FXDIB_Format::k8bppRgb, 0);
  ASSERT_TRUE(result);
  EXPECT_EQ(68174084u, result.value().pitch);
  EXPECT_EQ(4294967292u, result.value().size);
  EXPECT_FALSE(CFX_DIBitmap::CalculatePitchAndSize(68174085, 63,
                                                   FXDIB_Format::k8bppRgb, 0));
}

TEST(CFXDIBitmapTest, GetScanlineAsWith24Bpp) {
  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(3, 3, FXDIB_Format::kBgr));
  EXPECT_EQ(3, bitmap->GetWidth());
  EXPECT_EQ(12u, bitmap->GetPitch());

  EXPECT_EQ(36u, bitmap->GetBuffer().size());
  EXPECT_EQ(12u, bitmap->GetScanline(0).size());
  EXPECT_EQ(3u, bitmap->GetScanlineAs<FX_BGR_STRUCT<uint8_t>>(0).size());

  EXPECT_EQ(36u, bitmap->GetWritableBuffer().size());
  EXPECT_EQ(12u, bitmap->GetWritableScanline(0).size());
  EXPECT_EQ(3u,
            bitmap->GetWritableScanlineAs<FX_BGR_STRUCT<uint8_t>>(0).size());
}

#if defined(PDF_USE_SKIA)
TEST(CFXDIBitmapTest, UnPreMultiplyFromPreMultiplied) {
  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(1, 1, FXDIB_Format::kBgraPremul));
  EXPECT_TRUE(bitmap->IsPremultiplied());
  UNSAFE_TODO(FXARGB_SetDIB(bitmap->GetWritableBuffer().data(), 0x7f'7f'7f'7f));

  bitmap->UnPreMultiply();
  EXPECT_FALSE(bitmap->IsPremultiplied());
  EXPECT_THAT(bitmap->GetBuffer(), ElementsAre(0xff, 0xff, 0xff, 0x7f));
}

TEST(CFXDIBitmapTest, UnPreMultiplyFromUnPreMultiplied) {
  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(1, 1, FXDIB_Format::kBgra));
  EXPECT_FALSE(bitmap->IsPremultiplied());
  UNSAFE_TODO(FXARGB_SetDIB(bitmap->GetWritableBuffer().data(), 0x7f'ff'ff'ff));

  bitmap->UnPreMultiply();
  EXPECT_FALSE(bitmap->IsPremultiplied());
  EXPECT_THAT(bitmap->GetBuffer(), ElementsAre(0xff, 0xff, 0xff, 0x7f));
}

TEST(CFXDIBitmapTest, PreMultiplyFromUnPreMultiplied) {
  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(1, 1, FXDIB_Format::kBgra));
  EXPECT_FALSE(bitmap->IsPremultiplied());
  UNSAFE_TODO(FXARGB_SetDIB(bitmap->GetWritableBuffer().data(), 0x7f'ff'ff'ff));

  bitmap->PreMultiply();
  EXPECT_TRUE(bitmap->IsPremultiplied());
  EXPECT_THAT(bitmap->GetBuffer(), ElementsAre(0x7f, 0x7f, 0x7f, 0x7f));
}

TEST(CFXDIBitmapTest, PreMultiplyFromPreMultiplied) {
  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(1, 1, FXDIB_Format::kBgraPremul));
  EXPECT_TRUE(bitmap->IsPremultiplied());
  UNSAFE_TODO(FXARGB_SetDIB(bitmap->GetWritableBuffer().data(), 0x7f'7f'7f'7f));

  bitmap->PreMultiply();
  EXPECT_TRUE(bitmap->IsPremultiplied());
  EXPECT_THAT(bitmap->GetBuffer(), ElementsAre(0x7f, 0x7f, 0x7f, 0x7f));
}
#endif  // defined(PDF_USE_SKIA)
