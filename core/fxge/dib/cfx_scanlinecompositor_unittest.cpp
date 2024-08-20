// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/dib/cfx_scanlinecompositor.h"

#include <stdint.h>

#include <array>

#include "core/fxcrt/span.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/dib/fx_dib.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::ElementsAreArray;

namespace {

constexpr FX_BGRA_STRUCT<uint8_t> kDestScan[] = {
    {.blue = 255, .green = 100, .red = 0, .alpha = 0},
    {.blue = 255, .green = 100, .red = 0, .alpha = 0},
    {.blue = 255, .green = 100, .red = 0, .alpha = 255},
    {.blue = 255, .green = 100, .red = 0, .alpha = 255},
    {.blue = 255, .green = 100, .red = 0, .alpha = 100},
    {.blue = 255, .green = 100, .red = 0, .alpha = 100},
    {.blue = 255, .green = 100, .red = 0, .alpha = 200},
    {.blue = 255, .green = 100, .red = 0, .alpha = 200},
};
constexpr FX_BGRA_STRUCT<uint8_t> kSrcScan1[] = {
    {.blue = 255, .green = 100, .red = 0, .alpha = 0},
    {.blue = 255, .green = 100, .red = 0, .alpha = 255},
    {.blue = 255, .green = 100, .red = 0, .alpha = 0},
    {.blue = 255, .green = 100, .red = 0, .alpha = 255},
    {.blue = 255, .green = 100, .red = 0, .alpha = 100},
    {.blue = 255, .green = 100, .red = 0, .alpha = 200},
    {.blue = 255, .green = 100, .red = 0, .alpha = 100},
    {.blue = 255, .green = 100, .red = 0, .alpha = 200},
};
constexpr FX_BGRA_STRUCT<uint8_t> kSrcScan2[] = {
    {.blue = 100, .green = 0, .red = 255, .alpha = 0},
    {.blue = 100, .green = 0, .red = 255, .alpha = 255},
    {.blue = 100, .green = 0, .red = 255, .alpha = 0},
    {.blue = 100, .green = 0, .red = 255, .alpha = 255},
    {.blue = 100, .green = 0, .red = 255, .alpha = 100},
    {.blue = 100, .green = 0, .red = 255, .alpha = 200},
    {.blue = 100, .green = 0, .red = 255, .alpha = 100},
    {.blue = 100, .green = 0, .red = 255, .alpha = 200},
};
constexpr FX_BGRA_STRUCT<uint8_t> kSrcScan3[] = {
    {.blue = 0, .green = 255, .red = 100, .alpha = 0},
    {.blue = 0, .green = 255, .red = 100, .alpha = 255},
    {.blue = 0, .green = 255, .red = 100, .alpha = 0},
    {.blue = 0, .green = 255, .red = 100, .alpha = 255},
    {.blue = 0, .green = 255, .red = 100, .alpha = 100},
    {.blue = 0, .green = 255, .red = 100, .alpha = 200},
    {.blue = 0, .green = 255, .red = 100, .alpha = 100},
    {.blue = 0, .green = 255, .red = 100, .alpha = 200},
};

void RunTest(CFX_ScanlineCompositor& compositor,
             pdfium::span<const FX_BGRA_STRUCT<uint8_t>> src_span,
             pdfium::span<const FX_BGRA_STRUCT<uint8_t>> expectations) {
  std::array<FX_BGRA_STRUCT<uint8_t>, 8> dest_scan;
  fxcrt::Copy(kDestScan, dest_scan);
  compositor.CompositeRgbBitmapLine(pdfium::as_writable_byte_span(dest_scan),
                                    pdfium::as_bytes(src_span),
                                    dest_scan.size(), {});
  EXPECT_THAT(dest_scan, ElementsAreArray(expectations));
}

}  // namespace

inline bool operator==(const FX_BGRA_STRUCT<uint8_t>& lhs,
                       const FX_BGRA_STRUCT<uint8_t>& rhs) {
  return lhs.blue == rhs.blue && lhs.green == rhs.green && lhs.red == rhs.red &&
         lhs.alpha == rhs.alpha;
}

