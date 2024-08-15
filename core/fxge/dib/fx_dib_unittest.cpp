// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/dib/fx_dib.h"

#include <stdint.h>

#include "testing/gtest/include/gtest/gtest.h"

TEST(FxDibTest, ArgbToBGRAStruct) {
  FX_BGRA_STRUCT<uint8_t> white = ArgbToBGRAStruct(0xffffffff);
  EXPECT_EQ(0xff, white.blue);
  EXPECT_EQ(0xff, white.green);
  EXPECT_EQ(0xff, white.red);
  EXPECT_EQ(0xff, white.alpha);

  FX_BGRA_STRUCT<uint8_t> black = ArgbToBGRAStruct(0xff000000);
  EXPECT_EQ(0, black.blue);
  EXPECT_EQ(0, black.green);
  EXPECT_EQ(0, black.red);
  EXPECT_EQ(0xff, black.alpha);

  FX_BGRA_STRUCT<uint8_t> abeebead = ArgbToBGRAStruct(0xabeebead);
  EXPECT_EQ(0xad, abeebead.blue);
  EXPECT_EQ(0xbe, abeebead.green);
  EXPECT_EQ(0xee, abeebead.red);
  EXPECT_EQ(0xab, abeebead.alpha);
}

#if defined(PDF_USE_SKIA)
TEST(PreMultiplyTest, PreMultiplyColor) {
  FX_ABGR_STRUCT<uint8_t> result = PreMultiplyColor(FX_ABGR_STRUCT<uint8_t>{
      .alpha = 192, .blue = 0, .green = 255, .red = 100});
  EXPECT_EQ(192, result.alpha);
  EXPECT_EQ(0, result.blue);
  EXPECT_EQ(192, result.green);
  EXPECT_EQ(75, result.red);

  FX_RGBA_STRUCT<float> result_f = PreMultiplyColor(
      FX_RGBA_STRUCT<float>{.red = 100, .green = 255, .blue = 0, .alpha = 192});
  EXPECT_FLOAT_EQ(75.294121f, result_f.red);
  EXPECT_FLOAT_EQ(192.0f, result_f.green);
  EXPECT_FLOAT_EQ(0.0f, result_f.blue);
  EXPECT_FLOAT_EQ(192.0f, result_f.alpha);
}

TEST(PreMultiplyTest, PreMultiplyColorFullyTransparent) {
  FX_ABGR_STRUCT<uint8_t> result = PreMultiplyColor(
      FX_ABGR_STRUCT<uint8_t>{.alpha = 0, .blue = 0, .green = 255, .red = 100});
  EXPECT_EQ(0, result.alpha);
  EXPECT_EQ(0, result.blue);
  EXPECT_EQ(0, result.green);
  EXPECT_EQ(0, result.red);

  FX_RGBA_STRUCT<float> result_f = PreMultiplyColor(
      FX_RGBA_STRUCT<float>{.red = 100, .green = 255, .blue = 0, .alpha = 0});
  EXPECT_FLOAT_EQ(0.0f, result_f.red);
  EXPECT_FLOAT_EQ(0.0f, result_f.green);
  EXPECT_FLOAT_EQ(0.0f, result_f.blue);
  EXPECT_FLOAT_EQ(0.0f, result_f.alpha);
}

TEST(PreMultiplyTest, PreMultiplyColorFullyOpaque) {
  FX_ABGR_STRUCT<uint8_t> result = PreMultiplyColor(FX_ABGR_STRUCT<uint8_t>{
      .alpha = 255, .blue = 0, .green = 255, .red = 100});
  EXPECT_EQ(255, result.alpha);
  EXPECT_EQ(0, result.blue);
  EXPECT_EQ(255, result.green);
  EXPECT_EQ(100, result.red);

  FX_RGBA_STRUCT<float> result_f = PreMultiplyColor(
      FX_RGBA_STRUCT<float>{.red = 100, .green = 255, .blue = 0, .alpha = 255});
  EXPECT_FLOAT_EQ(100.0f, result_f.red);
  EXPECT_FLOAT_EQ(255.0f, result_f.green);
  EXPECT_FLOAT_EQ(0.0f, result_f.blue);
  EXPECT_FLOAT_EQ(255.0f, result_f.alpha);
}

TEST(PreMultiplyTest, UnPreMultiplyColor) {
  FX_ABGR_STRUCT<uint8_t> result = UnPreMultiplyColor(FX_ABGR_STRUCT<uint8_t>{
      .alpha = 192, .blue = 0, .green = 192, .red = 100});
  EXPECT_EQ(192, result.alpha);
  EXPECT_EQ(0, result.blue);
  EXPECT_EQ(255, result.green);
  EXPECT_EQ(132, result.red);

  FX_RGBA_STRUCT<float> result_f = UnPreMultiplyColor(
      FX_RGBA_STRUCT<float>{.red = 100, .green = 192, .blue = 0, .alpha = 192});
  EXPECT_FLOAT_EQ(132.8125f, result_f.red);
  EXPECT_FLOAT_EQ(255.0f, result_f.green);
  EXPECT_FLOAT_EQ(0.0f, result_f.blue);
  EXPECT_FLOAT_EQ(192.0f, result_f.alpha);
}

TEST(PreMultiplyTest, UnPreMultiplyColorFullyTransparent) {
  FX_ABGR_STRUCT<uint8_t> result = UnPreMultiplyColor(
      FX_ABGR_STRUCT<uint8_t>{.alpha = 0, .blue = 0, .green = 0, .red = 0});
  EXPECT_EQ(0, result.alpha);
  EXPECT_EQ(0, result.blue);
  EXPECT_EQ(0, result.green);
  EXPECT_EQ(0, result.red);

  FX_RGBA_STRUCT<float> result_f = UnPreMultiplyColor(
      FX_RGBA_STRUCT<float>{.red = 0, .green = 0, .blue = 0, .alpha = 0});
  EXPECT_FLOAT_EQ(0.0f, result_f.red);
  EXPECT_FLOAT_EQ(0.0f, result_f.green);
  EXPECT_FLOAT_EQ(0.0f, result_f.blue);
  EXPECT_FLOAT_EQ(0.0f, result_f.alpha);
}

TEST(PreMultiplyTest, UnPreMultiplyColorFullyOpaque) {
  FX_ABGR_STRUCT<uint8_t> result = UnPreMultiplyColor(FX_ABGR_STRUCT<uint8_t>{
      .alpha = 255, .blue = 0, .green = 255, .red = 100});
  EXPECT_EQ(255, result.alpha);
  EXPECT_EQ(0, result.blue);
  EXPECT_EQ(255, result.green);
  EXPECT_EQ(100, result.red);

  FX_RGBA_STRUCT<float> result_f = UnPreMultiplyColor(
      FX_RGBA_STRUCT<float>{.red = 100, .green = 255, .blue = 0, .alpha = 255});
  EXPECT_FLOAT_EQ(100.0f, result_f.red);
  EXPECT_FLOAT_EQ(255.0f, result_f.green);
  EXPECT_FLOAT_EQ(0.0f, result_f.blue);
  EXPECT_FLOAT_EQ(255.0f, result_f.alpha);
}
#endif  // defined(PDF_USE_SKIA)
