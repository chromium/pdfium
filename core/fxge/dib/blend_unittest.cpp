// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/dib/blend.h"

#include "core/fxge/dib/fx_dib.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace fxge {

TEST(Blend, Normal) {
  EXPECT_EQ(0, Blend(BlendMode::kNormal, 0, 0));
  EXPECT_EQ(0, Blend(BlendMode::kNormal, 99, 0));
  EXPECT_EQ(0, Blend(BlendMode::kNormal, 255, 0));
  EXPECT_EQ(99, Blend(BlendMode::kNormal, 0, 99));
  EXPECT_EQ(99, Blend(BlendMode::kNormal, 99, 99));
  EXPECT_EQ(99, Blend(BlendMode::kNormal, 255, 99));
  EXPECT_EQ(255, Blend(BlendMode::kNormal, 0, 255));
  EXPECT_EQ(255, Blend(BlendMode::kNormal, 99, 255));
  EXPECT_EQ(255, Blend(BlendMode::kNormal, 255, 255));
}

TEST(Blend, Multiply) {
  EXPECT_EQ(0, Blend(BlendMode::kMultiply, 0, 0));
  EXPECT_EQ(0, Blend(BlendMode::kMultiply, 99, 0));
  EXPECT_EQ(0, Blend(BlendMode::kMultiply, 255, 0));
  EXPECT_EQ(0, Blend(BlendMode::kMultiply, 0, 99));
  EXPECT_EQ(38, Blend(BlendMode::kMultiply, 99, 99));
  EXPECT_EQ(99, Blend(BlendMode::kMultiply, 255, 99));
  EXPECT_EQ(0, Blend(BlendMode::kMultiply, 0, 255));
  EXPECT_EQ(99, Blend(BlendMode::kMultiply, 99, 255));
  EXPECT_EQ(255, Blend(BlendMode::kMultiply, 255, 255));
}

TEST(Blend, Screen) {
  EXPECT_EQ(0, Blend(BlendMode::kScreen, 0, 0));
  EXPECT_EQ(99, Blend(BlendMode::kScreen, 99, 0));
  EXPECT_EQ(255, Blend(BlendMode::kScreen, 255, 0));
  EXPECT_EQ(99, Blend(BlendMode::kScreen, 0, 99));
  EXPECT_EQ(160, Blend(BlendMode::kScreen, 99, 99));
  EXPECT_EQ(255, Blend(BlendMode::kScreen, 255, 99));
  EXPECT_EQ(255, Blend(BlendMode::kScreen, 0, 255));
  EXPECT_EQ(255, Blend(BlendMode::kScreen, 99, 255));
  EXPECT_EQ(255, Blend(BlendMode::kScreen, 255, 255));
}

TEST(Blend, Overlay) {
  EXPECT_EQ(0, Blend(BlendMode::kOverlay, 0, 0));
  EXPECT_EQ(0, Blend(BlendMode::kOverlay, 99, 0));
  EXPECT_EQ(143, Blend(BlendMode::kOverlay, 199, 0));
  EXPECT_EQ(255, Blend(BlendMode::kOverlay, 255, 0));
  EXPECT_EQ(0, Blend(BlendMode::kOverlay, 0, 99));
  EXPECT_EQ(76, Blend(BlendMode::kOverlay, 99, 99));
  EXPECT_EQ(187, Blend(BlendMode::kOverlay, 199, 99));
  EXPECT_EQ(255, Blend(BlendMode::kOverlay, 255, 99));
  EXPECT_EQ(0, Blend(BlendMode::kOverlay, 0, 199));
  EXPECT_EQ(154, Blend(BlendMode::kOverlay, 99, 199));
  EXPECT_EQ(231, Blend(BlendMode::kOverlay, 199, 199));
  EXPECT_EQ(255, Blend(BlendMode::kOverlay, 255, 199));
  EXPECT_EQ(0, Blend(BlendMode::kOverlay, 0, 255));
  EXPECT_EQ(198, Blend(BlendMode::kOverlay, 99, 255));
  EXPECT_EQ(255, Blend(BlendMode::kOverlay, 199, 255));
  EXPECT_EQ(255, Blend(BlendMode::kOverlay, 255, 255));
}

