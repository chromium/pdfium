// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"
#include "third_party/base/ptr_util.h"

TEST(CFX_XMLNodeTest, GetParent) {
  auto node1 = pdfium::MakeUnique<CFX_XMLElement>(L"node");
  auto child2 = pdfium::MakeUnique<CFX_XMLElement>(L"node2");
  auto child3 = pdfium::MakeUnique<CFX_XMLElement>(L"node3");

  CFX_XMLElement* node2 = child2.get();
  CFX_XMLElement* node3 = child3.get();

  node1->AppendChild(std::move(child2));
  node2->AppendChild(std::move(child3));

  EXPECT_EQ(nullptr, node1->GetParent());
  EXPECT_EQ(node1.get(), node2->GetParent());
  EXPECT_EQ(node2, node3->GetParent());
}

TEST(CFX_XMLNodeTest, GetRoot) {
  auto node1 = pdfium::MakeUnique<CFX_XMLElement>(L"node");
  auto child2 = pdfium::MakeUnique<CFX_XMLElement>(L"node2");
  auto child3 = pdfium::MakeUnique<CFX_XMLElement>(L"node3");

  CFX_XMLElement* node2 = child2.get();
  CFX_XMLElement* node3 = child3.get();

  node1->AppendChild(std::move(child2));
  node2->AppendChild(std::move(child3));

  EXPECT_EQ(node1.get(), node1->GetRoot());
  EXPECT_EQ(node1.get(), node2->GetRoot());
  EXPECT_EQ(node1.get(), node3->GetRoot());
}

TEST(CFX_XMLNodeTest, GetChildren) {
  auto node1 = pdfium::MakeUnique<CFX_XMLElement>(L"node");
  auto child2 = pdfium::MakeUnique<CFX_XMLElement>(L"node2");
  auto child3 = pdfium::MakeUnique<CFX_XMLElement>(L"node3");
  auto child4 = pdfium::MakeUnique<CFX_XMLElement>(L"node4");

  CFX_XMLElement* node2 = child2.get();
  CFX_XMLElement* node3 = child3.get();
  CFX_XMLElement* node4 = child4.get();

  node1->AppendChild(std::move(child2));
  node1->AppendChild(std::move(child4));
  node2->AppendChild(std::move(child3));

  EXPECT_EQ(node2, node1->GetFirstChild());

  EXPECT_EQ(node4, node2->GetNextSibling());
  EXPECT_EQ(node3, node2->GetFirstChild());

  EXPECT_TRUE(node3->GetNextSibling() == nullptr);
  EXPECT_TRUE(node3->GetFirstChild() == nullptr);

  EXPECT_TRUE(node4->GetNextSibling() == nullptr);
  EXPECT_TRUE(node4->GetFirstChild() == nullptr);
}

TEST(CFX_XMLNodeTest, DeleteChildren) {
  auto node1 = pdfium::MakeUnique<CFX_XMLElement>(L"node");
  auto child2 = pdfium::MakeUnique<CFX_XMLElement>(L"node2");
  auto child3 = pdfium::MakeUnique<CFX_XMLElement>(L"node3");
  auto child4 = pdfium::MakeUnique<CFX_XMLElement>(L"node4");

  CFX_XMLElement* node2 = child2.get();
  // CFX_XMLElement* node3 = child3.get();
  // CFX_XMLElement* node4 = child4.get();

  node1->AppendChild(std::move(child2));
  node1->AppendChild(std::move(child4));
  node2->AppendChild(std::move(child3));

  node1->DeleteChildren();
  EXPECT_TRUE(node1->GetFirstChild() == nullptr);
  EXPECT_TRUE(node1->GetLastChildForTesting() == nullptr);

  // TODO(dsinclair): This isn't true currently but will be true when
  // we own the nodes in an XML document. (Currently nodes are unique_ptrs
  // so the objects have been deleted by this point.)

  // EXPECT_TRUE(node2->GetParent() == nullptr);
  // EXPECT_TRUE(node4->GetParent() == nullptr);

  // // node2 and node4 should no longer be siblings.
  // EXPECT_TRUE(node2->GetNextSibling() == nullptr);
  // EXPECT_TRUE(node4->GetPrevSiblingForTesting() == nullptr);

  // Deleting children doesn't change deleted substructure
  // EXPECT_EQ(node3, node2->GetFirstChild());
  // EXPECT_TRUE(node3->GetParent() == node2);
}

