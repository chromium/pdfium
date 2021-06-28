// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/span_util.h"

#include <vector>

#include "testing/gtest/include/gtest/gtest.h"

TEST(Spanset, Fits) {
  std::vector<char> dst(4, 'B');
  fxcrt::spanset(pdfium::make_span(dst).first(2), 'A');
  EXPECT_EQ(dst[0], 'A');
  EXPECT_EQ(dst[1], 'A');
  EXPECT_EQ(dst[2], 'B');
  EXPECT_EQ(dst[3], 'B');
}

TEST(Spanset, Empty) {
  std::vector<char> dst(4, 'B');
  fxcrt::spanset(fxcrt::Subspan(dst, 4), 'A');
  EXPECT_EQ(dst[0], 'B');
  EXPECT_EQ(dst[1], 'B');
  EXPECT_EQ(dst[2], 'B');
  EXPECT_EQ(dst[3], 'B');
}

TEST(Spancpy, FitsEntirely) {
  std::vector<char> src(4, 'A');
  std::vector<char> dst(4, 'B');
  fxcrt::spancpy(pdfium::make_span(dst), pdfium::make_span(src));
  EXPECT_EQ(dst[0], 'A');
  EXPECT_EQ(dst[1], 'A');
  EXPECT_EQ(dst[2], 'A');
  EXPECT_EQ(dst[3], 'A');
}

TEST(Spancpy, FitsWithin) {
  std::vector<char> src(2, 'A');
  std::vector<char> dst(4, 'B');
  fxcrt::spancpy(fxcrt::Subspan(dst, 1), pdfium::make_span(src));
  EXPECT_EQ(dst[0], 'B');
  EXPECT_EQ(dst[1], 'A');
  EXPECT_EQ(dst[2], 'A');
  EXPECT_EQ(dst[3], 'B');
}

TEST(Spancpy, EmptyCopyWithin) {
  std::vector<char> src(2, 'A');
  std::vector<char> dst(4, 'B');
  fxcrt::spancpy(fxcrt::Subspan(dst, 1), fxcrt::Subspan(src, 2));
  EXPECT_EQ(dst[0], 'B');
  EXPECT_EQ(dst[1], 'B');
  EXPECT_EQ(dst[2], 'B');
  EXPECT_EQ(dst[3], 'B');
}

TEST(Spancpy, EmptyCopyToEmpty) {
  std::vector<char> src(2, 'A');
  std::vector<char> dst(4, 'B');
  fxcrt::spancpy(fxcrt::Subspan(dst, 4), fxcrt::Subspan(src, 2));
  EXPECT_EQ(dst[0], 'B');
  EXPECT_EQ(dst[1], 'B');
  EXPECT_EQ(dst[2], 'B');
  EXPECT_EQ(dst[3], 'B');
}
