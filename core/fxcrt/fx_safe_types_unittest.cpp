// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>

#include "core/fxcrt/fx_safe_types.h"
#include "testing/gtest/include/gtest/gtest.h"

// PDFium relies on safe types handling the --2147483648 boundary
// condition without overflow.
TEST(FXSafeTypes, UnaryMinus) {
  FX_SAFE_INT32 safe_val = std::numeric_limits<int32_t>::min();
  EXPECT_TRUE(safe_val.IsValid());
  EXPECT_FALSE((-safe_val).IsValid());
}

TEST(FXSafeTypes, SubtractFromZero) {
  FX_SAFE_INT32 safe_val = std::numeric_limits<int32_t>::min();
  EXPECT_TRUE(safe_val.IsValid());
  EXPECT_FALSE((0 - safe_val).IsValid());
}
