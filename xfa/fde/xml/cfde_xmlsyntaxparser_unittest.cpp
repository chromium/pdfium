// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fde/xml/cfde_xmlsyntaxparser.h"

#include <memory>

#include "testing/gtest/include/gtest/gtest.h"
#include "xfa/fgas/crt/ifgas_stream.h"

TEST(CFDE_XMLSyntaxParser, CData) {
  const wchar_t* input =
      L"<script contentType=\"application/x-javascript\">\n"
      L"  <![CDATA[\n"
      L"    if (a[1] < 3)\n"
      L"      app.alert(\"Tclams\");\n"
      L"  ]]>\n"
      L"</script>";

  const wchar_t* cdata =
      L"\n"
      L"    if (a[1] < 3)\n"
      L"      app.alert(\"Tclams\");\n"
      L"  ";

  // We * sizeof(wchar_t) because we pass in the uint8_t, not the wchar_t.
  size_t len = FXSYS_wcslen(input) * sizeof(wchar_t);
  CFX_RetainPtr<IFGAS_Stream> stream = IFGAS_Stream::CreateStream(
      reinterpret_cast<uint8_t*>(const_cast<wchar_t*>(input)), len, 0);
  CFDE_XMLSyntaxParser parser;
  parser.Init(stream, 256);
  EXPECT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  EXPECT_EQ(L"contentType", parser.GetAttributeName());
  EXPECT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n  ", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::CData, parser.DoSyntaxParse());
  EXPECT_EQ(cdata, parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFDE_XMLSyntaxParser, CDataWithInnerScript) {
  const wchar_t* input =
      L"<script contentType=\"application/x-javascript\">\n"
      L"  <![CDATA[\n"
      L"    if (a[1] < 3)\n"
      L"      app.alert(\"Tclams\");\n"
      L"    </script>\n"
      L"  ]]>\n"
      L"</script>";

  const wchar_t* cdata =
      L"\n"
      L"    if (a[1] < 3)\n"
      L"      app.alert(\"Tclams\");\n"
      L"    </script>\n"
      L"  ";

  // We * sizeof(wchar_t) because we pass in the uint8_t, not the wchar_t.
  size_t len = FXSYS_wcslen(input) * sizeof(wchar_t);
  CFX_RetainPtr<IFGAS_Stream> stream = IFGAS_Stream::CreateStream(
      reinterpret_cast<uint8_t*>(const_cast<wchar_t*>(input)), len, 0);
  CFDE_XMLSyntaxParser parser;
  parser.Init(stream, 256);
  EXPECT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  EXPECT_EQ(L"contentType", parser.GetAttributeName());
  EXPECT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n  ", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::CData, parser.DoSyntaxParse());
  EXPECT_EQ(cdata, parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFDE_XMLSyntaxParser, ArrowBangArrow) {
  const wchar_t* input =
      L"<script contentType=\"application/x-javascript\">\n"
      L"  <!>\n"
      L"</script>";

  // We * sizeof(wchar_t) because we pass in the uint8_t, not the wchar_t.
  size_t len = FXSYS_wcslen(input) * sizeof(wchar_t);
  CFX_RetainPtr<IFGAS_Stream> stream = IFGAS_Stream::CreateStream(
      reinterpret_cast<uint8_t*>(const_cast<wchar_t*>(input)), len, 0);
  CFDE_XMLSyntaxParser parser;
  parser.Init(stream, 256);
  EXPECT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());

  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  EXPECT_EQ(L"contentType", parser.GetAttributeName());
  EXPECT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n  ", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFDE_XMLSyntaxParser, ArrowBangBracketArrow) {
  const wchar_t* input =
      L"<script contentType=\"application/x-javascript\">\n"
      L"  <![>\n"
      L"</script>";

  // We * sizeof(wchar_t) because we pass in the uint8_t, not the wchar_t.
  size_t len = FXSYS_wcslen(input) * sizeof(wchar_t);
  CFX_RetainPtr<IFGAS_Stream> stream = IFGAS_Stream::CreateStream(
      reinterpret_cast<uint8_t*>(const_cast<wchar_t*>(input)), len, 0);
  CFDE_XMLSyntaxParser parser;
  parser.Init(stream, 256);
  EXPECT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  EXPECT_EQ(L"contentType", parser.GetAttributeName());
  EXPECT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n  ", parser.GetTextData());

  // Parser walks to end of input.

  EXPECT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFDE_XMLSyntaxParser, IncompleteCData) {
  const wchar_t* input =
      L"<script contentType=\"application/x-javascript\">\n"
      L"  <![CDATA>\n"
      L"</script>";

  // We * sizeof(wchar_t) because we pass in the uint8_t, not the wchar_t.
  size_t len = FXSYS_wcslen(input) * sizeof(wchar_t);
  CFX_RetainPtr<IFGAS_Stream> stream = IFGAS_Stream::CreateStream(
      reinterpret_cast<uint8_t*>(const_cast<wchar_t*>(input)), len, 0);
  CFDE_XMLSyntaxParser parser;
  parser.Init(stream, 256);
  EXPECT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  EXPECT_EQ(L"contentType", parser.GetAttributeName());
  EXPECT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n  ", parser.GetTextData());

  // Parser walks to end of input.

  EXPECT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFDE_XMLSyntaxParser, UnClosedCData) {
  const wchar_t* input =
      L"<script contentType=\"application/x-javascript\">\n"
      L"  <![CDATA[\n"
      L"</script>";

  // We * sizeof(wchar_t) because we pass in the uint8_t, not the wchar_t.
  size_t len = FXSYS_wcslen(input) * sizeof(wchar_t);
  CFX_RetainPtr<IFGAS_Stream> stream = IFGAS_Stream::CreateStream(
      reinterpret_cast<uint8_t*>(const_cast<wchar_t*>(input)), len, 0);
  CFDE_XMLSyntaxParser parser;
  parser.Init(stream, 256);
  EXPECT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  EXPECT_EQ(L"contentType", parser.GetAttributeName());
  EXPECT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n  ", parser.GetTextData());

  // Parser walks to end of input.

  EXPECT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFDE_XMLSyntaxParser, EmptyCData) {
  const wchar_t* input =
      L"<script contentType=\"application/x-javascript\">\n"
      L"  <![CDATA[]]>\n"
      L"</script>";

  // We * sizeof(wchar_t) because we pass in the uint8_t, not the wchar_t.
  size_t len = FXSYS_wcslen(input) * sizeof(wchar_t);
  CFX_RetainPtr<IFGAS_Stream> stream = IFGAS_Stream::CreateStream(
      reinterpret_cast<uint8_t*>(const_cast<wchar_t*>(input)), len, 0);
  CFDE_XMLSyntaxParser parser;
  parser.Init(stream, 256);
  EXPECT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  EXPECT_EQ(L"contentType", parser.GetAttributeName());
  EXPECT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n  ", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::CData, parser.DoSyntaxParse());
  EXPECT_EQ(L"", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFDE_XMLSyntaxParser, Comment) {
  const wchar_t* input =
      L"<script contentType=\"application/x-javascript\">\n"
      L"  <!-- A Comment -->\n"
      L"</script>";

  // We * sizeof(wchar_t) because we pass in the uint8_t, not the wchar_t.
  size_t len = FXSYS_wcslen(input) * sizeof(wchar_t);
  CFX_RetainPtr<IFGAS_Stream> stream = IFGAS_Stream::CreateStream(
      reinterpret_cast<uint8_t*>(const_cast<wchar_t*>(input)), len, 0);
  CFDE_XMLSyntaxParser parser;
  parser.Init(stream, 256);
  EXPECT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  EXPECT_EQ(L"contentType", parser.GetAttributeName());
  EXPECT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n  ", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFDE_XMLSyntaxParser, IncorrectCommentStart) {
  const wchar_t* input =
      L"<script contentType=\"application/x-javascript\">\n"
      L"  <!- A Comment -->\n"
      L"</script>";

  // We * sizeof(wchar_t) because we pass in the uint8_t, not the wchar_t.
  size_t len = FXSYS_wcslen(input) * sizeof(wchar_t);
  CFX_RetainPtr<IFGAS_Stream> stream = IFGAS_Stream::CreateStream(
      reinterpret_cast<uint8_t*>(const_cast<wchar_t*>(input)), len, 0);
  CFDE_XMLSyntaxParser parser;
  parser.Init(stream, 256);
  EXPECT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  EXPECT_EQ(L"contentType", parser.GetAttributeName());
  EXPECT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n  ", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFDE_XMLSyntaxParser, CommentEmpty) {
  const wchar_t* input =
      L"<script contentType=\"application/x-javascript\">\n"
      L"  <!---->\n"
      L"</script>";

  // We * sizeof(wchar_t) because we pass in the uint8_t, not the wchar_t.
  size_t len = FXSYS_wcslen(input) * sizeof(wchar_t);
  CFX_RetainPtr<IFGAS_Stream> stream = IFGAS_Stream::CreateStream(
      reinterpret_cast<uint8_t*>(const_cast<wchar_t*>(input)), len, 0);
  CFDE_XMLSyntaxParser parser;
  parser.Init(stream, 256);
  EXPECT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  EXPECT_EQ(L"contentType", parser.GetAttributeName());
  EXPECT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n  ", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFDE_XMLSyntaxParser, CommentThreeDash) {
  const wchar_t* input =
      L"<script contentType=\"application/x-javascript\">\n"
      L"  <!--->\n"
      L"</script>";

  // We * sizeof(wchar_t) because we pass in the uint8_t, not the wchar_t.
  size_t len = FXSYS_wcslen(input) * sizeof(wchar_t);
  CFX_RetainPtr<IFGAS_Stream> stream = IFGAS_Stream::CreateStream(
      reinterpret_cast<uint8_t*>(const_cast<wchar_t*>(input)), len, 0);
  CFDE_XMLSyntaxParser parser;
  parser.Init(stream, 256);
  EXPECT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  EXPECT_EQ(L"contentType", parser.GetAttributeName());
  EXPECT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n  ", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFDE_XMLSyntaxParser, CommentTwoDash) {
  const wchar_t* input =
      L"<script contentType=\"application/x-javascript\">\n"
      L"  <!-->\n"
      L"</script>";

  // We * sizeof(wchar_t) because we pass in the uint8_t, not the wchar_t.
  size_t len = FXSYS_wcslen(input) * sizeof(wchar_t);
  CFX_RetainPtr<IFGAS_Stream> stream = IFGAS_Stream::CreateStream(
      reinterpret_cast<uint8_t*>(const_cast<wchar_t*>(input)), len, 0);
  CFDE_XMLSyntaxParser parser;
  parser.Init(stream, 256);
  EXPECT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  EXPECT_EQ(L"contentType", parser.GetAttributeName());
  EXPECT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"\n  ", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFDE_XMLSyntaxParser, Entities) {
  const wchar_t* input =
      L"<script contentType=\"application/x-javascript\">"
      L"&#66;"
      L"&#x54;"
      L"&#x00000000000000000048;"
      L"&#x0000000000000000AB48;"
      L"&#x0000000000000000000;"
      L"</script>";

  // We * sizeof(wchar_t) because we pass in the uint8_t, not the wchar_t.
  size_t len = FXSYS_wcslen(input) * sizeof(wchar_t);
  CFX_RetainPtr<IFGAS_Stream> stream = IFGAS_Stream::CreateStream(
      reinterpret_cast<uint8_t*>(const_cast<wchar_t*>(input)), len, 0);
  CFDE_XMLSyntaxParser parser;
  parser.Init(stream, 256);
  EXPECT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  EXPECT_EQ(L"contentType", parser.GetAttributeName());
  EXPECT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"BTH\xab48", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFDE_XMLSyntaxParser, EntityOverflowHex) {
  const wchar_t* input =
      L"<script contentType=\"application/x-javascript\">"
      L"&#xaDBDFFFFF;"
      L"&#xafffffffffffffffffffffffffffffffff;"
      L"</script>";

  // We * sizeof(wchar_t) because we pass in the uint8_t, not the wchar_t.
  size_t len = FXSYS_wcslen(input) * sizeof(wchar_t);
  CFX_RetainPtr<IFGAS_Stream> stream = IFGAS_Stream::CreateStream(
      reinterpret_cast<uint8_t*>(const_cast<wchar_t*>(input)), len, 0);
  CFDE_XMLSyntaxParser parser;
  parser.Init(stream, 256);
  EXPECT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  EXPECT_EQ(L"contentType", parser.GetAttributeName());
  EXPECT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"  ", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFDE_XMLSyntaxParser, EntityOverflowDecimal) {
  const wchar_t* input =
      L"<script contentType=\"application/x-javascript\">"
      L"&#2914910205;"
      L"&#29149102052342342134521341234512351234213452315;"
      L"</script>";

  // We * sizeof(wchar_t) because we pass in the uint8_t, not the wchar_t.
  size_t len = FXSYS_wcslen(input) * sizeof(wchar_t);
  CFX_RetainPtr<IFGAS_Stream> stream = IFGAS_Stream::CreateStream(
      reinterpret_cast<uint8_t*>(const_cast<wchar_t*>(input)), len, 0);
  CFDE_XMLSyntaxParser parser;
  parser.Init(stream, 256);
  EXPECT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  EXPECT_EQ(L"contentType", parser.GetAttributeName());
  EXPECT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  EXPECT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  EXPECT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  EXPECT_EQ(L"  ", parser.GetTextData());

  EXPECT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  EXPECT_EQ(L"script", parser.GetTagName());

  EXPECT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}
