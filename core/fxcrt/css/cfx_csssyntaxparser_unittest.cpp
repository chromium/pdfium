// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/css/cfx_csssyntaxparser.h"

#include "testing/gtest/include/gtest/gtest.h"

// These tests cover the "declaration only" mode of the CSS Syntax Parser
// (i.e. inline style="" attribute). The cfx_cssstylesheet_unitttest.cpp
// file covers the full-up selector { ... } parsing.

TEST(CSSSyntaxParserTest, ParseEmpty) {
  const wchar_t* input = L"";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();
  EXPECT_EQ(CFX_CSSSyntaxStatus::kEOS, parser.DoSyntaxParse());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kEOS, parser.DoSyntaxParse());
}

TEST(CSSSyntaxParserTest, ParseBlank) {
  const wchar_t* input = L"  \n\r\t";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();
  EXPECT_EQ(CFX_CSSSyntaxStatus::kEOS, parser.DoSyntaxParse());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kEOS, parser.DoSyntaxParse());
}

TEST(CSSSyntaxParserTest, ParseMissingColon) {
  const wchar_t* input = L"foo ";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();
  EXPECT_EQ(CFX_CSSSyntaxStatus::kEOS, parser.DoSyntaxParse());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kEOS, parser.DoSyntaxParse());
}

TEST(CSSSyntaxParserTest, ParseMissingValue) {
  const wchar_t* input = L"foo: ";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyName, parser.DoSyntaxParse());
  EXPECT_EQ(L"foo", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kEOS, parser.DoSyntaxParse());
}

TEST(CSSSyntaxParserTest, ParseSingleProp1) {
  const wchar_t* input = L"foo:bar";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyName, parser.DoSyntaxParse());
  EXPECT_EQ(L"foo", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"bar", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kEOS, parser.DoSyntaxParse());
}

TEST(CSSSyntaxParserTest, ParseSingleProp2) {
  const wchar_t* input = L"foo:bar;";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyName, parser.DoSyntaxParse());
  EXPECT_EQ(L"foo", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"bar", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kEOS, parser.DoSyntaxParse());
}

TEST(CSSSyntaxParserTest, ParseMissingColonMultiple) {
  const wchar_t* input = L"foo:bar; baz";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyName, parser.DoSyntaxParse());
  EXPECT_EQ(L"foo", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"bar", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kEOS, parser.DoSyntaxParse());
}

TEST(CSSSyntaxParserTest, ParseMissingPropertyMultiple) {
  const wchar_t* input = L"foo:bar; baz: ";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyName, parser.DoSyntaxParse());
  EXPECT_EQ(L"foo", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"bar", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyName, parser.DoSyntaxParse());
  EXPECT_EQ(L"baz", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kEOS, parser.DoSyntaxParse());
}

TEST(CSSSyntaxParserTest, ParseMultipleProp1) {
  const wchar_t* input = L"foo : bar; baz : bam";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyName, parser.DoSyntaxParse());
  EXPECT_EQ(L"foo", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"bar", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyName, parser.DoSyntaxParse());
  EXPECT_EQ(L"baz", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"bam", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kEOS, parser.DoSyntaxParse());
}

TEST(CSSSyntaxParserTest, ParseMultipleProp2) {
  const wchar_t* input = L"foo:bar;baz:bam;";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyName, parser.DoSyntaxParse());
  EXPECT_EQ(L"foo", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"bar", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyName, parser.DoSyntaxParse());
  EXPECT_EQ(L"baz", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"bam", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kEOS, parser.DoSyntaxParse());
}

TEST(CSSSyntaxParserTest, ParseOpenBrace1) {
  const wchar_t* input = L"{a:3}";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();

  // TODO(tsepez): these should fail on stray punctuation.
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyName, parser.DoSyntaxParse());
  EXPECT_EQ(L"{a", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"3", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kError, parser.DoSyntaxParse());
}

TEST(CSSSyntaxParserTest, ParseOpenBrace2) {
  const wchar_t* input = L"foo {a:3}";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();

  // TODO(tsepez): these should fail on stray punctuation.
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyName, parser.DoSyntaxParse());
  EXPECT_EQ(L"foo {a", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"3", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kError, parser.DoSyntaxParse());
}

TEST(CSSSyntaxParserTest, ParseOpenBrace3) {
  const wchar_t* input = L"foo: bar {a:3}";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();

  // TODO(tsepez): these should fail on stray punctuation.
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyName, parser.DoSyntaxParse());
  EXPECT_EQ(L"foo", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"bar {a:3", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kError, parser.DoSyntaxParse());
}

TEST(CSSSyntaxParserTest, ParseOpenBrace4) {
  const wchar_t* input = L"foo: bar; {a:3}";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyName, parser.DoSyntaxParse());
  EXPECT_EQ(L"foo", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"bar", parser.GetCurrentString());

  // TODO(tsepez): these should fail on stray punctuation.
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyName, parser.DoSyntaxParse());
  EXPECT_EQ(L"{a", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"3", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kError, parser.DoSyntaxParse());
}

TEST(CSSSyntaxParserTest, ParseCloseBrace1) {
  const wchar_t* input = L"} foo:bar";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();
  EXPECT_EQ(CFX_CSSSyntaxStatus::kError, parser.DoSyntaxParse());
}

TEST(CSSSyntaxParserTest, ParseCloseBrace2) {
  const wchar_t* input = L"foo}:bar";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();
  EXPECT_EQ(CFX_CSSSyntaxStatus::kError, parser.DoSyntaxParse());
}

TEST(CSSSyntaxParserTest, ParseCloseBrace3) {
  const wchar_t* input = L"foo:bar}";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyName, parser.DoSyntaxParse());
  EXPECT_EQ(L"foo", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"bar", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kError, parser.DoSyntaxParse());
}

TEST(CSSSyntaxParserTest, ParseCloseBrace4) {
  const wchar_t* input = L"foo:bar;}";
  CFX_CSSSyntaxParser parser(input);
  parser.SetParseOnlyDeclarations();
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyName, parser.DoSyntaxParse());
  EXPECT_EQ(L"foo", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kPropertyValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"bar", parser.GetCurrentString());
  EXPECT_EQ(CFX_CSSSyntaxStatus::kError, parser.DoSyntaxParse());
}