TEST(Blend, Darken) {
  EXPECT_EQ(0, Blend(BlendMode::kDarken, 0, 0));
  EXPECT_EQ(0, Blend(BlendMode::kDarken, 99, 0));
  EXPECT_EQ(0, Blend(BlendMode::kDarken, 255, 0));
  EXPECT_EQ(0, Blend(BlendMode::kDarken, 0, 99));
  EXPECT_EQ(99, Blend(BlendMode::kDarken, 99, 99));
  EXPECT_EQ(99, Blend(BlendMode::kDarken, 255, 99));
  EXPECT_EQ(0, Blend(BlendMode::kDarken, 0, 255));
  EXPECT_EQ(99, Blend(BlendMode::kDarken, 99, 255));
  EXPECT_EQ(255, Blend(BlendMode::kDarken, 255, 255));
}

TEST(Blend, Lighten) {
  EXPECT_EQ(0, Blend(BlendMode::kLighten, 0, 0));
  EXPECT_EQ(99, Blend(BlendMode::kLighten, 99, 0));
  EXPECT_EQ(255, Blend(BlendMode::kLighten, 255, 0));
  EXPECT_EQ(99, Blend(BlendMode::kLighten, 0, 99));
  EXPECT_EQ(99, Blend(BlendMode::kLighten, 99, 99));
  EXPECT_EQ(255, Blend(BlendMode::kLighten, 255, 99));
  EXPECT_EQ(255, Blend(BlendMode::kLighten, 0, 255));
  EXPECT_EQ(255, Blend(BlendMode::kLighten, 99, 255));
  EXPECT_EQ(255, Blend(BlendMode::kLighten, 255, 255));
}

TEST(Blend, ColorDodge) {
  EXPECT_EQ(0, Blend(BlendMode::kColorDodge, 0, 0));
  EXPECT_EQ(99, Blend(BlendMode::kColorDodge, 99, 0));
  EXPECT_EQ(255, Blend(BlendMode::kColorDodge, 255, 0));
  EXPECT_EQ(0, Blend(BlendMode::kColorDodge, 0, 99));
  EXPECT_EQ(161, Blend(BlendMode::kColorDodge, 99, 99));
  EXPECT_EQ(255, Blend(BlendMode::kColorDodge, 255, 99));
  EXPECT_EQ(255, Blend(BlendMode::kColorDodge, 0, 255));
  EXPECT_EQ(255, Blend(BlendMode::kColorDodge, 99, 255));
  EXPECT_EQ(255, Blend(BlendMode::kColorDodge, 255, 255));
}

TEST(Blend, ColorBurn) {
  EXPECT_EQ(0, Blend(BlendMode::kColorBurn, 0, 0));
  EXPECT_EQ(0, Blend(BlendMode::kColorBurn, 99, 0));
  EXPECT_EQ(0, Blend(BlendMode::kColorBurn, 255, 0));
  EXPECT_EQ(0, Blend(BlendMode::kColorBurn, 0, 99));
  EXPECT_EQ(0, Blend(BlendMode::kColorBurn, 99, 99));
  EXPECT_EQ(255, Blend(BlendMode::kColorBurn, 255, 99));
  EXPECT_EQ(0, Blend(BlendMode::kColorBurn, 0, 255));
  EXPECT_EQ(99, Blend(BlendMode::kColorBurn, 99, 255));
  EXPECT_EQ(255, Blend(BlendMode::kColorBurn, 255, 255));
}

