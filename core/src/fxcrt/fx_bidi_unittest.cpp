// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/include/fxcrt/fx_bidi.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

const FX_WCHAR kNeutralChar = 32;
const FX_WCHAR kLeftChar = 65;
const FX_WCHAR kRightChar = 1424;

}  // namespace

TEST(fxcrt, BidiCharEmpty) {
  int32_t start = -1;
  int32_t count = -1;
  CFX_BidiChar bidi;
  CFX_BidiChar::Direction dir = bidi.GetBidiInfo(nullptr, nullptr);
  EXPECT_EQ(CFX_BidiChar::NEUTRAL, dir);

  dir = bidi.GetBidiInfo(&start, nullptr);
  EXPECT_EQ(CFX_BidiChar::NEUTRAL, dir);
  EXPECT_EQ(0, start);

  dir = bidi.GetBidiInfo(nullptr, &count);
  EXPECT_EQ(CFX_BidiChar::NEUTRAL, dir);
  EXPECT_EQ(0, count);

  start = -1;
  count = -1;
  dir = bidi.GetBidiInfo(&start, &count);
  EXPECT_EQ(CFX_BidiChar::NEUTRAL, dir);
  EXPECT_EQ(0, start);
  EXPECT_EQ(0, count);

  EXPECT_FALSE(bidi.EndChar());
}

TEST(fxcrt, BidiCharLeft) {
  int32_t start = -1;
  int32_t count = -1;
  CFX_BidiChar bidi;

  EXPECT_TRUE(bidi.AppendChar(kLeftChar));
  CFX_BidiChar::Direction dir = bidi.GetBidiInfo(&start, &count);
  EXPECT_EQ(0, start);
  EXPECT_EQ(0, count);

  EXPECT_FALSE(bidi.AppendChar(kLeftChar));
  EXPECT_FALSE(bidi.AppendChar(kLeftChar));

  dir = bidi.GetBidiInfo(&start, &count);
  EXPECT_EQ(CFX_BidiChar::NEUTRAL, dir);
  EXPECT_EQ(0, start);
  EXPECT_EQ(0, count);

  EXPECT_TRUE(bidi.EndChar());
  dir = bidi.GetBidiInfo(&start, &count);
  EXPECT_EQ(CFX_BidiChar::LEFT, dir);
  EXPECT_EQ(0, start);
  EXPECT_EQ(3, count);

  EXPECT_FALSE(bidi.EndChar());
}

TEST(fxcrt, BidiCharLeftNeutralRight) {
  int32_t start = -1;
  int32_t count = -1;
  CFX_BidiChar bidi;

  EXPECT_TRUE(bidi.AppendChar(kLeftChar));
  CFX_BidiChar::Direction dir = bidi.GetBidiInfo(&start, &count);
  EXPECT_EQ(0, start);
  EXPECT_EQ(0, count);

  EXPECT_FALSE(bidi.AppendChar(kLeftChar));
  EXPECT_FALSE(bidi.AppendChar(kLeftChar));
  EXPECT_TRUE(bidi.AppendChar(kNeutralChar));
  dir = bidi.GetBidiInfo(&start, &count);
  EXPECT_EQ(0, start);
  EXPECT_EQ(3, count);

  EXPECT_FALSE(bidi.AppendChar(kNeutralChar));
  EXPECT_FALSE(bidi.AppendChar(kNeutralChar));
  EXPECT_FALSE(bidi.AppendChar(kNeutralChar));
  EXPECT_TRUE(bidi.AppendChar(kRightChar));
  dir = bidi.GetBidiInfo(&start, &count);
  EXPECT_EQ(CFX_BidiChar::NEUTRAL, dir);
  EXPECT_EQ(3, start);
  EXPECT_EQ(4, count);

  EXPECT_TRUE(bidi.EndChar());
  dir = bidi.GetBidiInfo(&start, &count);
  EXPECT_EQ(CFX_BidiChar::RIGHT, dir);
  EXPECT_EQ(7, start);
  EXPECT_EQ(1, count);

  EXPECT_FALSE(bidi.EndChar());
}

TEST(fxcrt, BidiCharLeftRightLeft) {
  int32_t start = -1;
  int32_t count = -1;
  CFX_BidiChar bidi;

  EXPECT_TRUE(bidi.AppendChar(kLeftChar));
  CFX_BidiChar::Direction dir = bidi.GetBidiInfo(&start, &count);
  EXPECT_EQ(0, start);
  EXPECT_EQ(0, count);

  EXPECT_FALSE(bidi.AppendChar(kLeftChar));
  EXPECT_FALSE(bidi.AppendChar(kLeftChar));
  EXPECT_TRUE(bidi.AppendChar(kRightChar));
  dir = bidi.GetBidiInfo(&start, &count);
  EXPECT_EQ(0, start);
  EXPECT_EQ(3, count);

  EXPECT_FALSE(bidi.AppendChar(kRightChar));
  EXPECT_FALSE(bidi.AppendChar(kRightChar));
  EXPECT_FALSE(bidi.AppendChar(kRightChar));
  EXPECT_TRUE(bidi.AppendChar(kLeftChar));
  dir = bidi.GetBidiInfo(&start, &count);
  EXPECT_EQ(CFX_BidiChar::RIGHT, dir);
  EXPECT_EQ(3, start);
  EXPECT_EQ(4, count);

  EXPECT_TRUE(bidi.EndChar());
  dir = bidi.GetBidiInfo(&start, &count);
  EXPECT_EQ(CFX_BidiChar::LEFT, dir);
  EXPECT_EQ(7, start);
  EXPECT_EQ(1, count);

  EXPECT_FALSE(bidi.EndChar());
}
