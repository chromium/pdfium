// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/xml/cfx_xmlparser.h"

#include <memory>

#include "core/fxcrt/cfx_seekablestreamproxy.h"
#include "core/fxcrt/fx_codepage.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

class CFX_XMLTestParser : public CFX_XMLParser {
 public:
  CFX_XMLTestParser(CFX_XMLNode* pParent,
                    const RetainPtr<CFX_SeekableStreamProxy>& pStream)
      : CFX_XMLParser(pParent, pStream) {}

  ~CFX_XMLTestParser() override = default;

  FX_XmlSyntaxResult DoSyntaxParse() { return CFX_XMLParser::DoSyntaxParse(); }

  WideString GetTagName() const { return CFX_XMLParser::GetTagName(); }

  WideString GetAttributeName() const {
    return CFX_XMLParser::GetAttributeName();
  }

  WideString GetAttributeValue() const {
    return CFX_XMLParser::GetAttributeValue();
  }

  WideString GetTextData() const { return CFX_XMLParser::GetTextData(); }
};

TEST(CFX_XMLParserTest, CData) {
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

  RetainPtr<CFX_SeekableStreamProxy> stream =
      pdfium::MakeRetain<CFX_SeekableStreamProxy>(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  auto root = pdfium::MakeUnique<CFX_XMLNode>();
  CFX_XMLTestParser parser(root.get(), stream);
  ASSERT_EQ(FX_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FX_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::CData, parser.DoSyntaxParse());
  ASSERT_EQ(cdata, parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFX_XMLParserTest, CDataWithInnerScript) {
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

  RetainPtr<CFX_SeekableStreamProxy> stream =
      pdfium::MakeRetain<CFX_SeekableStreamProxy>(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  auto root = pdfium::MakeUnique<CFX_XMLNode>();
  CFX_XMLTestParser parser(root.get(), stream);
  ASSERT_EQ(FX_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FX_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::CData, parser.DoSyntaxParse());
  ASSERT_EQ(cdata, parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFX_XMLParserTest, ArrowBangArrow) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <!>\n"
      "</script>";

  RetainPtr<CFX_SeekableStreamProxy> stream =
      pdfium::MakeRetain<CFX_SeekableStreamProxy>(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  auto root = pdfium::MakeUnique<CFX_XMLNode>();
  CFX_XMLTestParser parser(root.get(), stream);
  ASSERT_EQ(FX_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::TagName, parser.DoSyntaxParse());

  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FX_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFX_XMLParserTest, ArrowBangBracketArrow) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <![>\n"
      "</script>";

  RetainPtr<CFX_SeekableStreamProxy> stream =
      pdfium::MakeRetain<CFX_SeekableStreamProxy>(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  auto root = pdfium::MakeUnique<CFX_XMLNode>();
  CFX_XMLTestParser parser(root.get(), stream);
  ASSERT_EQ(FX_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FX_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  // Parser walks to end of input.

  ASSERT_EQ(FX_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFX_XMLParserTest, IncompleteCData) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <![CDATA>\n"
      "</script>";

  RetainPtr<CFX_SeekableStreamProxy> stream =
      pdfium::MakeRetain<CFX_SeekableStreamProxy>(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  auto root = pdfium::MakeUnique<CFX_XMLNode>();
  CFX_XMLTestParser parser(root.get(), stream);
  ASSERT_EQ(FX_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FX_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  // Parser walks to end of input.

  ASSERT_EQ(FX_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFX_XMLParserTest, UnClosedCData) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <![CDATA[\n"
      "</script>";

  RetainPtr<CFX_SeekableStreamProxy> stream =
      pdfium::MakeRetain<CFX_SeekableStreamProxy>(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  auto root = pdfium::MakeUnique<CFX_XMLNode>();
  CFX_XMLTestParser parser(root.get(), stream);
  ASSERT_EQ(FX_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FX_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  // Parser walks to end of input.

  ASSERT_EQ(FX_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFX_XMLParserTest, EmptyCData) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <![CDATA[]]>\n"
      "</script>";

  RetainPtr<CFX_SeekableStreamProxy> stream =
      pdfium::MakeRetain<CFX_SeekableStreamProxy>(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  auto root = pdfium::MakeUnique<CFX_XMLNode>();
  CFX_XMLTestParser parser(root.get(), stream);
  ASSERT_EQ(FX_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FX_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::CData, parser.DoSyntaxParse());
  ASSERT_EQ(L"", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFX_XMLParserTest, Comment) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <!-- A Comment -->\n"
      "</script>";

  RetainPtr<CFX_SeekableStreamProxy> stream =
      pdfium::MakeRetain<CFX_SeekableStreamProxy>(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  auto root = pdfium::MakeUnique<CFX_XMLNode>();
  CFX_XMLTestParser parser(root.get(), stream);
  ASSERT_EQ(FX_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FX_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFX_XMLParserTest, IncorrectCommentStart) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <!- A Comment -->\n"
      "</script>";

  RetainPtr<CFX_SeekableStreamProxy> stream =
      pdfium::MakeRetain<CFX_SeekableStreamProxy>(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  auto root = pdfium::MakeUnique<CFX_XMLNode>();
  CFX_XMLTestParser parser(root.get(), stream);
  ASSERT_EQ(FX_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FX_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFX_XMLParserTest, CommentEmpty) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <!---->\n"
      "</script>";

  RetainPtr<CFX_SeekableStreamProxy> stream =
      pdfium::MakeRetain<CFX_SeekableStreamProxy>(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  auto root = pdfium::MakeUnique<CFX_XMLNode>();
  CFX_XMLTestParser parser(root.get(), stream);
  ASSERT_EQ(FX_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FX_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFX_XMLParserTest, CommentThreeDash) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <!--->\n"
      "</script>";

  RetainPtr<CFX_SeekableStreamProxy> stream =
      pdfium::MakeRetain<CFX_SeekableStreamProxy>(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  auto root = pdfium::MakeUnique<CFX_XMLNode>();
  CFX_XMLTestParser parser(root.get(), stream);
  ASSERT_EQ(FX_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FX_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFX_XMLParserTest, CommentTwoDash) {
  const char* input =
      "<script contentType=\"application/x-javascript\">\n"
      "  <!-->\n"
      "</script>";

  RetainPtr<CFX_SeekableStreamProxy> stream =
      pdfium::MakeRetain<CFX_SeekableStreamProxy>(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  auto root = pdfium::MakeUnique<CFX_XMLNode>();
  CFX_XMLTestParser parser(root.get(), stream);
  ASSERT_EQ(FX_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FX_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"\n  ", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFX_XMLParserTest, Entities) {
  const char* input =
      "<script contentType=\"application/x-javascript\">"
      "&#66;"
      "&#x54;"
      "&#x00000000000000000048;"
      "&#x0000000000000000AB48;"
      "&#x0000000000000000000;"
      "</script>";

  RetainPtr<CFX_SeekableStreamProxy> stream =
      pdfium::MakeRetain<CFX_SeekableStreamProxy>(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  auto root = pdfium::MakeUnique<CFX_XMLNode>();
  CFX_XMLTestParser parser(root.get(), stream);
  ASSERT_EQ(FX_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FX_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"BTH\xab48", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFX_XMLParserTest, EntityOverflowHex) {
  const char* input =
      "<script contentType=\"application/x-javascript\">"
      "&#xaDBDFFFFF;"
      "&#xafffffffffffffffffffffffffffffffff;"
      "</script>";

  RetainPtr<CFX_SeekableStreamProxy> stream =
      pdfium::MakeRetain<CFX_SeekableStreamProxy>(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  auto root = pdfium::MakeUnique<CFX_XMLNode>();
  CFX_XMLTestParser parser(root.get(), stream);
  ASSERT_EQ(FX_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FX_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"  ", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFX_XMLParserTest, EntityOverflowDecimal) {
  const char* input =
      "<script contentType=\"application/x-javascript\">"
      "&#2914910205;"
      "&#29149102052342342134521341234512351234213452315;"
      "</script>";

  RetainPtr<CFX_SeekableStreamProxy> stream =
      pdfium::MakeRetain<CFX_SeekableStreamProxy>(
          reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input));
  stream->SetCodePage(FX_CODEPAGE_UTF8);

  auto root = pdfium::MakeUnique<CFX_XMLNode>();
  CFX_XMLTestParser parser(root.get(), stream);
  ASSERT_EQ(FX_XmlSyntaxResult::ElementOpen, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::TagName, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::AttriName, parser.DoSyntaxParse());
  ASSERT_EQ(L"contentType", parser.GetAttributeName());
  ASSERT_EQ(FX_XmlSyntaxResult::AttriValue, parser.DoSyntaxParse());
  ASSERT_EQ(L"application/x-javascript", parser.GetAttributeValue());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementBreak, parser.DoSyntaxParse());
  ASSERT_EQ(FX_XmlSyntaxResult::Text, parser.DoSyntaxParse());
  ASSERT_EQ(L"  ", parser.GetTextData());

  ASSERT_EQ(FX_XmlSyntaxResult::ElementClose, parser.DoSyntaxParse());
  ASSERT_EQ(L"script", parser.GetTagName());

  ASSERT_EQ(FX_XmlSyntaxResult::EndOfString, parser.DoSyntaxParse());
}

TEST(CFX_XMLParserTest, IsXMLNameChar) {
  EXPECT_FALSE(CFX_XMLTestParser::IsXMLNameChar(L'-', true));
  EXPECT_TRUE(CFX_XMLTestParser::IsXMLNameChar(L'-', false));

  EXPECT_FALSE(CFX_XMLTestParser::IsXMLNameChar(0x2069, true));
  EXPECT_TRUE(CFX_XMLTestParser::IsXMLNameChar(0x2070, true));
  EXPECT_TRUE(CFX_XMLTestParser::IsXMLNameChar(0x2073, true));
  EXPECT_TRUE(CFX_XMLTestParser::IsXMLNameChar(0x218F, true));
  EXPECT_FALSE(CFX_XMLTestParser::IsXMLNameChar(0x2190, true));

  EXPECT_FALSE(CFX_XMLTestParser::IsXMLNameChar(0xFDEF, true));
  EXPECT_TRUE(CFX_XMLTestParser::IsXMLNameChar(0xFDF0, true));
  EXPECT_TRUE(CFX_XMLTestParser::IsXMLNameChar(0xFDF1, true));
  EXPECT_TRUE(CFX_XMLTestParser::IsXMLNameChar(0xFFFD, true));
  EXPECT_FALSE(CFX_XMLTestParser::IsXMLNameChar(0xFFFE, true));
}
