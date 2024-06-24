// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/stl_util.h"

#include <stdint.h>

#include <array>
#include <vector>

#include "core/fxcrt/span.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(fxcrt, FillCArray) {
  uint32_t buf[4];
  fxcrt::Fill(buf, 0x01020304u);
  for (const auto b : buf) {
    EXPECT_EQ(b, 0x01020304u);
  }
}

TEST(fxcrt, FillStdArray) {
  std::array<uint16_t, 10> buf;
  fxcrt::Fill(buf, 0x0102u);
  for (const auto b : buf) {
    EXPECT_EQ(b, 0x0102u);
  }
}

TEST(fxcrt, FillStdVector) {
  std::vector<uint8_t> buf(15);
  fxcrt::Fill(buf, 0x32u);
  for (const auto b : buf) {
    EXPECT_EQ(b, 0x32u);
  }
}

TEST(fxcrt, FillSpan) {
  float buf[12];
  auto buf_span = pdfium::make_span(buf);
  fxcrt::Fill(buf_span, 123.0f);
  for (const auto b : buf) {
    EXPECT_EQ(b, 123.0f);
  }
}

TEST(fxcrt, CopyCArray) {
  uint32_t dst[4];
  {
    uint32_t buf[4];
    fxcrt::Fill(buf, 0x01020304u);
    fxcrt::Copy(buf, dst);
  }
  for (const auto b : dst) {
    EXPECT_EQ(b, 0x01020304u);
  }
}

TEST(fxcrt, CopyStdArray) {
  uint16_t dst[10];
  {
    std::array<uint16_t, 10> buf;
    fxcrt::Fill(buf, 0x0102u);
    fxcrt::Copy(buf, dst);
  }
  for (const auto b : dst) {
    EXPECT_EQ(b, 0x0102u);
  }
}

TEST(fxcrt, CopyStdVector) {
  uint8_t dst[15];
  {
    std::vector<uint8_t> buf(15);
    fxcrt::Fill(buf, 0x32u);
    fxcrt::Copy(buf, dst);
  }
  for (const auto b : dst) {
    EXPECT_EQ(b, 0x32u);
  }
}

TEST(fxcrt, CopySpan) {
  float dst[12];
  {
    float buf[12];
    auto buf_span = pdfium::make_span(buf);
    fxcrt::Fill(buf_span, 123.0f);
    fxcrt::Copy(buf_span, dst);
  }
  for (const auto b : dst) {
    EXPECT_EQ(b, 123.0f);
  }
}