TEST(ScanlineCompositorTest, CompositeRgbBitmapLineBgraNormal) {
  CFX_ScanlineCompositor compositor;
  ASSERT_TRUE(compositor.Init(/*dest_format=*/FXDIB_Format::kBgra,
                              /*src_format=*/FXDIB_Format::kBgra,
                              /*src_palette=*/{},
                              /*mask_color=*/0,
                              /*blend_type=*/BlendMode::kNormal,
                              /*bRgbByteOrder=*/false));

  constexpr FX_BGRA_STRUCT<uint8_t> kExpectations1[] = {
      {.blue = 255, .green = 100, .red = 0, .alpha = 0},
      {.blue = 255, .green = 100, .red = 0, .alpha = 255},
      {.blue = 255, .green = 100, .red = 0, .alpha = 255},
      {.blue = 255, .green = 100, .red = 0, .alpha = 255},
      {.blue = 255, .green = 100, .red = 0, .alpha = 161},
      {.blue = 255, .green = 100, .red = 0, .alpha = 222},
      {.blue = 255, .green = 100, .red = 0, .alpha = 222},
      {.blue = 255, .green = 100, .red = 0, .alpha = 244},
  };
  RunTest(compositor, kSrcScan1, kExpectations1);
  constexpr FX_BGRA_STRUCT<uint8_t> kExpectations2[] = {
      {.blue = 100, .green = 0, .red = 255, .alpha = 0},
      {.blue = 100, .green = 0, .red = 255, .alpha = 255},
      {.blue = 255, .green = 100, .red = 0, .alpha = 255},
      {.blue = 100, .green = 0, .red = 255, .alpha = 255},
      {.blue = 158, .green = 38, .red = 158, .alpha = 161},
      {.blue = 115, .green = 10, .red = 229, .alpha = 222},
      {.blue = 185, .green = 55, .red = 114, .alpha = 222},
      {.blue = 127, .green = 18, .red = 209, .alpha = 244},
  };
  RunTest(compositor, kSrcScan2, kExpectations2);
  constexpr FX_BGRA_STRUCT<uint8_t> kExpectations3[] = {
      {.blue = 0, .green = 255, .red = 100, .alpha = 0},
      {.blue = 0, .green = 255, .red = 100, .alpha = 255},
      {.blue = 255, .green = 100, .red = 0, .alpha = 255},
      {.blue = 0, .green = 255, .red = 100, .alpha = 255},
      {.blue = 97, .green = 196, .red = 61, .alpha = 161},
      {.blue = 26, .green = 239, .red = 89, .alpha = 222},
      {.blue = 141, .green = 169, .red = 44, .alpha = 222},
      {.blue = 46, .green = 227, .red = 81, .alpha = 244},
  };
  RunTest(compositor, kSrcScan3, kExpectations3);
}

TEST(ScanlineCompositorTest, CompositeRgbBitmapLineBgraDarken) {
  CFX_ScanlineCompositor compositor;
  ASSERT_TRUE(compositor.Init(/*dest_format=*/FXDIB_Format::kBgra,
                              /*src_format=*/FXDIB_Format::kBgra,
                              /*src_palette=*/{},
                              /*mask_color=*/0,
                              /*blend_type=*/BlendMode::kDarken,
                              /*bRgbByteOrder=*/false));

  constexpr FX_BGRA_STRUCT<uint8_t> kExpectations1[] = {
      {.blue = 255, .green = 100, .red = 0, .alpha = 0},
      {.blue = 255, .green = 100, .red = 0, .alpha = 255},
      {.blue = 255, .green = 100, .red = 0, .alpha = 255},
      {.blue = 255, .green = 100, .red = 0, .alpha = 255},
      {.blue = 255, .green = 100, .red = 0, .alpha = 161},
      {.blue = 255, .green = 100, .red = 0, .alpha = 222},
      {.blue = 255, .green = 100, .red = 0, .alpha = 222},
      {.blue = 255, .green = 100, .red = 0, .alpha = 244},
  };
  RunTest(compositor, kSrcScan1, kExpectations1);
  constexpr FX_BGRA_STRUCT<uint8_t> kExpectations2[] = {
      {.blue = 100, .green = 0, .red = 255, .alpha = 0},
      {.blue = 100, .green = 0, .red = 255, .alpha = 255},
      {.blue = 255, .green = 100, .red = 0, .alpha = 255},
      {.blue = 100, .green = 0, .red = 0, .alpha = 255},
      {.blue = 158, .green = 38, .red = 96, .alpha = 161},
      {.blue = 115, .green = 10, .red = 139, .alpha = 222},
      {.blue = 185, .green = 55, .red = 24, .alpha = 222},
      {.blue = 127, .green = 18, .red = 45, .alpha = 244},
  };
  RunTest(compositor, kSrcScan2, kExpectations2);
  constexpr FX_BGRA_STRUCT<uint8_t> kExpectations3[] = {
      {.blue = 0, .green = 255, .red = 100, .alpha = 0},
      {.blue = 0, .green = 255, .red = 100, .alpha = 255},
      {.blue = 255, .green = 100, .red = 0, .alpha = 255},
      {.blue = 0, .green = 100, .red = 0, .alpha = 255},
      {.blue = 97, .green = 158, .red = 37, .alpha = 161},
      {.blue = 26, .green = 184, .red = 53, .alpha = 222},
      {.blue = 141, .green = 114, .red = 9, .alpha = 222},
      {.blue = 46, .green = 127, .red = 17, .alpha = 244},
  };
  RunTest(compositor, kSrcScan3, kExpectations3);
}

