// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/code_point_view.h"

#include <string>

#include "build/build_config.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

using ::pdfium::CodePointView;

std::u32string Materialize(CodePointView view) {
  std::u32string materialized;
  for (char32_t code_point : view) {
    materialized += code_point;
  }
  return materialized;
}

}  // namespace

TEST(CodePointViewTest, Empty) {
  EXPECT_EQ(U"", Materialize(CodePointView(L"")));
}

TEST(CodePointViewTest, Basic) {
  EXPECT_EQ(U"(\u0080\uffff)", Materialize(CodePointView(L"(\u0080\uffff)")));
}

TEST(CodePointViewTest, Supplementary) {
  EXPECT_EQ(U"(🎨)", Materialize(CodePointView(L"(🎨)")));
}

TEST(CodePointViewTest, UnpairedHighSurrogate) {
  EXPECT_EQ(U"\xd800", Materialize(CodePointView(L"\xd800")));
}

TEST(CodePointViewTest, UnpairedLowSurrogate) {
  EXPECT_EQ(U"\xdc00", Materialize(CodePointView(L"\xdc00")));
}

#if defined(WCHAR_T_IS_16_BIT)
TEST(CodePointViewTest, SurrogateErrorRecovery) {
  EXPECT_EQ(U"(\xd800)", Materialize(CodePointView(L"(\xd800)"))) << "High";
  EXPECT_EQ(U"(\xdc00)", Materialize(CodePointView(L"(\xdc00)"))) << "Low";
  EXPECT_EQ(U"(\xd800🎨)", Materialize(CodePointView(L"(\xd800\xd83c\xdfa8)")))
      << "High-high";
  EXPECT_EQ(U"(🎨\xdc00)", Materialize(CodePointView(L"(\xd83c\xdfa8\xdc00)")))
      << "Low-low";
}
#endif  // defined(WCHAR_T_IS_16_BIT)
