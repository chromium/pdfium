// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/xml/cfx_xmlparser.h"

#include <memory>

#include "core/fxcrt/cfx_read_only_span_stream.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/xml/cfx_xmldocument.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlinstruction.h"
#include "testing/gtest/include/gtest/gtest.h"

class CFXXMLParserTest : public testing::Test {
 public:
  std::unique_ptr<CFX_XMLDocument> Parse(pdfium::span<const char> input) {
    CFX_XMLParser parser(
        pdfium::MakeRetain<CFX_ReadOnlySpanStream>(pdfium::as_bytes(input)));
    return parser.Parse();
  }
};

TEST_F(CFXXMLParserTest, AttributesMustBeQuoted) {
  static const char input[] =
      "<script display=1>\n"
      "</script>";
  ASSERT_TRUE(Parse(input) == nullptr);
}

TEST_F(CFXXMLParserTest, Attributes) {
  static const char input[] =
      "<script contentType=\"application/x-javascript\" display=\"1\">\n"
      "</script>";

  std::unique_ptr<CFX_XMLDocument> doc = Parse(input);
  ASSERT_TRUE(doc != nullptr);

  CFX_XMLElement* script = doc->GetRoot()->GetFirstChildNamed(L"script");
  ASSERT_TRUE(script != nullptr);

  EXPECT_EQ(L"application/x-javascript", script->GetAttribute(L"contentType"));
  EXPECT_EQ(L"1", script->GetAttribute(L"display"));
}

TEST_F(CFXXMLParserTest, CData) {
  static const char input[] =
      "<script>\n"
      "  <![CDATA[\n"
      "    if (a[1] < 3)\n"
      "      app.alert(\"Tclams\");\n"
      "  ]]>\n"
      "</script>";

  static const wchar_t cdata[] =
      L"\n  \n"
      L"    if (a[1] < 3)\n"
      L"      app.alert(\"Tclams\");\n"
      L"  \n";

  std::unique_ptr<CFX_XMLDocument> doc = Parse(input);
  ASSERT_TRUE(doc != nullptr);

  CFX_XMLElement* script = doc->GetRoot()->GetFirstChildNamed(L"script");
  ASSERT_TRUE(script != nullptr);
  EXPECT_EQ(cdata, script->GetTextData());
}

TEST_F(CFXXMLParserTest, CDataWithInnerScript) {
  static const char input[] =
      "<script>\n"
      "  <![CDATA[\n"
      "    if (a[1] < 3)\n"
      "      app.alert(\"Tclams\");\n"
      "    </script>\n"
      "  ]]>\n"
      "</script>";

  static const wchar_t cdata[] =
      L"\n  \n"
      L"    if (a[1] < 3)\n"
      L"      app.alert(\"Tclams\");\n"
      L"    </script>\n"
      L"  \n";

  std::unique_ptr<CFX_XMLDocument> doc = Parse(input);
  ASSERT_TRUE(doc != nullptr);

  CFX_XMLElement* script = doc->GetRoot()->GetFirstChildNamed(L"script");
  ASSERT_TRUE(script != nullptr);
  EXPECT_EQ(cdata, script->GetTextData());
}

TEST_F(CFXXMLParserTest, ArrowBangArrow) {
  static const char input[] =
      "<script>\n"
      "  <!>\n"
      "</script>";

  std::unique_ptr<CFX_XMLDocument> doc = Parse(input);
  ASSERT_TRUE(doc != nullptr);

  CFX_XMLElement* script = doc->GetRoot()->GetFirstChildNamed(L"script");
  ASSERT_TRUE(script != nullptr);
  EXPECT_EQ(L"\n  \n", script->GetTextData());
}

TEST_F(CFXXMLParserTest, ArrowBangBracketArrow) {
  static const char input[] =
      "<script>\n"
      "  <![>\n"
      "</script>";

  std::unique_ptr<CFX_XMLDocument> doc = Parse(input);
  ASSERT_TRUE(doc != nullptr);

  CFX_XMLElement* script = doc->GetRoot()->GetFirstChildNamed(L"script");
  ASSERT_TRUE(script != nullptr);
  EXPECT_EQ(L"\n  ", script->GetTextData());
}

TEST_F(CFXXMLParserTest, IncompleteCData) {
  static const char input[] =
      "<script>\n"
      "  <![CDATA>\n"
      "</script>";

  std::unique_ptr<CFX_XMLDocument> doc = Parse(input);
  ASSERT_TRUE(doc != nullptr);

  CFX_XMLElement* script = doc->GetRoot()->GetFirstChildNamed(L"script");
  ASSERT_TRUE(script != nullptr);
  EXPECT_EQ(L"\n  ", script->GetTextData());
}

