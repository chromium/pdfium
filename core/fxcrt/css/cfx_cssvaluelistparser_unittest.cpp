// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_cssvaluelistparser.h"

#include <memory>

#include "core/fxcrt/widestring.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(CFXCSSValueListParserTest, RGBShort) {
  auto parser = std::make_unique<CFX_CSSValueListParser>(L"#abc", L' ');
  auto maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kRGB, maybe_next.value().type);
  EXPECT_EQ(L"#abc", maybe_next.value().string_view);
  EXPECT_FALSE(parser->NextValue());

  parser = std::make_unique<CFX_CSSValueListParser>(L"#abcdef", L' ');
  maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kRGB, maybe_next.value().type);
  EXPECT_EQ(L"#abcdef", maybe_next.value().string_view);

  parser = std::make_unique<CFX_CSSValueListParser>(L"rgb(1, 255, 4)", L' ');
  maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kRGB, maybe_next.value().type);
  EXPECT_EQ(L"rgb(1, 255, 4)", maybe_next.value().string_view);

  parser = std::make_unique<CFX_CSSValueListParser>(L"#abcdefghij", L' ');
  maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kUnknown, maybe_next.value().type);
  EXPECT_EQ(L"#abcdefghij", maybe_next.value().string_view);
  EXPECT_FALSE(parser->NextValue());
}

TEST(CFXCSSValueListParserTest, NumberParsing) {
  auto parser = std::make_unique<CFX_CSSValueListParser>(L"1234", L' ');
  auto maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, maybe_next.value().type);
  EXPECT_EQ(L"1234", maybe_next.value().string_view);

  parser = std::make_unique<CFX_CSSValueListParser>(L"-1234", L' ');
  maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, maybe_next.value().type);
  EXPECT_EQ(L"-1234", maybe_next.value().string_view);

  parser = std::make_unique<CFX_CSSValueListParser>(L"+1234", L' ');
  maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, maybe_next.value().type);
  EXPECT_EQ(L"+1234", maybe_next.value().string_view);

  parser = std::make_unique<CFX_CSSValueListParser>(L".1234", L' ');
  maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, maybe_next.value().type);
  EXPECT_EQ(L".1234", maybe_next.value().string_view);

  parser = std::make_unique<CFX_CSSValueListParser>(L"4321.1234", L' ');
  maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, maybe_next.value().type);
  EXPECT_EQ(L"4321.1234", maybe_next.value().string_view);

  // TODO(dsinclair): These should probably fail but currently don't.
  parser = std::make_unique<CFX_CSSValueListParser>(L"4321.12.34", L' ');
  maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, maybe_next.value().type);
  EXPECT_EQ(L"4321.12.34", maybe_next.value().string_view);

  parser = std::make_unique<CFX_CSSValueListParser>(L"43a1.12.34", L' ');
  maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, maybe_next.value().type);
  EXPECT_EQ(L"43a1.12.34", maybe_next.value().string_view);
}

TEST(CFXCSSValueListParserTest, StringParsing) {
  auto parser = std::make_unique<CFX_CSSValueListParser>(L"'string'", L' ');
  auto maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kString, maybe_next.value().type);
  EXPECT_EQ(L"string", maybe_next.value().string_view);

  parser =
      std::make_unique<CFX_CSSValueListParser>(L"\"another string\"", L' ');
  maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kString, maybe_next.value().type);
  EXPECT_EQ(L"another string", maybe_next.value().string_view);

  parser = std::make_unique<CFX_CSSValueListParser>(L"standalone", L' ');
  maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kString, maybe_next.value().type);
  EXPECT_EQ(L"standalone", maybe_next.value().string_view);
}

TEST(CFXCSSValueListParserTest, MultiParsing) {
  auto parser = std::make_unique<CFX_CSSValueListParser>(L"1, 2, 3", L',');
  auto maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, maybe_next.value().type);
  EXPECT_EQ(L"1", maybe_next.value().string_view);

  maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, maybe_next.value().type);
  EXPECT_EQ(L"2", maybe_next.value().string_view);

  maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, maybe_next.value().type);
  EXPECT_EQ(L"3", maybe_next.value().string_view);

  EXPECT_FALSE(parser->NextValue());

  parser =
      std::make_unique<CFX_CSSValueListParser>(L"'str', rgb(1, 2, 3), 4", L',');
  maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kString, maybe_next.value().type);
  EXPECT_EQ(L"str", maybe_next.value().string_view);

  maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kRGB, maybe_next.value().type);
  EXPECT_EQ(L"rgb(1, 2, 3)", maybe_next.value().string_view);

  maybe_next = parser->NextValue();
  ASSERT_TRUE(maybe_next.has_value());
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, maybe_next.value().type);
  EXPECT_EQ(L"4", maybe_next.value().string_view);
}
