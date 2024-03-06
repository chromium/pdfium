// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/css/cfx_cssdeclaration.h"

#include <optional>

#include "testing/gtest/include/gtest/gtest.h"

TEST(CFX_CSSDeclarationTest, HexEncodingParsing) {
  std::optional<FX_ARGB> maybe_color;

  // Length value invalid.
  EXPECT_FALSE(CFX_CSSDeclaration::ParseCSSColor(L"#00"));
  EXPECT_FALSE(CFX_CSSDeclaration::ParseCSSColor(L"#0000"));
  EXPECT_FALSE(CFX_CSSDeclaration::ParseCSSColor(L"#0000000"));

  // Invalid characters
  maybe_color = CFX_CSSDeclaration::ParseCSSColor(L"#zxytlm");
  ASSERT_TRUE(maybe_color.has_value());
  EXPECT_EQ(0, FXARGB_R(maybe_color.value()));
  EXPECT_EQ(0, FXARGB_G(maybe_color.value()));
  EXPECT_EQ(0, FXARGB_B(maybe_color.value()));

  maybe_color = CFX_CSSDeclaration::ParseCSSColor(L"#000");
  ASSERT_TRUE(maybe_color.has_value());
  EXPECT_EQ(0, FXARGB_R(maybe_color.value()));
  EXPECT_EQ(0, FXARGB_G(maybe_color.value()));
  EXPECT_EQ(0, FXARGB_B(maybe_color.value()));

  maybe_color = CFX_CSSDeclaration::ParseCSSColor(L"#FFF");
  ASSERT_TRUE(maybe_color.has_value());
  EXPECT_EQ(255, FXARGB_R(maybe_color.value()));
  EXPECT_EQ(255, FXARGB_G(maybe_color.value()));
  EXPECT_EQ(255, FXARGB_B(maybe_color.value()));

  maybe_color = CFX_CSSDeclaration::ParseCSSColor(L"#F0F0F0");
  ASSERT_TRUE(maybe_color.has_value());
  EXPECT_EQ(240, FXARGB_R(maybe_color.value()));
  EXPECT_EQ(240, FXARGB_G(maybe_color.value()));
  EXPECT_EQ(240, FXARGB_B(maybe_color.value()));

  // Upper and lower case characters.
  maybe_color = CFX_CSSDeclaration::ParseCSSColor(L"#1b2F3c");
  ASSERT_TRUE(maybe_color.has_value());
  EXPECT_EQ(27, FXARGB_R(maybe_color.value()));
  EXPECT_EQ(47, FXARGB_G(maybe_color.value()));
  EXPECT_EQ(60, FXARGB_B(maybe_color.value()));
}

TEST(CFX_CSSDeclarationTest, RGBEncodingParsing) {
  std::optional<FX_ARGB> maybe_color;

  // Invalid input for rgb() syntax.
  EXPECT_FALSE(CFX_CSSDeclaration::ParseCSSColor(L"blahblahblah"));

  maybe_color = CFX_CSSDeclaration::ParseCSSColor(L"rgb(0, 0, 0)");
  ASSERT_TRUE(maybe_color.has_value());
  EXPECT_EQ(0, FXARGB_R(maybe_color.value()));
  EXPECT_EQ(0, FXARGB_G(maybe_color.value()));
  EXPECT_EQ(0, FXARGB_B(maybe_color.value()));

  maybe_color = CFX_CSSDeclaration::ParseCSSColor(L"rgb(128,255,48)");
  ASSERT_TRUE(maybe_color.has_value());
  EXPECT_EQ(128, FXARGB_R(maybe_color.value()));
  EXPECT_EQ(255, FXARGB_G(maybe_color.value()));
  EXPECT_EQ(48, FXARGB_B(maybe_color.value()));
}