TEST_F(CFXXMLParserTest, UnClosedCData) {
  static const char input[] =
      "<script>\n"
      "  <![CDATA[\n"
      "</script>";

  std::unique_ptr<CFX_XMLDocument> doc = Parse(input);
  ASSERT_TRUE(doc != nullptr);

  CFX_XMLElement* script = doc->GetRoot()->GetFirstChildNamed(L"script");
  ASSERT_TRUE(script != nullptr);
  EXPECT_EQ(L"\n  ", script->GetTextData());
}

TEST_F(CFXXMLParserTest, EmptyCData) {
  static const char input[] =
      "<script>\n"
      "  <![CDATA[]]>\n"
      "</script>";

  std::unique_ptr<CFX_XMLDocument> doc = Parse(input);
  ASSERT_TRUE(doc != nullptr);

  CFX_XMLElement* script = doc->GetRoot()->GetFirstChildNamed(L"script");
  ASSERT_TRUE(script != nullptr);
  EXPECT_EQ(L"\n  \n", script->GetTextData());
}

TEST_F(CFXXMLParserTest, Comment) {
  static const char input[] =
      "<script>\n"
      "  <!-- A Comment -->\n"
      "</script>";

  std::unique_ptr<CFX_XMLDocument> doc = Parse(input);
  ASSERT_TRUE(doc != nullptr);

  CFX_XMLElement* script = doc->GetRoot()->GetFirstChildNamed(L"script");
  ASSERT_TRUE(script != nullptr);
  EXPECT_EQ(L"\n  \n", script->GetTextData());
}

TEST_F(CFXXMLParserTest, IncorrectCommentStart) {
  static const char input[] =
      "<script>\n"
      "  <!- A Comment -->\n"
      "</script>";

  std::unique_ptr<CFX_XMLDocument> doc = Parse(input);
  ASSERT_TRUE(doc != nullptr);

  CFX_XMLElement* script = doc->GetRoot()->GetFirstChildNamed(L"script");
  ASSERT_TRUE(script != nullptr);
  EXPECT_EQ(L"\n  \n", script->GetTextData());
}

TEST_F(CFXXMLParserTest, CommentEmpty) {
  static const char input[] =
      "<script>\n"
      "  <!---->\n"
      "</script>";

  std::unique_ptr<CFX_XMLDocument> doc = Parse(input);
  ASSERT_TRUE(doc != nullptr);

  CFX_XMLElement* script = doc->GetRoot()->GetFirstChildNamed(L"script");
  ASSERT_TRUE(script != nullptr);
  EXPECT_EQ(L"\n  \n", script->GetTextData());
}

TEST_F(CFXXMLParserTest, CommentThreeDash) {
  static const char input[] =
      "<script>\n"
      "  <!--->\n"
      "</script>";

  std::unique_ptr<CFX_XMLDocument> doc = Parse(input);
  ASSERT_TRUE(doc != nullptr);

  CFX_XMLElement* script = doc->GetRoot()->GetFirstChildNamed(L"script");
  ASSERT_TRUE(script != nullptr);
  EXPECT_EQ(L"\n  ", script->GetTextData());
}

TEST_F(CFXXMLParserTest, CommentTwoDash) {
  static const char input[] =
      "<script>\n"
      "  <!-->\n"
      "</script>";

  std::unique_ptr<CFX_XMLDocument> doc = Parse(input);
  ASSERT_TRUE(doc != nullptr);

  CFX_XMLElement* script = doc->GetRoot()->GetFirstChildNamed(L"script");
  EXPECT_EQ(L"\n  ", script->GetTextData());
}

TEST_F(CFXXMLParserTest, Entities) {
  static const char input[] =
      "<script>"
      "&#66;"                     // B
      "&#x54;"                    // T
      "&#x6a;"                    // j
      "&#x00000000000000000048;"  // H
      "&#x0000000000000000AB48;"  // \xab48
      "&#x0000000000000000000;"
      "&amp;"
      "&lt;"
      "&gt;"
      "&apos;"
      "&quot;"
      "&something_else;"
      "</script>";

  std::unique_ptr<CFX_XMLDocument> doc = Parse(input);
  ASSERT_TRUE(doc != nullptr);

  CFX_XMLElement* script = doc->GetRoot()->GetFirstChildNamed(L"script");
  ASSERT_TRUE(script != nullptr);
  EXPECT_EQ(L"BTjH\xab48&<>'\"", script->GetTextData());
}

TEST_F(CFXXMLParserTest, EntityOverflowHex) {
  static const char input[] =
      "<script>"
      "&#xaDBDFFFFF;"
      "&#xafffffffffffffffffffffffffffffffff;"
      "</script>";

  std::unique_ptr<CFX_XMLDocument> doc = Parse(input);
  ASSERT_TRUE(doc != nullptr);

  CFX_XMLElement* script = doc->GetRoot()->GetFirstChildNamed(L"script");
  ASSERT_TRUE(script != nullptr);
  EXPECT_EQ(L"  ", script->GetTextData());
}