TEST(CFX_XMLNodeTest, AddingChildren) {
  auto node1 = pdfium::MakeUnique<CFX_XMLElement>(L"node");
  auto child2 = pdfium::MakeUnique<CFX_XMLElement>(L"node2");
  auto child3 = pdfium::MakeUnique<CFX_XMLElement>(L"node3");
  auto child4 = pdfium::MakeUnique<CFX_XMLElement>(L"node4");
  auto child5 = pdfium::MakeUnique<CFX_XMLElement>(L"node5");

  CFX_XMLElement* node2 = child2.get();
  CFX_XMLElement* node3 = child3.get();
  CFX_XMLElement* node4 = child4.get();
  CFX_XMLElement* node5 = child5.get();

  node1->AppendChild(std::move(child2));
  node1->AppendChild(std::move(child3));

  EXPECT_EQ(node1.get(), node2->GetParent());
  EXPECT_EQ(node1.get(), node3->GetParent());

  EXPECT_EQ(node2, node1->GetFirstChild());
  EXPECT_EQ(node3, node2->GetNextSibling());
  EXPECT_TRUE(node3->GetNextSibling() == nullptr);

  // Insert to negative appends.
  node1->InsertChildNode(std::move(child4), -1);
  EXPECT_EQ(node1.get(), node4->GetParent());
  EXPECT_EQ(node4, node3->GetNextSibling());
  EXPECT_TRUE(node4->GetNextSibling() == nullptr);

  node1->InsertChildNode(std::move(child5), 1);
  EXPECT_EQ(node1.get(), node5->GetParent());
  EXPECT_EQ(node2, node1->GetFirstChild());
  EXPECT_EQ(node5, node2->GetNextSibling());
  EXPECT_EQ(node3, node5->GetNextSibling());
  EXPECT_EQ(node4, node3->GetNextSibling());
  EXPECT_TRUE(node4->GetNextSibling() == nullptr);
}

TEST(CFX_XMLNodeTest, RemovingMiddleChild) {
  auto node1 = pdfium::MakeUnique<CFX_XMLElement>(L"node");
  auto child2 = pdfium::MakeUnique<CFX_XMLElement>(L"node2");
  auto child3 = pdfium::MakeUnique<CFX_XMLElement>(L"node3");
  auto child4 = pdfium::MakeUnique<CFX_XMLElement>(L"node4");

  CFX_XMLElement* node2 = child2.get();
  CFX_XMLElement* node3 = child3.get();
  CFX_XMLElement* node4 = child4.get();

  node1->AppendChild(std::move(child2));
  node1->AppendChild(std::move(child3));
  node1->AppendChild(std::move(child4));

  EXPECT_EQ(node2, node1->GetFirstChild());
  EXPECT_EQ(node3, node2->GetNextSibling());
  EXPECT_EQ(node4, node3->GetNextSibling());
  EXPECT_TRUE(node4->GetNextSibling() == nullptr);

  node1->RemoveChildNode(node3);
  // Node is released by parent, so need to take ownership
  child3 = pdfium::WrapUnique(node3);

  EXPECT_TRUE(node3->GetParent() == nullptr);
  EXPECT_TRUE(node3->GetNextSibling() == nullptr);
  EXPECT_TRUE(node3->GetPrevSiblingForTesting() == nullptr);

  EXPECT_EQ(node2, node1->GetFirstChild());
  EXPECT_EQ(node4, node2->GetNextSibling());
  EXPECT_EQ(node2, node4->GetPrevSiblingForTesting());
  EXPECT_TRUE(node4->GetNextSibling() == nullptr);
}

