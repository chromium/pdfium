// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

WideString ChildrenString(CFX_XMLElement* pParent) {
  WideString result;
  for (CFX_XMLNode* pChild = pParent->GetFirstChild(); pChild;
       pChild = pChild->GetNextSibling()) {
    result += static_cast<CFX_XMLElement*>(pChild)->GetName();
  }
  return result;
}

WideString ReverseChildrenString(CFX_XMLElement* pParent) {
  WideString result;
  for (CFX_XMLNode* pChild = pParent->GetLastChild(); pChild;
       pChild = pChild->GetPrevSibling()) {
    result = static_cast<CFX_XMLElement*>(pChild)->GetName() + result;
  }
  return result;
}

}  // namespace

TEST(CFX_XMLNodeTest, GetParent) {
  CFX_XMLElement node1(L"node");
  CFX_XMLElement node2(L"node2");
  CFX_XMLElement node3(L"node3");

  node1.AppendLastChild(&node2);
  node2.AppendLastChild(&node3);

  EXPECT_EQ(nullptr, node1.GetParent());
  EXPECT_EQ(&node1, node2.GetParent());
  EXPECT_EQ(&node2, node3.GetParent());
}

TEST(CFX_XMLNodeTest, GetRoot) {
  CFX_XMLElement node1(L"node");
  CFX_XMLElement node2(L"node2");
  CFX_XMLElement node3(L"node3");

  node1.AppendLastChild(&node2);
  node2.AppendLastChild(&node3);

  EXPECT_EQ(&node1, node1.GetRoot());
  EXPECT_EQ(&node1, node2.GetRoot());
  EXPECT_EQ(&node1, node3.GetRoot());
}

TEST(CFX_XMLNodeTest, GetChildren) {
  CFX_XMLElement node1(L"node");
  CFX_XMLElement node2(L"node2");
  CFX_XMLElement node3(L"node3");
  CFX_XMLElement node4(L"node4");

  node1.AppendLastChild(&node2);
  node1.AppendLastChild(&node4);
  node2.AppendLastChild(&node3);

  EXPECT_EQ(&node2, node1.GetFirstChild());

  EXPECT_EQ(&node4, node2.GetNextSibling());
  EXPECT_EQ(&node3, node2.GetFirstChild());

  EXPECT_TRUE(node3.GetNextSibling() == nullptr);
  EXPECT_TRUE(node3.GetFirstChild() == nullptr);

  EXPECT_TRUE(node4.GetNextSibling() == nullptr);
  EXPECT_TRUE(node4.GetFirstChild() == nullptr);
}

TEST(CFX_XMLNodeTest, DeleteChildren) {
  CFX_XMLElement node1(L"node");
  CFX_XMLElement node2(L"node2");
  CFX_XMLElement node3(L"node3");
  CFX_XMLElement node4(L"node4");

  node1.AppendLastChild(&node2);
  node1.AppendLastChild(&node4);
  node2.AppendLastChild(&node3);

  node1.RemoveAllChildren();
  EXPECT_TRUE(node1.GetFirstChild() == nullptr);
  EXPECT_TRUE(node2.GetParent() == nullptr);
  EXPECT_TRUE(node4.GetParent() == nullptr);

  // node2 and node4 should no longer be siblings.
  EXPECT_TRUE(node2.GetNextSibling() == nullptr);
  EXPECT_TRUE(node4.GetPrevSibling() == nullptr);

  // Deleting children doesn't change deleted substructure
  EXPECT_EQ(&node3, node2.GetFirstChild());
  EXPECT_TRUE(node3.GetParent() == &node2);
}

TEST(CFX_XMLNodeTest, AddingChildren) {
  CFX_XMLElement parent(L"Root");
  CFX_XMLElement nodeA(L"A");
  CFX_XMLElement nodeB(L"B");

  parent.AppendLastChild(&nodeA);
  parent.AppendLastChild(&nodeB);

  EXPECT_EQ(L"AB", ChildrenString(&parent));
  EXPECT_EQ(L"AB", ReverseChildrenString(&parent));
  EXPECT_EQ(&parent, nodeA.GetParent());
  EXPECT_EQ(&parent, nodeB.GetParent());
  EXPECT_EQ(&nodeA, parent.GetFirstChild());
  EXPECT_EQ(&nodeB, nodeA.GetNextSibling());
  EXPECT_TRUE(nodeB.GetNextSibling() == nullptr);

  // Insert to negative appends last child.
  CFX_XMLElement nodeC(L"C");
  parent.InsertChildNode(&nodeC, -1);
  EXPECT_EQ(L"ABC", ChildrenString(&parent));
  EXPECT_EQ(L"ABC", ReverseChildrenString(&parent));
  EXPECT_EQ(&parent, nodeC.GetParent());
  EXPECT_EQ(&nodeC, nodeB.GetNextSibling());
  EXPECT_TRUE(nodeC.GetNextSibling() == nullptr);

  // Insertion occurs before a zero based index.
  CFX_XMLElement nodeD(L"D");
  parent.InsertChildNode(&nodeD, 1);
  EXPECT_EQ(L"ADBC", ChildrenString(&parent));
  EXPECT_EQ(L"ADBC", ReverseChildrenString(&parent));

  // Insert to 0 appends first child.
  CFX_XMLElement nodeE(L"E");
  parent.InsertChildNode(&nodeE, 0);
  EXPECT_EQ(L"EADBC", ChildrenString(&parent));
  EXPECT_EQ(L"EADBC", ReverseChildrenString(&parent));

  // Insert to out-of-bounds index appends last child.
  CFX_XMLElement nodeF(L"F");
  parent.InsertChildNode(&nodeF, 10);
  EXPECT_EQ(L"EADBCF", ChildrenString(&parent));
  EXPECT_EQ(L"EADBCF", ReverseChildrenString(&parent));
}

