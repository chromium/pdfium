// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_cssvaluelistparser.h"

#include <memory>

#include "core/fxcrt/widestring.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(CFX_CSSValueListParserTest, rgb_short) {
  CFX_CSSValue::PrimitiveType type;
  const wchar_t* start;
  size_t len;

  auto parser = std::make_unique<CFX_CSSValueListParser>(L"#abc", 4, L' ');
  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kRGB, type);
  EXPECT_EQ(L"#abc", WideString(start, len));
  EXPECT_FALSE(parser->NextValue(&type, &start, &len));

  parser = std::make_unique<CFX_CSSValueListParser>(L"#abcdef", 7, L' ');
  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kRGB, type);
  EXPECT_EQ(L"#abcdef", WideString(start, len));
  EXPECT_FALSE(parser->NextValue(&type, &start, &len));

  parser =
      std::make_unique<CFX_CSSValueListParser>(L"rgb(1, 255, 4)", 14, L' ');
  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kRGB, type);
  EXPECT_EQ(L"rgb(1, 255, 4)", WideString(start, len));

  parser = std::make_unique<CFX_CSSValueListParser>(L"#abcdefghij", 11, L' ');
  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kUnknown, type);
  EXPECT_EQ(L"#abcdefghij", WideString(start, len));
  EXPECT_FALSE(parser->NextValue(&type, &start, &len));
}

TEST(CFX_CSSValueListParserTest, number_parsing) {
  CFX_CSSValue::PrimitiveType type;
  const wchar_t* start;
  size_t len;

  auto parser = std::make_unique<CFX_CSSValueListParser>(L"1234", 4, L' ');
  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, type);
  EXPECT_EQ(L"1234", WideString(start, len));

  parser = std::make_unique<CFX_CSSValueListParser>(L"-1234", 5, L' ');
  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, type);
  EXPECT_EQ(L"-1234", WideString(start, len));

  parser = std::make_unique<CFX_CSSValueListParser>(L"+1234", 5, L' ');
  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, type);
  EXPECT_EQ(L"+1234", WideString(start, len));

  parser = std::make_unique<CFX_CSSValueListParser>(L".1234", 5, L' ');
  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, type);
  EXPECT_EQ(L".1234", WideString(start, len));

  parser = std::make_unique<CFX_CSSValueListParser>(L"4321.1234", 9, L' ');
  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, type);
  EXPECT_EQ(L"4321.1234", WideString(start, len));

  // TODO(dsinclair): These should probably fail but currently don't.
  parser = std::make_unique<CFX_CSSValueListParser>(L"4321.12.34", 10, L' ');
  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, type);
  EXPECT_EQ(L"4321.12.34", WideString(start, len));

  parser = std::make_unique<CFX_CSSValueListParser>(L"43a1.12.34", 10, L' ');
  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, type);
  EXPECT_EQ(L"43a1.12.34", WideString(start, len));
}

TEST(CFX_CSSValueListParserTest, string_parsing) {
  CFX_CSSValue::PrimitiveType type;
  const wchar_t* start;
  size_t len;

  auto parser = std::make_unique<CFX_CSSValueListParser>(L"'string'", 8, L' ');
  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kString, type);
  EXPECT_EQ(L"string", WideString(start, len));

  parser =
      std::make_unique<CFX_CSSValueListParser>(L"\"another string\"", 16, L' ');
  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kString, type);
  EXPECT_EQ(L"another string", WideString(start, len));

  parser = std::make_unique<CFX_CSSValueListParser>(L"standalone", 10, L' ');
  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kString, type);
  EXPECT_EQ(L"standalone", WideString(start, len));
}

TEST(CFX_CSSValueListParserTest, multiparsing) {
  CFX_CSSValue::PrimitiveType type;
  const wchar_t* start;
  size_t len;

  auto parser = std::make_unique<CFX_CSSValueListParser>(L"1, 2, 3", 7, L',');
  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, type);
  EXPECT_EQ(L"1", WideString(start, len));

  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, type);
  EXPECT_EQ(L"2", WideString(start, len));

  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, type);
  EXPECT_EQ(L"3", WideString(start, len));

  EXPECT_FALSE(parser->NextValue(&type, &start, &len));

  parser = std::make_unique<CFX_CSSValueListParser>(L"'str', rgb(1, 2, 3), 4",
                                                    22, L',');
  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kString, type);
  EXPECT_EQ(L"str", WideString(start, len));

  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kRGB, type);
  EXPECT_EQ(L"rgb(1, 2, 3)", WideString(start, len));

  EXPECT_TRUE(parser->NextValue(&type, &start, &len));
  EXPECT_EQ(CFX_CSSValue::PrimitiveType::kNumber, type);
  EXPECT_EQ(L"4", WideString(start, len));
}
