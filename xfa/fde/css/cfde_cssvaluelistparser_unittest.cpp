// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssvaluelistparser.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

TEST(CFDE_CSSValueListParser, rgb_short) {
  FDE_CSSPrimitiveType type;
  const FX_WCHAR* start;
  int32_t len;

  auto parser = pdfium::MakeUnique<CFDE_CSSValueListParser>(L"#abc", 4, L' ');
  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::RGB, type);
  EXPECT_EQ(L"#abc", CFX_WideString(start, len));
  EXPECT_FALSE(parser->NextValue(type, start, len));

  parser = pdfium::MakeUnique<CFDE_CSSValueListParser>(L"#abcdef", 7, L' ');
  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::RGB, type);
  EXPECT_EQ(L"#abcdef", CFX_WideString(start, len));
  EXPECT_FALSE(parser->NextValue(type, start, len));

  parser =
      pdfium::MakeUnique<CFDE_CSSValueListParser>(L"rgb(1, 255, 4)", 14, L' ');
  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::RGB, type);
  EXPECT_EQ(L"rgb(1, 255, 4)", CFX_WideString(start, len));

  parser =
      pdfium::MakeUnique<CFDE_CSSValueListParser>(L"#abcdefghij", 11, L' ');
  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::Unknown, type);
  EXPECT_EQ(L"#abcdefghij", CFX_WideString(start, len));
  EXPECT_FALSE(parser->NextValue(type, start, len));
}

TEST(CFDE_CSSValueListParser, number_parsing) {
  FDE_CSSPrimitiveType type;
  const FX_WCHAR* start;
  int32_t len;

  auto parser = pdfium::MakeUnique<CFDE_CSSValueListParser>(L"1234", 4, L' ');
  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::Number, type);
  EXPECT_EQ(L"1234", CFX_WideString(start, len));

  parser = pdfium::MakeUnique<CFDE_CSSValueListParser>(L"-1234", 5, L' ');
  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::Number, type);
  EXPECT_EQ(L"-1234", CFX_WideString(start, len));

  parser = pdfium::MakeUnique<CFDE_CSSValueListParser>(L"+1234", 5, L' ');
  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::Number, type);
  EXPECT_EQ(L"+1234", CFX_WideString(start, len));

  parser = pdfium::MakeUnique<CFDE_CSSValueListParser>(L".1234", 5, L' ');
  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::Number, type);
  EXPECT_EQ(L".1234", CFX_WideString(start, len));

  parser = pdfium::MakeUnique<CFDE_CSSValueListParser>(L"4321.1234", 9, L' ');
  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::Number, type);
  EXPECT_EQ(L"4321.1234", CFX_WideString(start, len));

  // TODO(dsinclair): These should probably fail but currently don't.
  parser = pdfium::MakeUnique<CFDE_CSSValueListParser>(L"4321.12.34", 10, L' ');
  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::Number, type);
  EXPECT_EQ(L"4321.12.34", CFX_WideString(start, len));

  parser = pdfium::MakeUnique<CFDE_CSSValueListParser>(L"43a1.12.34", 10, L' ');
  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::Number, type);
  EXPECT_EQ(L"43a1.12.34", CFX_WideString(start, len));
}

TEST(CFDE_CSSValueListParser, string_parsing) {
  FDE_CSSPrimitiveType type;
  const FX_WCHAR* start;
  int32_t len;

  auto parser =
      pdfium::MakeUnique<CFDE_CSSValueListParser>(L"'string'", 8, L' ');
  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::String, type);
  EXPECT_EQ(L"string", CFX_WideString(start, len));

  parser = pdfium::MakeUnique<CFDE_CSSValueListParser>(L"\"another string\"",
                                                       16, L' ');
  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::String, type);
  EXPECT_EQ(L"another string", CFX_WideString(start, len));

  parser = pdfium::MakeUnique<CFDE_CSSValueListParser>(L"standalone", 10, L' ');
  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::String, type);
  EXPECT_EQ(L"standalone", CFX_WideString(start, len));
}

TEST(CFDE_CSSValueListParser, multiparsing) {
  FDE_CSSPrimitiveType type;
  const FX_WCHAR* start;
  int32_t len;

  auto parser =
      pdfium::MakeUnique<CFDE_CSSValueListParser>(L"1, 2, 3", 7, L',');
  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::Number, type);
  EXPECT_EQ(L"1", CFX_WideString(start, len));

  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::Number, type);
  EXPECT_EQ(L"2", CFX_WideString(start, len));

  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::Number, type);
  EXPECT_EQ(L"3", CFX_WideString(start, len));

  EXPECT_FALSE(parser->NextValue(type, start, len));

  parser = pdfium::MakeUnique<CFDE_CSSValueListParser>(
      L"'str', rgb(1, 2, 3), 4", 22, L',');
  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::String, type);
  EXPECT_EQ(L"str", CFX_WideString(start, len));

  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::RGB, type);
  EXPECT_EQ(L"rgb(1, 2, 3)", CFX_WideString(start, len));

  EXPECT_TRUE(parser->NextValue(type, start, len));
  EXPECT_EQ(FDE_CSSPrimitiveType::Number, type);
  EXPECT_EQ(L"4", CFX_WideString(start, len));
}
