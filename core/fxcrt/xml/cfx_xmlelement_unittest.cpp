// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlchardata.h"
#include "core/fxcrt/xml/cfx_xmldocument.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/string_write_stream.h"

TEST(CFXXMLElementTest, GetType) {
  CFX_XMLElement node(L"node");
  EXPECT_EQ(CFX_XMLNode::Type::kElement, node.GetType());
}

TEST(CFXXMLElementTest, GetName) {
  CFX_XMLElement node(L"node");
  EXPECT_EQ(L"node", node.GetName());
}

TEST(CFXXMLElementTest, GetLocalTagName) {
  CFX_XMLElement node1(L"node1");
  EXPECT_EQ(L"node1", node1.GetLocalTagName());

  CFX_XMLElement node2(L"test:node2");
  EXPECT_EQ(L"node2", node2.GetLocalTagName());
}

TEST(CFXXMLElementTest, GetNamespacePrefix) {
  CFX_XMLElement node1(L"node1");
  EXPECT_EQ(L"", node1.GetNamespacePrefix());

  CFX_XMLElement node2(L"test:node2");
  EXPECT_EQ(L"test", node2.GetNamespacePrefix());
}

TEST(CFXXMLElementTest, GetNamespaceURI) {
  CFX_XMLElement node1(L"node1");
  EXPECT_EQ(L"", node1.GetNamespaceURI());

  node1.SetAttribute(L"xmlns", L"https://example.org/ns1");
  EXPECT_EQ(L"https://example.org/ns1", node1.GetNamespaceURI());

  CFX_XMLElement node2(L"test:node2");
  EXPECT_EQ(L"", node2.GetNamespaceURI());

  node2.SetAttribute(L"xmlns", L"https://example.org/ns2");
  EXPECT_EQ(L"", node2.GetNamespaceURI());

  node2.SetAttribute(L"xmlns:test", L"https://example.org/ns2");
  EXPECT_EQ(L"https://example.org/ns2", node2.GetNamespaceURI());
}

TEST(CFXXMLElementTest, Attributes) {
  CFX_XMLElement node(L"test:node");
  node.SetAttribute(L"first", L"one");
  node.SetAttribute(L"second", L"two");

  ASSERT_TRUE(node.HasAttribute(L"first"));
  EXPECT_EQ(L"one", node.GetAttribute(L"first"));
  ASSERT_TRUE(node.HasAttribute(L"second"));
  EXPECT_EQ(L"two", node.GetAttribute(L"second"));

  ASSERT_EQ(2U, node.GetAttributes().size());

  node.RemoveAttribute(L"first");
  EXPECT_FALSE(node.HasAttribute(L"first"));

  ASSERT_EQ(1U, node.GetAttributes().size());
}

TEST(CFXXMLElementTest, Clone) {
  CFX_XMLDocument doc;
  CFX_XMLElement node(L"test:node");
  node.SetAttribute(L"first", L"one");
  node.SetAttribute(L"second", L"two");
  node.SetAttribute(L"xmlns:test", L"https://example.org/test");

  CFX_XMLText text_child1(L"Text Child");
  node.AppendLastChild(&text_child1);

  CFX_XMLElement node_child1(L"Node child");
  node.AppendLastChild(&node_child1);

  CFX_XMLNode* clone = node.Clone(&doc);
  EXPECT_TRUE(clone != nullptr);
  ASSERT_EQ(CFX_XMLNode::Type::kElement, clone->GetType());

  CFX_XMLElement* inst = ToXMLElement(clone);
  EXPECT_EQ(L"test:node", inst->GetName());
  EXPECT_EQ(L"node", inst->GetLocalTagName());
  EXPECT_EQ(L"test", inst->GetNamespacePrefix());
  EXPECT_EQ(L"https://example.org/test", inst->GetNamespaceURI());

  ASSERT_TRUE(inst->HasAttribute(L"first"));
  EXPECT_EQ(L"one", inst->GetAttribute(L"first"));
  ASSERT_TRUE(inst->HasAttribute(L"second"));
  EXPECT_EQ(L"two", inst->GetAttribute(L"second"));

  // Only clone the Text node, so expect only one child.
  ASSERT_TRUE(inst->GetFirstChild() != nullptr);
  EXPECT_TRUE(inst->GetFirstChild()->GetNextSibling() == nullptr);

  ASSERT_EQ(CFX_XMLNode::Type::kText, inst->GetFirstChild()->GetType());
  auto* text = ToXMLText(inst->GetFirstChild());
  EXPECT_EQ(L"Text Child", text->GetText());
}

