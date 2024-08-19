// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_simple_parser.h"

#include <iterator>

#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/span.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

TEST(SimpleParserTest, GetWord) {
  static const pdfium::StrFuncTestData test_data[] = {
      // Empty src string.
      STR_IN_OUT_CASE("", ""),
      // Content with whitespaces only.
      STR_IN_OUT_CASE(" \t \0 \n", ""),
      // Content with comments only.
      STR_IN_OUT_CASE("%this is a test case\r\n%2nd line", ""),
      // Mixed whitespaces and comments.
      STR_IN_OUT_CASE(" \t \0%try()%haha\n %another line \aa", ""),
      // Name.
      STR_IN_OUT_CASE("/", ""),
      STR_IN_OUT_CASE("/99", ""),
      STR_IN_OUT_CASE("/99}", "/99"),
      STR_IN_OUT_CASE(" /Tester ", "/Tester"),
      // String.
      STR_IN_OUT_CASE("\t(nice day)!\n ", "(nice day)"),
      // String with nested braces.
      STR_IN_OUT_CASE("\t(It is a (long) day)!\n ", "(It is a (long) day)"),
      // String with escaped chars.
      STR_IN_OUT_CASE("\t(It is a \\(long\\) day!)hi\n ",
                      "(It is a \\(long\\) day!)"),
      // Angle brackets.
      STR_IN_OUT_CASE("<", "<"),
      STR_IN_OUT_CASE(">", ">"),
      // Hex string.
      STR_IN_OUT_CASE(" \n<4545acdfedertt>abc ", "<4545acdfedertt>"),
      STR_IN_OUT_CASE(" \n<4545a<ed>ertt>abc ", "<4545a<ed>"),
      // Dictionary.
      STR_IN_OUT_CASE("<</oc 234 /color 2 3 R>>", "<<"),
      STR_IN_OUT_CASE("\t\t<< /abc>>", "<<"),
      // Parentheses.
      STR_IN_OUT_CASE("(\\", "(\\"),
      // Handling ending delimiters.
      STR_IN_OUT_CASE("> little bear", ">"),
      STR_IN_OUT_CASE(") another bear", ")"),
      STR_IN_OUT_CASE(">> end ", ">>"),
      // No ending delimiters.
      STR_IN_OUT_CASE("(sdfgfgbcv", "(sdfgfgbcv"),
      // Other delimiters.
      STR_IN_OUT_CASE("}", "}"),
      // Regular cases.
      STR_IN_OUT_CASE("apple pear", "apple"),
      STR_IN_OUT_CASE(" pi=3.1415 ", "pi=3.1415"),
      STR_IN_OUT_CASE(" p t x c ", "p"),
      STR_IN_OUT_CASE(" pt\0xc ", "pt"),
      STR_IN_OUT_CASE(" $^&&*\t\0sdff ", "$^&&*"),
      STR_IN_OUT_CASE("\n\r+3.5656 -11.0", "+3.5656"),
  };
  size_t i = 0;
  for (const pdfium::StrFuncTestData& data : test_data) {
    CPDF_SimpleParser parser(data.input_span());
    EXPECT_EQ(parser.GetWord(), ByteStringView(data.expected_span()))
        << " for case " << i;
    ++i;
  }
}

TEST(SimpleParserTest, Bug358381390) {
  const char kInput[] = "1 beginbfchar\n<01> <>\nendbfchar\n1 beginbfchar";

  CPDF_SimpleParser parser(pdfium::as_byte_span(kInput));
  EXPECT_EQ(parser.GetWord(), "1");
  EXPECT_EQ(parser.GetWord(), "beginbfchar");
  EXPECT_EQ(parser.GetWord(), "<01>");
  EXPECT_EQ(parser.GetWord(), "<>");
  EXPECT_EQ(parser.GetWord(), "endbfchar");
  EXPECT_EQ(parser.GetWord(), "1");
  EXPECT_EQ(parser.GetWord(), "beginbfchar");
  EXPECT_EQ(parser.GetWord(), "");
}
