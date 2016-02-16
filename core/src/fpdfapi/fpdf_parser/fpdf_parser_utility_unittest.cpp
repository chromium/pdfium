// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/include/fpdfapi/fpdf_parser.h"

#include <string>

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

TEST(SimpleParserTest, GetWord) {
  pdfium::StrFuncTestData test_data[] = {
      // Empty src string.
      STR_TEST_CASE("", ""),
      // Content with whitespaces only.
      STR_TEST_CASE(" \t \0 \n", ""),
      // Content with comments only.
      STR_TEST_CASE("%this is a test case\r\n%2nd line", ""),
      // Mixed whitespaces and comments.
      STR_TEST_CASE(" \t \0%try()%haha\n %another line \aa", ""),
      // Name.
      STR_TEST_CASE(" /Tester ", "/Tester"),
      // String.
      STR_TEST_CASE("\t(nice day)!\n ", "(nice day)"),
      // String with nested braces.
      STR_TEST_CASE("\t(It is a (long) day)!\n ", "(It is a (long) day)"),
      // String with escaped chars.
      STR_TEST_CASE("\t(It is a \\(long\\) day!)hi\n ",
                    "(It is a \\(long\\) day!)"),
      // Hex string.
      STR_TEST_CASE(" \n<4545acdfedertt>abc ", "<4545acdfedertt>"),
      STR_TEST_CASE(" \n<4545a<ed>ertt>abc ", "<4545a<ed>"),
      // Dictionary.
      STR_TEST_CASE("<</oc 234 /color 2 3 R>>", "<<"),
      STR_TEST_CASE("\t\t<< /abc>>", "<<"),
      // Handling ending delimiters.
      STR_TEST_CASE("> little bear", ">"),
      STR_TEST_CASE(") another bear", ")"),
      STR_TEST_CASE(">> end ", ">>"),
      // No ending delimiters.
      STR_TEST_CASE("(sdfgfgbcv", "(sdfgfgbcv"),
      // Regular cases.
      STR_TEST_CASE("apple pear", "apple"),
      STR_TEST_CASE(" pi=3.1415 ", "pi=3.1415"),
      STR_TEST_CASE(" p t x c ", "p"),
      STR_TEST_CASE(" pt\0xc ", "pt"),
      STR_TEST_CASE(" $^&&*\t\0sdff ", "$^&&*"),
      STR_TEST_CASE("\n\r+3.5656 -11.0", "+3.5656"),
  };
  for (size_t i = 0; i < FX_ArraySize(test_data); ++i) {
    const pdfium::StrFuncTestData& data = test_data[i];
    CPDF_SimpleParser parser(data.input, data.input_size);
    CFX_ByteStringC word = parser.GetWord();
    EXPECT_EQ(std::string(reinterpret_cast<const char*>(data.expected),
                          data.expected_size),
              std::string(word.GetCStr(), word.GetLength()))
        << " for case " << i;
  }
}

TEST(SimpleParserTest, FindTagParamFromStart) {
  struct FindTagTestStruct {
    const unsigned char* input;
    unsigned int input_size;
    const char* token;
    int num_params;
    bool result;
    unsigned int result_pos;
  } test_data[] = {
      // Empty strings.
      STR_IN_TEST_CASE("", "Tj", 1, false, 0),
      STR_IN_TEST_CASE("", "", 1, false, 0),
      // Empty token.
      STR_IN_TEST_CASE("  T j", "", 1, false, 5),
      // No parameter.
      STR_IN_TEST_CASE("Tj", "Tj", 1, false, 2),
      STR_IN_TEST_CASE("(Tj", "Tj", 1, false, 3),
      // Partial token match.
      STR_IN_TEST_CASE("\r12\t34  56 78Tj", "Tj", 1, false, 15),
      // Regular cases with various parameters.
      STR_IN_TEST_CASE("\r\0abd Tj", "Tj", 1, true, 0),
      STR_IN_TEST_CASE("12 4 Tj 3 46 Tj", "Tj", 1, true, 2),
      STR_IN_TEST_CASE("er^ 2 (34) (5667) Tj", "Tj", 2, true, 5),
      STR_IN_TEST_CASE("<344> (232)\t343.4\n12 45 Tj", "Tj", 3, true, 11),
      STR_IN_TEST_CASE("1 2 3 4 5 6 7 8 cm", "cm", 6, true, 3),
  };
  for (size_t i = 0; i < FX_ArraySize(test_data); ++i) {
    const FindTagTestStruct& data = test_data[i];
    CPDF_SimpleParser parser(data.input, data.input_size);
    EXPECT_EQ(data.result,
              parser.FindTagParamFromStart(data.token, data.num_params))
        << " for case " << i;
    EXPECT_EQ(data.result_pos, parser.GetCurPos()) << " for case " << i;
  }
}