TEST(CFXXMLElementTest, Save) {
  auto stream = pdfium::MakeRetain<StringWriteStream>();
  CFX_XMLElement node(L"root");

  node.Save(stream);
  EXPECT_EQ("<root />\n", stream->ToString());
}

TEST(CFXXMLElementTest, SaveWithAttributes) {
  auto stream = pdfium::MakeRetain<StringWriteStream>();
  CFX_XMLElement node(L"root");
  node.SetAttribute(L"first", L"one");
  node.SetAttribute(L"second", L"two");

  node.Save(stream);
  EXPECT_EQ("<root first=\"one\" second=\"two\" />\n", stream->ToString());
}

TEST(CFXXMLElementTest, SaveWithChildren) {
  auto stream = pdfium::MakeRetain<StringWriteStream>();
  CFX_XMLElement node(L"node");

  CFX_XMLText text_child1(L"Text Child 1");
  node.AppendLastChild(&text_child1);

  CFX_XMLElement node_child1(L"node-child");
  node.AppendLastChild(&node_child1);

  CFX_XMLText text_child2(L"Text Child 2");
  node_child1.AppendLastChild(&text_child2);

  CFX_XMLCharData char_data1(L"Char Data");
  node.AppendLastChild(&char_data1);

  node.Save(stream);
  EXPECT_EQ(
      "<node>\n"
      "Text Child 1"
      "<node-child>\nText Child 2</node-child>\n"
      "<![CDATA[Char Data]]>"
      "</node>\n",
      stream->ToString());
}

TEST(CFXXMLElementTest, SaveWithNamespace) {
  auto stream = pdfium::MakeRetain<StringWriteStream>();
  CFX_XMLElement node(L"test:root");
  node.SetAttribute(L"xmlns:test", L"https://example.org/ns1");

  node.Save(stream);
  EXPECT_EQ("<test:root xmlns:test=\"https://example.org/ns1\" />\n",
            stream->ToString());
}

TEST(CFXXMLElementTest, GetFirstChildNamed) {
  CFX_XMLElement node(L"node");
  CFX_XMLElement node_child1(L"node-child");
  node.AppendLastChild(&node_child1);

  auto* found = node.GetFirstChildNamed(L"node-child");
  EXPECT_TRUE(found != nullptr);
  EXPECT_EQ(&node_child1, found);
}

TEST(CFXXMLElementTest, GetFirstChildNamedMissing) {
  CFX_XMLElement node(L"node");
  CFX_XMLElement node_child1(L"node-child");
  node.AppendLastChild(&node_child1);

  auto* found = node.GetFirstChildNamed(L"node-sibling");
  EXPECT_TRUE(found == nullptr);
}

TEST(CFXXMLElementTest, GetNthChildNamed) {
  CFX_XMLElement node(L"node");
  CFX_XMLElement node_child1(L"node-child");
  CFX_XMLElement node_child2(L"node-child");
  CFX_XMLElement node_child3(L"node-child");
  node.AppendLastChild(&node_child1);
  node.AppendLastChild(&node_child2);
  node.AppendLastChild(&node_child3);

  auto* found = node.GetNthChildNamed(L"node-child", 2);
  EXPECT_TRUE(found != nullptr);
  EXPECT_EQ(&node_child3, found);
}

TEST(CFXXMLElementTest, GetNthChildNamedMissingChild) {
  CFX_XMLElement node(L"node");
  CFX_XMLElement node_child1(L"node-child");
  CFX_XMLElement node_child2(L"node-child");
  CFX_XMLElement node_child3(L"node-child");
  node.AppendLastChild(&node_child1);
  node.AppendLastChild(&node_child2);
  node.AppendLastChild(&node_child3);

  auto* found = node.GetNthChildNamed(L"node-child", 5);
  EXPECT_TRUE(found == nullptr);
}

TEST(CFXXMLElementTest, GetTextData) {
  CFX_XMLElement node(L"node");

  CFX_XMLText text_child1(L"Text Child 1");
  node.AppendLastChild(&text_child1);

  CFX_XMLElement node_child1(L"Node child");
  node.AppendLastChild(&node_child1);

  CFX_XMLText text_child2(L"Text Child 2");
  node_child1.AppendLastChild(&text_child2);

  CFX_XMLCharData char_data1(L"Char Data");
  node.AppendLastChild(&char_data1);

  EXPECT_EQ(L"Text Child 1Char Data", node.GetTextData());
}
