// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fde/xml/cfde_xmlsyntaxparser.h"

#include <memory>

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"
#include "xfa/fgas/crt/fgas_codepage.h"
#include "xfa/fgas/crt/ifgas_stream.h"

class CFDE_XMLSyntaxParserTest : public pdfium::FPDF_Test {};

TEST_F(CFDE_XMLSyntaxParserTest, CData) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <![CDATA[\n"
      "    if (a[1] < 3)\n"
      "      app.alert(\"Tclams\");\n"
      "  ]]>\n"
      "</script>";

  const wchar_t* cdata =
      L"\n"
      L"    if (a[1] < 3)\n"
      L"      app.alert(\"Tclams\");\n"
      L"  ";

  CFX_RetainPtr<IFGAS_Stream> stream =
      IFGAS_Stream::CreateReadStream(IFX_MemoryStream::Create(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input)));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  CFDE_XMLSyntaxParser parser(stream);
  ASSERT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::CData, parser.DoSyntaxParse());
  ASSERT_EQ(cdata, parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST_F(CFDE_XMLSyntaxParserTest, CDataWithInnerScript) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <![CDATA[\n"
      "    if (a[1] < 3)\n"
      "      app.alert(\"Tclams\");\n"
      "    </script>\n"
      "  ]]>\n"
      "</script>";

  const wchar_t* cdata =
      L"\n"
      L"    if (a[1] < 3)\n"
      L"      app.alert(\"Tclams\");\n"
      L"    </script>\n"
      L"  ";

  CFX_RetainPtr<IFGAS_Stream> stream =
      IFGAS_Stream::CreateReadStream(IFX_MemoryStream::Create(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input)));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  CFDE_XMLSyntaxParser parser(stream);
  ASSERT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::CData, parser.DoSyntaxParse());
  ASSERT_EQ(cdata, parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST_F(CFDE_XMLSyntaxParserTest, ArrowBangArrow) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <!>\n"
      "</script>";

  CFX_RetainPtr<IFGAS_Stream> stream =
      IFGAS_Stream::CreateReadStream(IFX_MemoryStream::Create(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input)));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  CFDE_XMLSyntaxParser parser(stream);
  ASSERT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());

  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST_F(CFDE_XMLSyntaxParserTest, ArrowBangBracketArrow) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <![>\n"
      "</script>";

  CFX_RetainPtr<IFGAS_Stream> stream =
      IFGAS_Stream::CreateReadStream(IFX_MemoryStream::Create(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input)));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  CFDE_XMLSyntaxParser parser(stream);
  ASSERT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  // Parser walks to end of input.

  ASSERT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST_F(CFDE_XMLSyntaxParserTest, IncompleteCData) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <![CDATA>\n"
      "</script>";

  CFX_RetainPtr<IFGAS_Stream> stream =
      IFGAS_Stream::CreateReadStream(IFX_MemoryStream::Create(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input)));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  CFDE_XMLSyntaxParser parser(stream);
  ASSERT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  // Parser walks to end of input.

  ASSERT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST_F(CFDE_XMLSyntaxParserTest, UnClosedCData) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <![CDATA[\n"
      "</script>";

  CFX_RetainPtr<IFGAS_Stream> stream =
      IFGAS_Stream::CreateReadStream(IFX_MemoryStream::Create(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input)));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  CFDE_XMLSyntaxParser parser(stream);
  ASSERT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  // Parser walks to end of input.

  ASSERT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST_F(CFDE_XMLSyntaxParserTest, EmptyCData) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <![CDATA[]]>\n"
      "</script>";

  CFX_RetainPtr<IFGAS_Stream> stream =
      IFGAS_Stream::CreateReadStream(IFX_MemoryStream::Create(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input)));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  CFDE_XMLSyntaxParser parser(stream);
  ASSERT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::CData, parser.DoSyntaxParse());
  ASSERT_EQ(L"", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST_F(CFDE_XMLSyntaxParserTest, Comment) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <!-- A Comment -->\n"
      "</script>";

  CFX_RetainPtr<IFGAS_Stream> stream =
      IFGAS_Stream::CreateReadStream(IFX_MemoryStream::Create(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input)));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  CFDE_XMLSyntaxParser parser(stream);
  ASSERT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST_F(CFDE_XMLSyntaxParserTest, IncorrectCommentStart) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <!- A Comment -->\n"
      "</script>";

  CFX_RetainPtr<IFGAS_Stream> stream =
      IFGAS_Stream::CreateReadStream(IFX_MemoryStream::Create(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input)));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  CFDE_XMLSyntaxParser parser(stream);
  ASSERT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST_F(CFDE_XMLSyntaxParserTest, CommentEmpty) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <!---->\n"
      "</script>";

  CFX_RetainPtr<IFGAS_Stream> stream =
      IFGAS_Stream::CreateReadStream(IFX_MemoryStream::Create(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input)));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  CFDE_XMLSyntaxParser parser(stream);
  ASSERT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST_F(CFDE_XMLSyntaxParserTest, CommentThreeDash) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <!--->\n"
      "</script>";

  CFX_RetainPtr<IFGAS_Stream> stream =
      IFGAS_Stream::CreateReadStream(IFX_MemoryStream::Create(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input)));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  CFDE_XMLSyntaxParser parser(stream);
  ASSERT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST_F(CFDE_XMLSyntaxParserTest, CommentTwoDash) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <!-->\n"
      "</script>";

  CFX_RetainPtr<IFGAS_Stream> stream =
      IFGAS_Stream::CreateReadStream(IFX_MemoryStream::Create(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input)));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  CFDE_XMLSyntaxParser parser(stream);
  ASSERT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST_F(CFDE_XMLSyntaxParserTest, Entities) {
  const char* input =
      "<script contentType=\"application/x-javascript\">"
      "&#66;"
      "&#x54;"
      "&#x00000000000000000048;"
      "&#x0000000000000000AB48;"
      "&#x0000000000000000000;"
      "</script>";

  CFX_RetainPtr<IFGAS_Stream> stream =
      IFGAS_Stream::CreateReadStream(IFX_MemoryStream::Create(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input)));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  CFDE_XMLSyntaxParser parser(stream);
  ASSERT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"BTH\xab48", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST_F(CFDE_XMLSyntaxParserTest, EntityOverflowHex) {
  const char* input =
      "<script contentType=\"application/x-javascript\">"
      "&#xaDBDFFFFF;"
      "&#xafffffffffffffffffffffffffffffffff;"
      "</script>";

  CFX_RetainPtr<IFGAS_Stream> stream =
      IFGAS_Stream::CreateReadStream(IFX_MemoryStream::Create(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input)));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  CFDE_XMLSyntaxParser parser(stream);
  ASSERT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"  ", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST_F(CFDE_XMLSyntaxParserTest, EntityOverflowDecimal) {
  const char* input =
      "<script contentType=\"application/x-javascript\">"
      "&#2914910205;"
      "&#29149102052342342134521341234512351234213452315;"
      "</script>";

  CFX_RetainPtr<IFGAS_Stream> stream =
      IFGAS_Stream::CreateReadStream(IFX_MemoryStream::Create(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input)));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  CFDE_XMLSyntaxParser parser(stream);
  ASSERT_EQ(FDE_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FDE_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FDE_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"  ", parser.GetTextData());

  ASSERT_EQ(FDE_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FDE_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}