TEST(CFX_XMLNodeTest, RemovingFirstChild) {
  auto node1 = pdfium::MakeUnique<CFX_XMLElement>(L"node");
  auto child2 = pdfium::MakeUnique<CFX_XMLElement>(L"node2");
  auto child3 = pdfium::MakeUnique<CFX_XMLElement>(L"node3");
  auto child4 = pdfium::MakeUnique<CFX_XMLElement>(L"node4");

  CFX_XMLElement* node2 = child2.get();
  CFX_XMLElement* node3 = child3.get();
  CFX_XMLElement* node4 = child4.get();

  node1->AppendChild(std::move(child2));
  node1->AppendChild(std::move(child3));
  node1->AppendChild(std::move(child4));

  EXPECT_EQ(node2, node1->GetFirstChild());
  EXPECT_EQ(node3, node2->GetNextSibling());
  EXPECT_EQ(node4, node3->GetNextSibling());
  EXPECT_TRUE(node4->GetNextSibling() == nullptr);

  node1->RemoveChildNode(node2);
  // Node is released by parent, so need to take ownership
  child2 = pdfium::WrapUnique(node2);

  EXPECT_TRUE(node2->GetParent() == nullptr);
  EXPECT_TRUE(node2->GetNextSibling() == nullptr);
  EXPECT_TRUE(node2->GetPrevSiblingForTesting() == nullptr);

  EXPECT_EQ(node3, node1->GetFirstChild());
  EXPECT_TRUE(node3->GetPrevSiblingForTesting() == nullptr);
  EXPECT_EQ(node4, node3->GetNextSibling());
  EXPECT_TRUE(node4->GetNextSibling() == nullptr);
}

TEST(CFX_XMLNodeTest, RemovingLastChild) {
  auto node1 = pdfium::MakeUnique<CFX_XMLElement>(L"node");
  auto child2 = pdfium::MakeUnique<CFX_XMLElement>(L"node2");
  auto child3 = pdfium::MakeUnique<CFX_XMLElement>(L"node3");
  auto child4 = pdfium::MakeUnique<CFX_XMLElement>(L"node4");

  CFX_XMLElement* node2 = child2.get();
  CFX_XMLElement* node3 = child3.get();
  CFX_XMLElement* node4 = child4.get();

  node1->AppendChild(std::move(child2));
  node1->AppendChild(std::move(child3));
  node1->AppendChild(std::move(child4));

  EXPECT_EQ(node2, node1->GetFirstChild());
  EXPECT_EQ(node3, node2->GetNextSibling());
  EXPECT_EQ(node4, node3->GetNextSibling());
  EXPECT_TRUE(node4->GetNextSibling() == nullptr);

  node1->RemoveChildNode(node4);
  // Node is released by parent, so need to take ownership
  child4 = pdfium::WrapUnique(node4);

  EXPECT_TRUE(node4->GetParent() == nullptr);
  EXPECT_TRUE(node4->GetNextSibling() == nullptr);
  EXPECT_TRUE(node4->GetPrevSiblingForTesting() == nullptr);

  EXPECT_EQ(node2, node1->GetFirstChild());
  EXPECT_EQ(node3, node2->GetNextSibling());
  EXPECT_TRUE(node3->GetNextSibling() == nullptr);
}

TEST(CFX_XMLNodeTest, RemovingOnlyChild) {
  auto node1 = pdfium::MakeUnique<CFX_XMLElement>(L"node");
  auto child2 = pdfium::MakeUnique<CFX_XMLElement>(L"node2");

  CFX_XMLElement* node2 = child2.get();

  node1->AppendChild(std::move(child2));

  EXPECT_EQ(node2, node1->GetFirstChild());
  EXPECT_TRUE(node2->GetNextSibling() == nullptr);

  node1->RemoveChildNode(node2);
  // Node is released by parent, so need to take ownership
  child2 = pdfium::WrapUnique(node2);

  EXPECT_TRUE(node2->GetParent() == nullptr);

  EXPECT_TRUE(node1->GetFirstChild() == nullptr);
  EXPECT_TRUE(node2->GetNextSibling() == nullptr);
  EXPECT_TRUE(node2->GetPrevSiblingForTesting() == nullptr);
}

TEST(CFX_XMLNodeTest, RemoveMissingChild) {
  auto node1 = pdfium::MakeUnique<CFX_XMLElement>(L"node");
  auto child2 = pdfium::MakeUnique<CFX_XMLElement>(L"node2");
  auto child3 = pdfium::MakeUnique<CFX_XMLElement>(L"node3");

  CFX_XMLElement* node2 = child2.get();
  CFX_XMLElement* node3 = child3.get();

  node1->AppendChild(std::move(child2));
  node1->RemoveChildNode(node3);

  EXPECT_TRUE(node3->GetParent() == nullptr);
  EXPECT_EQ(node2, node1->GetFirstChild());
  EXPECT_TRUE(node2->GetNextSibling() == nullptr);
}
