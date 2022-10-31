// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/mask.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace fxcrt {
namespace {

enum class Privilege : uint8_t {
  kPriv1 = 1 << 0,
  kPriv2 = 1 << 1,
  kPriv4 = 1 << 2,
  kPriv8 = 1 << 3,
  kPriv16 = 1 << 4,
  kPriv32 = 1 << 5,
  kPriv64 = 1 << 6,
  kPriv128 = 1 << 7,
};

constexpr Mask<Privilege> kAllMask = {
    Privilege::kPriv1,  Privilege::kPriv2,   Privilege::kPriv4,
    Privilege::kPriv8,  Privilege::kPriv16,  Privilege::kPriv32,
    Privilege::kPriv64, Privilege::kPriv128,
};

}  // namespace

static_assert(sizeof(Mask<Privilege>) == sizeof(Privilege),
              "Mask size must be the same as enum");

TEST(Mask, Empty) {
  constexpr Mask<Privilege> privs;
  EXPECT_EQ(0u, privs.UncheckedValue());
  EXPECT_FALSE(privs & Privilege::kPriv1);
  EXPECT_FALSE(privs & Privilege::kPriv4);
  EXPECT_FALSE(privs & Privilege::kPriv8);
  EXPECT_FALSE(privs & kAllMask);
}

TEST(Mask, FromOne) {
  Mask<Privilege> privs = Privilege::kPriv1;
  EXPECT_EQ(1u, privs.UncheckedValue());
  EXPECT_TRUE(privs & Privilege::kPriv1);
  EXPECT_FALSE(privs & Privilege::kPriv4);
  EXPECT_FALSE(privs & Privilege::kPriv8);
  EXPECT_TRUE(privs & kAllMask);
}

TEST(Mask, FromTwo) {
  // Not adjacent bits, just because.
  Mask<Privilege> privs = {Privilege::kPriv1, Privilege::kPriv8};
  EXPECT_EQ(9u, privs.UncheckedValue());
  EXPECT_TRUE(privs & Privilege::kPriv1);
  EXPECT_FALSE(privs & Privilege::kPriv4);
  EXPECT_TRUE(privs & Privilege::kPriv8);
  EXPECT_TRUE(privs & kAllMask);
}

TEST(Mask, FromThree) {
  Mask<Privilege> privs = {
      Privilege::kPriv1,
      Privilege::kPriv2,
      Privilege::kPriv4,
  };
  EXPECT_EQ(7u, privs.UncheckedValue());
}

TEST(Mask, FromFour) {
  Mask<Privilege> privs = {
      Privilege::kPriv1,
      Privilege::kPriv2,
      Privilege::kPriv4,
      Privilege::kPriv8,
  };
  EXPECT_EQ(15u, privs.UncheckedValue());
}

TEST(Mask, FromFive) {
  Mask<Privilege> privs = {
      Privilege::kPriv1, Privilege::kPriv2,  Privilege::kPriv4,
      Privilege::kPriv8, Privilege::kPriv16,
  };
  EXPECT_EQ(31u, privs.UncheckedValue());
}

TEST(Mask, FromSix) {
  Mask<Privilege> privs = {
      Privilege::kPriv1, Privilege::kPriv2,  Privilege::kPriv4,
      Privilege::kPriv8, Privilege::kPriv16, Privilege::kPriv32,
  };
  EXPECT_EQ(63u, privs.UncheckedValue());
}

TEST(Mask, FromSeven) {
  Mask<Privilege> privs = {
      Privilege::kPriv1,  Privilege::kPriv2,  Privilege::kPriv4,
      Privilege::kPriv8,  Privilege::kPriv16, Privilege::kPriv32,
      Privilege::kPriv64,
  };
  EXPECT_EQ(127u, privs.UncheckedValue());
}

TEST(Mask, FromEight) {
  Mask<Privilege> privs = {
      Privilege::kPriv1,  Privilege::kPriv2,   Privilege::kPriv4,
      Privilege::kPriv8,  Privilege::kPriv16,  Privilege::kPriv32,
      Privilege::kPriv64, Privilege::kPriv128,
  };
  EXPECT_EQ(255u, privs.UncheckedValue());
}

TEST(Mask, FromUnderlying) {
  auto privs = Mask<Privilege>::FromUnderlyingUnchecked(5);
  EXPECT_EQ(5u, privs.UncheckedValue());
  EXPECT_TRUE(privs & Privilege::kPriv1);
  EXPECT_TRUE(privs & Privilege::kPriv4);
  EXPECT_FALSE(privs & Privilege::kPriv8);
}

TEST(Mask, AssignAndEQ) {
  Mask<Privilege> source = {Privilege::kPriv1, Privilege::kPriv8};
  Mask<Privilege> other = Privilege::kPriv1;
  Mask<Privilege> dest;
  dest = source;
  EXPECT_EQ(9u, dest.UncheckedValue());
  EXPECT_EQ(source, dest);
  EXPECT_NE(other, dest);
}

TEST(Mask, OrAndAnd) {
  Mask<Privilege> source = {Privilege::kPriv1, Privilege::kPriv8};
  Mask<Privilege> or_result =
      source | Mask<Privilege>{Privilege::kPriv1, Privilege::kPriv4};
  Mask<Privilege> and_result =
      source & Mask<Privilege>{Privilege::kPriv1, Privilege::kPriv4};
  EXPECT_EQ(13u, or_result.UncheckedValue());
  EXPECT_EQ(1u, and_result.UncheckedValue());
}

TEST(Mask, OrEqualsAndAndEquals) {
  Mask<Privilege> source_or = {Privilege::kPriv1, Privilege::kPriv8};
  Mask<Privilege> source_and = {Privilege::kPriv1, Privilege::kPriv8};
  source_or |= {Privilege::kPriv1, Privilege::kPriv4};
  source_and &= {Privilege::kPriv1, Privilege::kPriv4};
  EXPECT_EQ(13u, source_or.UncheckedValue());
  EXPECT_EQ(1u, source_and.UncheckedValue());
}

TEST(Mask, Clear) {
  Mask<Privilege> source = kAllMask;
  source.Clear({Privilege::kPriv1, Privilege::kPriv4});
  EXPECT_EQ(250u, source.UncheckedValue());
}

TEST(Mask, TestAll) {
  Mask<Privilege> source = {
      Privilege::kPriv1,
      Privilege::kPriv8,
      Privilege::kPriv64,
  };
  Mask<Privilege> passes = {Privilege::kPriv1, Privilege::kPriv64};
  Mask<Privilege> fails = {Privilege::kPriv1, Privilege::kPriv32};
  EXPECT_TRUE(source.TestAll(passes));
  EXPECT_FALSE(source.TestAll(fails));
}

}  // namespace fxcrt