TEST(ScanlineCompositorTest, CompositeRgbBitmapLineBgraHue) {
  CFX_ScanlineCompositor compositor;
  ASSERT_TRUE(compositor.Init(/*dest_format=*/FXDIB_Format::kBgra,
                              /*src_format=*/FXDIB_Format::kBgra,
                              /*src_palette=*/{},
                              /*mask_color=*/0,
                              /*blend_type=*/BlendMode::kHue,
                              /*bRgbByteOrder=*/false));

  constexpr FX_BGRA_STRUCT<uint8_t> kExpectations1[] = {
      {.blue = 255, .green = 100, .red = 0, .alpha = 0},
      {.blue = 255, .green = 100, .red = 0, .alpha = 255},
      {.blue = 255, .green = 100, .red = 0, .alpha = 255},
      {.blue = 255, .green = 100, .red = 0, .alpha = 255},
      {.blue = 255, .green = 100, .red = 0, .alpha = 161},
      {.blue = 255, .green = 100, .red = 0, .alpha = 222},
      {.blue = 255, .green = 100, .red = 0, .alpha = 222},
      {.blue = 255, .green = 100, .red = 0, .alpha = 244},
  };
  RunTest(compositor, kSrcScan1, kExpectations1);
  constexpr FX_BGRA_STRUCT<uint8_t> kExpectations2[] = {
      {.blue = 100, .green = 0, .red = 255, .alpha = 0},
      {.blue = 100, .green = 0, .red = 255, .alpha = 255},
      {.blue = 255, .green = 100, .red = 0, .alpha = 255},
      {.blue = 100, .green = 0, .red = 255, .alpha = 255},
      {.blue = 158, .green = 38, .red = 158, .alpha = 161},
      {.blue = 115, .green = 10, .red = 229, .alpha = 222},
      {.blue = 185, .green = 55, .red = 114, .alpha = 222},
      {.blue = 127, .green = 18, .red = 209, .alpha = 244},
  };
  RunTest(compositor, kSrcScan2, kExpectations2);
  constexpr FX_BGRA_STRUCT<uint8_t> kExpectations3[] = {
      {.blue = 0, .green = 255, .red = 100, .alpha = 0},
      {.blue = 0, .green = 255, .red = 100, .alpha = 255},
      {.blue = 255, .green = 100, .red = 0, .alpha = 255},
      {.blue = 0, .green = 123, .red = 49, .alpha = 255},
      {.blue = 97, .green = 163, .red = 49, .alpha = 161},
      {.blue = 26, .green = 192, .red = 71, .alpha = 222},
      {.blue = 141, .green = 122, .red = 26, .alpha = 222},
      {.blue = 46, .green = 141, .red = 49, .alpha = 244},
  };
  RunTest(compositor, kSrcScan3, kExpectations3);
}
