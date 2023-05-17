// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/utf16.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace pdfium {

static_assert(kSurrogateMask == 0x3ff);
static_assert(kMaximumSupplementaryCodePoint == 0x10ffff);
static_assert(kMaximumHighSurrogateCodeUnit == 0xdbff);
static_assert(kMinimumLowSurrogateCodeUnit == 0xdc00);
static_assert(kMaximumLowSurrogateCodeUnit == 0xdfff);

static_assert(!IsSupplementary(0xffff));
static_assert(IsSupplementary(0x10000));
static_assert(IsSupplementary(0x10ffff));
static_assert(!IsSupplementary(0x110000));

static_assert(!IsHighSurrogate(0xd7ff));
static_assert(IsHighSurrogate(0xd800));
static_assert(IsHighSurrogate(0xdbff));
static_assert(!IsHighSurrogate(0xdc00));

static_assert(!IsLowSurrogate(0xdbff));
static_assert(IsLowSurrogate(0xdc00));
static_assert(IsLowSurrogate(0xdfff));
static_assert(!IsLowSurrogate(0xe000));

static_assert(SurrogatePair(0xd800, 0xdc00).high() == 0xd800);
static_assert(SurrogatePair(0xd800, 0xdc00).low() == 0xdc00);
static_assert(SurrogatePair(0xd800, 0xdc00).ToCodePoint() == 0x10000);

static_assert(SurrogatePair(0xdbff, 0xdfff).high() == 0xdbff);
static_assert(SurrogatePair(0xdbff, 0xdfff).low() == 0xdfff);
static_assert(SurrogatePair(0xdbff, 0xdfff).ToCodePoint() == 0x10ffff);

static_assert(SurrogatePair(0x10000).high() == 0xd800);
static_assert(SurrogatePair(0x10000).low() == 0xdc00);
static_assert(SurrogatePair(0x10000).ToCodePoint() == 0x10000);

static_assert(SurrogatePair(0x10ffff).high() == 0xdbff);
static_assert(SurrogatePair(0x10ffff).low() == 0xdfff);
static_assert(SurrogatePair(0x10ffff).ToCodePoint() == 0x10ffff);

TEST(SurrogatePairTest, RoundTrip) {
  for (char32_t code_point = kMinimumSupplementaryCodePoint;
       code_point <= kMaximumSupplementaryCodePoint; ++code_point) {
    SurrogatePair from_code_point(code_point);
    EXPECT_EQ(code_point, from_code_point.ToCodePoint());

    SurrogatePair from_pair(from_code_point.high(), from_code_point.low());
    EXPECT_EQ(from_code_point.high(), from_pair.high());
    EXPECT_EQ(from_code_point.low(), from_pair.low());
    EXPECT_EQ(code_point, from_pair.ToCodePoint());
  }
}

}  // namespace pdfium