TEST(Blend, HardLight) {
  EXPECT_EQ(0, Blend(BlendMode::kHardLight, 0, 0));
  EXPECT_EQ(0, Blend(BlendMode::kHardLight, 99, 0));
  EXPECT_EQ(0, Blend(BlendMode::kHardLight, 255, 0));
  EXPECT_EQ(0, Blend(BlendMode::kHardLight, 0, 99));
  EXPECT_EQ(76, Blend(BlendMode::kHardLight, 99, 99));
  EXPECT_EQ(198, Blend(BlendMode::kHardLight, 255, 99));
  EXPECT_EQ(143, Blend(BlendMode::kHardLight, 0, 199));
  EXPECT_EQ(187, Blend(BlendMode::kHardLight, 99, 199));
  EXPECT_EQ(255, Blend(BlendMode::kHardLight, 255, 199));
  EXPECT_EQ(255, Blend(BlendMode::kHardLight, 0, 255));
  EXPECT_EQ(255, Blend(BlendMode::kHardLight, 99, 255));
  EXPECT_EQ(255, Blend(BlendMode::kHardLight, 255, 255));
}

TEST(Blend, SoftLight) {
  EXPECT_EQ(0, Blend(BlendMode::kSoftLight, 0, 0));
  EXPECT_EQ(39, Blend(BlendMode::kSoftLight, 99, 0));
  EXPECT_EQ(255, Blend(BlendMode::kSoftLight, 255, 0));
  EXPECT_EQ(0, Blend(BlendMode::kSoftLight, 0, 99));
  EXPECT_EQ(86, Blend(BlendMode::kSoftLight, 99, 99));
  EXPECT_EQ(255, Blend(BlendMode::kSoftLight, 255, 99));
  EXPECT_EQ(0, Blend(BlendMode::kSoftLight, 0, 199));
  EXPECT_EQ(81, Blend(BlendMode::kSoftLight, 47, 199));
  EXPECT_EQ(132, Blend(BlendMode::kSoftLight, 99, 199));
  EXPECT_EQ(255, Blend(BlendMode::kSoftLight, 255, 199));
  EXPECT_EQ(0, Blend(BlendMode::kSoftLight, 0, 255));
  EXPECT_EQ(109, Blend(BlendMode::kSoftLight, 47, 255));
  EXPECT_EQ(159, Blend(BlendMode::kSoftLight, 99, 255));
  EXPECT_EQ(255, Blend(BlendMode::kSoftLight, 255, 255));
}

TEST(Blend, Difference) {
  EXPECT_EQ(0, Blend(BlendMode::kDifference, 0, 0));
  EXPECT_EQ(99, Blend(BlendMode::kDifference, 99, 0));
  EXPECT_EQ(255, Blend(BlendMode::kDifference, 255, 0));
  EXPECT_EQ(99, Blend(BlendMode::kDifference, 0, 99));
  EXPECT_EQ(0, Blend(BlendMode::kDifference, 99, 99));
  EXPECT_EQ(156, Blend(BlendMode::kDifference, 255, 99));
  EXPECT_EQ(255, Blend(BlendMode::kDifference, 0, 255));
  EXPECT_EQ(156, Blend(BlendMode::kDifference, 99, 255));
  EXPECT_EQ(0, Blend(BlendMode::kDifference, 255, 255));
}

TEST(Blend, Exclusion) {
  EXPECT_EQ(0, Blend(BlendMode::kExclusion, 0, 0));
  EXPECT_EQ(99, Blend(BlendMode::kExclusion, 99, 0));
  EXPECT_EQ(255, Blend(BlendMode::kExclusion, 255, 0));
  EXPECT_EQ(99, Blend(BlendMode::kExclusion, 0, 99));
  EXPECT_EQ(122, Blend(BlendMode::kExclusion, 99, 99));
  EXPECT_EQ(156, Blend(BlendMode::kExclusion, 255, 99));
  EXPECT_EQ(255, Blend(BlendMode::kExclusion, 0, 255));
  EXPECT_EQ(156, Blend(BlendMode::kExclusion, 99, 255));
  EXPECT_EQ(0, Blend(BlendMode::kExclusion, 255, 255));
}

}  // namespace fxge