TEST(CFX_XMLNodeTest, RemovingMiddleChild) {
  CFX_XMLElement node1(L"node1");
  CFX_XMLElement node2(L"node2");
  CFX_XMLElement node3(L"node3");
  CFX_XMLElement node4(L"node4");

  node1.AppendLastChild(&node2);
  node1.AppendLastChild(&node3);
  node1.AppendLastChild(&node4);

  EXPECT_EQ(L"node2node3node4", ChildrenString(&node1));
  EXPECT_EQ(L"node2node3node4", ReverseChildrenString(&node1));
  EXPECT_EQ(&node2, node1.GetFirstChild());
  EXPECT_EQ(&node3, node2.GetNextSibling());
  EXPECT_EQ(&node4, node3.GetNextSibling());
  EXPECT_TRUE(node4.GetNextSibling() == nullptr);

  node1.RemoveChild(&node3);

  EXPECT_EQ(L"node2node4", ChildrenString(&node1));
  EXPECT_EQ(L"node2node4", ReverseChildrenString(&node1));
  EXPECT_TRUE(node3.GetParent() == nullptr);
  EXPECT_TRUE(node3.GetNextSibling() == nullptr);
  EXPECT_TRUE(node3.GetPrevSibling() == nullptr);
  EXPECT_EQ(&node2, node1.GetFirstChild());
  EXPECT_EQ(&node4, node2.GetNextSibling());
  EXPECT_EQ(&node2, node4.GetPrevSibling());
  EXPECT_TRUE(node4.GetNextSibling() == nullptr);
}

TEST(CFX_XMLNodeTest, RemovingFirstChild) {
  CFX_XMLElement node1(L"node1");
  CFX_XMLElement node2(L"node2");
  CFX_XMLElement node3(L"node3");
  CFX_XMLElement node4(L"node4");

  node1.AppendLastChild(&node2);
  node1.AppendLastChild(&node3);
  node1.AppendLastChild(&node4);

  EXPECT_EQ(L"node2node3node4", ChildrenString(&node1));
  EXPECT_EQ(L"node2node3node4", ReverseChildrenString(&node1));
  EXPECT_EQ(&node2, node1.GetFirstChild());
  EXPECT_EQ(&node3, node2.GetNextSibling());
  EXPECT_EQ(&node4, node3.GetNextSibling());
  EXPECT_TRUE(node4.GetNextSibling() == nullptr);

  node1.RemoveChild(&node2);

  EXPECT_EQ(L"node3node4", ChildrenString(&node1));
  EXPECT_EQ(L"node3node4", ReverseChildrenString(&node1));
  EXPECT_TRUE(node2.GetParent() == nullptr);
  EXPECT_TRUE(node2.GetNextSibling() == nullptr);
  EXPECT_TRUE(node2.GetPrevSibling() == nullptr);
  EXPECT_EQ(&node3, node1.GetFirstChild());
  EXPECT_TRUE(node3.GetPrevSibling() == nullptr);
  EXPECT_EQ(&node4, node3.GetNextSibling());
  EXPECT_TRUE(node4.GetNextSibling() == nullptr);
}

TEST(CFX_XMLNodeTest, RemovingLastChild) {
  CFX_XMLElement node1(L"node1");
  CFX_XMLElement node2(L"node2");
  CFX_XMLElement node3(L"node3");
  CFX_XMLElement node4(L"node4");

  node1.AppendLastChild(&node2);
  node1.AppendLastChild(&node3);
  node1.AppendLastChild(&node4);

  EXPECT_EQ(L"node2node3node4", ChildrenString(&node1));
  EXPECT_EQ(L"node2node3node4", ReverseChildrenString(&node1));
  EXPECT_EQ(&node2, node1.GetFirstChild());
  EXPECT_EQ(&node3, node2.GetNextSibling());
  EXPECT_EQ(&node4, node3.GetNextSibling());
  EXPECT_TRUE(node4.GetNextSibling() == nullptr);

  node1.RemoveChild(&node4);

  EXPECT_EQ(L"node2node3", ChildrenString(&node1));
  EXPECT_EQ(L"node2node3", ReverseChildrenString(&node1));
  EXPECT_TRUE(node4.GetParent() == nullptr);
  EXPECT_TRUE(node4.GetNextSibling() == nullptr);
  EXPECT_TRUE(node4.GetPrevSibling() == nullptr);
  EXPECT_EQ(&node2, node1.GetFirstChild());
  EXPECT_EQ(&node3, node2.GetNextSibling());
  EXPECT_TRUE(node3.GetNextSibling() == nullptr);
}

TEST(CFX_XMLNodeTest, RemovingOnlyChild) {
  CFX_XMLElement node1(L"node1");
  CFX_XMLElement node2(L"node2");

  node1.AppendLastChild(&node2);

  EXPECT_EQ(&node2, node1.GetFirstChild());
  EXPECT_TRUE(node2.GetNextSibling() == nullptr);

  node1.RemoveChild(&node2);
  EXPECT_TRUE(node2.GetParent() == nullptr);

  EXPECT_TRUE(node1.GetFirstChild() == nullptr);
  EXPECT_TRUE(node2.GetNextSibling() == nullptr);
  EXPECT_TRUE(node2.GetPrevSibling() == nullptr);
}

TEST(CFX_XMLNodeTest, RemoveMissingChild) {
  CFX_XMLElement node1(L"node1");
  CFX_XMLElement node2(L"node2");
  CFX_XMLElement node3(L"node3");

  node1.AppendLastChild(&node2);
  EXPECT_DEATH(node1.RemoveChild(&node3), "");
}