TEST_F(CFXXMLParserTest, EntityOverflowDecimal) {
  static const char input[] =
      "<script>"
      "&#2914910205;"
      "&#29149102052342342134521341234512351234213452315;"
      "</script>";

  std::unique_ptr<CFX_XMLDocument> doc = Parse(input);
  ASSERT_TRUE(doc != nullptr);

  CFX_XMLElement* script = doc->GetRoot()->GetFirstChildNamed(L"script");
  ASSERT_TRUE(script != nullptr);
  EXPECT_EQ(L"  ", script->GetTextData());
}

TEST_F(CFXXMLParserTest, IsXMLNameChar) {
  EXPECT_FALSE(CFX_XMLParser::IsXMLNameChar(L'-', true));
  EXPECT_TRUE(CFX_XMLParser::IsXMLNameChar(L'-', false));

  EXPECT_FALSE(CFX_XMLParser::IsXMLNameChar(L'.', true));
  EXPECT_TRUE(CFX_XMLParser::IsXMLNameChar(L'.', false));

  EXPECT_FALSE(CFX_XMLParser::IsXMLNameChar(L'0', true));
  EXPECT_TRUE(CFX_XMLParser::IsXMLNameChar(L'0', false));

  EXPECT_TRUE(CFX_XMLParser::IsXMLNameChar(L'a', true));
  EXPECT_TRUE(CFX_XMLParser::IsXMLNameChar(L'a', false));

  EXPECT_TRUE(CFX_XMLParser::IsXMLNameChar(L'A', true));
  EXPECT_TRUE(CFX_XMLParser::IsXMLNameChar(L'A', false));

  EXPECT_FALSE(CFX_XMLParser::IsXMLNameChar(L'(', false));
  EXPECT_FALSE(CFX_XMLParser::IsXMLNameChar(L'(', true));
  EXPECT_FALSE(CFX_XMLParser::IsXMLNameChar(L')', false));
  EXPECT_FALSE(CFX_XMLParser::IsXMLNameChar(L')', true));
  EXPECT_FALSE(CFX_XMLParser::IsXMLNameChar(L'[', false));
  EXPECT_FALSE(CFX_XMLParser::IsXMLNameChar(L'[', true));
  EXPECT_FALSE(CFX_XMLParser::IsXMLNameChar(L']', false));
  EXPECT_FALSE(CFX_XMLParser::IsXMLNameChar(L']', true));

  EXPECT_FALSE(CFX_XMLParser::IsXMLNameChar(0x2069, true));
  EXPECT_TRUE(CFX_XMLParser::IsXMLNameChar(0x2070, true));
  EXPECT_TRUE(CFX_XMLParser::IsXMLNameChar(0x2073, true));
  EXPECT_TRUE(CFX_XMLParser::IsXMLNameChar(0x218F, true));
  EXPECT_FALSE(CFX_XMLParser::IsXMLNameChar(0x2190, true));

  EXPECT_FALSE(CFX_XMLParser::IsXMLNameChar(0xFDEF, true));
  EXPECT_TRUE(CFX_XMLParser::IsXMLNameChar(0xFDF0, true));
  EXPECT_TRUE(CFX_XMLParser::IsXMLNameChar(0xFDF1, true));
  EXPECT_TRUE(CFX_XMLParser::IsXMLNameChar(0xFFFD, true));
  EXPECT_FALSE(CFX_XMLParser::IsXMLNameChar(0xFFFE, true));
}

TEST_F(CFXXMLParserTest, BadElementClose) {
  ASSERT_TRUE(Parse("</endtag>") == nullptr);
}

TEST_F(CFXXMLParserTest, DoubleElementClose) {
  ASSERT_TRUE(Parse("<p></p></p>") == nullptr);
}

TEST_F(CFXXMLParserTest, ParseInstruction) {
  static const char input[] =
      "<?originalXFAVersion http://www.xfa.org/schema/xfa-template/3.3/ ?>"
      "<form></form>";

  std::unique_ptr<CFX_XMLDocument> doc = Parse(input);
  ASSERT_TRUE(doc != nullptr);

  CFX_XMLElement* root = doc->GetRoot();
  ASSERT_TRUE(root->GetFirstChild() != nullptr);
  ASSERT_EQ(CFX_XMLNode::Type::kInstruction, root->GetFirstChild()->GetType());

  CFX_XMLInstruction* instruction = ToXMLInstruction(root->GetFirstChild());
  EXPECT_TRUE(instruction->IsOriginalXFAVersion());
}

TEST_F(CFXXMLParserTest, BadEntity) {
  static const char input[] =
      "<script>"
      "Test &<p>; thing"
      "</script>";
  ASSERT_TRUE(Parse(input) == nullptr);
}
