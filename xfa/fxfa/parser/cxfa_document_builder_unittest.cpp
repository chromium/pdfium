// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/parser/cxfa_document_builder.h"

#include <memory>

#include "core/fxcrt/cfx_read_only_span_stream.h"
#include "core/fxcrt/xml/cfx_xmldocument.h"
#include "core/fxcrt/xml/cfx_xmlparser.h"
#include "testing/fxgc_unittest.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "v8/include/cppgc/allocation.h"
#include "v8/include/cppgc/persistent.h"
#include "xfa/fxfa/parser/cxfa_document.h"

class CXFA_DocumentBuilderTest : public FXGCUnitTest {
 public:
  void SetUp() override {
    FXGCUnitTest::SetUp();
    doc_ = cppgc::MakeGarbageCollected<CXFA_Document>(
        heap()->GetAllocationHandle(), nullptr, heap(), nullptr);
  }

  void TearDown() override {
    doc_ = nullptr;
    FXGCUnitTest::TearDown();
  }

  CXFA_Document* GetDoc() const { return doc_; }

  CXFA_Node* ParseAndBuild(const RetainPtr<CFX_ReadOnlySpanStream>& stream) {
    xml_ = CFX_XMLParser(stream).Parse();
    if (!xml_)
      return nullptr;

    CXFA_DocumentBuilder builder(doc_);
    if (!builder.BuildDocument(xml_.get(), XFA_PacketType::Config))
      return nullptr;
    return builder.GetRootNode();
  }

 private:
  std::unique_ptr<CFX_XMLDocument> xml_;
  cppgc::Persistent<CXFA_Document> doc_;
};

TEST_F(CXFA_DocumentBuilderTest, EmptyInput) {
  static const char kInput[] = "";
  auto stream = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
      pdfium::as_bytes(pdfium::make_span(kInput)));
  EXPECT_FALSE(ParseAndBuild(stream));
}

TEST_F(CXFA_DocumentBuilderTest, BadInput) {
  static const char kInput[] = "<<<>bar?>>>>>>>";
  auto stream = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
      pdfium::as_bytes(pdfium::make_span(kInput)));
  EXPECT_FALSE(ParseAndBuild(stream));
}

TEST_F(CXFA_DocumentBuilderTest, XMLInstructionsScriptOff) {
  static const char kInput[] =
      "<config>\n"
      "<?originalXFAVersion http://www.xfa.org/schema/xfa-template/2.7 "
      "v2.7-scripting:0 ?>\n"
      "</config>";
  EXPECT_FALSE(GetDoc()->is_scripting());

  auto stream = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
      pdfium::as_bytes(pdfium::make_span(kInput)));

  CXFA_Node* root = ParseAndBuild(stream);
  ASSERT_TRUE(root);
  EXPECT_FALSE(GetDoc()->is_scripting());
}

TEST_F(CXFA_DocumentBuilderTest, XMLInstructionsScriptOn) {
  static const char kInput[] =
      "<config>\n"
      "<?originalXFAVersion http://www.xfa.org/schema/xfa-template/2.7 "
      "v2.7-scripting:1 ?>\n"
      "</config>";

  EXPECT_FALSE(GetDoc()->is_scripting());

  auto stream = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
      pdfium::as_bytes(pdfium::make_span(kInput)));

  CXFA_Node* root = ParseAndBuild(stream);
  ASSERT_TRUE(root);
  EXPECT_TRUE(GetDoc()->is_scripting());
}

TEST_F(CXFA_DocumentBuilderTest, XMLInstructionsStrictScope) {
  static const char kInput[] =
      "<config>"
      "<?acrobat JavaScript strictScoping ?>\n"
      "</config>";

  EXPECT_FALSE(GetDoc()->is_strict_scoping());

  auto stream = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
      pdfium::as_bytes(pdfium::make_span(kInput)));

  CXFA_Node* root = ParseAndBuild(stream);
  ASSERT_TRUE(root);
  EXPECT_TRUE(GetDoc()->is_strict_scoping());
}

TEST_F(CXFA_DocumentBuilderTest, XMLInstructionsStrictScopeBad) {
  static const char kInput[] =
      "<config>"
      "<?acrobat JavaScript otherScoping ?>\n"
      "</config>";

  EXPECT_FALSE(GetDoc()->is_strict_scoping());

  auto stream = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
      pdfium::as_bytes(pdfium::make_span(kInput)));

  CXFA_Node* root = ParseAndBuild(stream);
  ASSERT_TRUE(root);
  EXPECT_FALSE(GetDoc()->is_strict_scoping());
}

TEST_F(CXFA_DocumentBuilderTest, MultipleXMLInstructions) {
  static const char kInput[] =
      "<config>"
      "<?originalXFAVersion http://www.xfa.org/schema/xfa-template/2.7 "
      "v2.7-scripting:1 ?>\n"
      "<?acrobat JavaScript strictScoping ?>\n"
      "</config>";

  EXPECT_FALSE(GetDoc()->is_scripting());
  EXPECT_FALSE(GetDoc()->is_strict_scoping());

  auto stream = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
      pdfium::as_bytes(pdfium::make_span(kInput)));

  CXFA_Node* root = ParseAndBuild(stream);
  ASSERT_TRUE(root);
  EXPECT_TRUE(GetDoc()->is_scripting());
  EXPECT_TRUE(GetDoc()->is_strict_scoping());
}
