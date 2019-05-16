// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/tree_node.h"

#include <memory>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

namespace fxcrt {

class TestTreeNode : public TreeNode<TestTreeNode> {};

// NOTE: Successful cases are covered via RetainedTreeNode tests.
// These tests check that we trip CHECKS given bad calls.

TEST(TreeNode, SelfAppendFirstChild) {
  auto pNode = pdfium::MakeUnique<TestTreeNode>();
  EXPECT_DEATH(pNode->AppendFirstChild(pNode.get()), "");
}

TEST(TreeNode, SelfAppendLastChild) {
  auto pNode = pdfium::MakeUnique<TestTreeNode>();
  EXPECT_DEATH(pNode->AppendLastChild(pNode.get()), "");
}

TEST(TreeNode, SelfInsertBeforeOther) {
  auto pNode = pdfium::MakeUnique<TestTreeNode>();
  auto pOther = pdfium::MakeUnique<TestTreeNode>();
  pNode->AppendFirstChild(pOther.get());
  EXPECT_DEATH(pNode->InsertBefore(pNode.get(), pOther.get()), "");
}

TEST(TreeNode, InsertOtherBeforeSelf) {
  auto pNode = pdfium::MakeUnique<TestTreeNode>();
  auto pOther = pdfium::MakeUnique<TestTreeNode>();
  pNode->AppendFirstChild(pOther.get());
  EXPECT_DEATH(pNode->InsertBefore(pOther.get(), pNode.get()), "");
}

TEST(TreeNode, SelfInsertAfterOther) {
  auto pNode = pdfium::MakeUnique<TestTreeNode>();
  auto pOther = pdfium::MakeUnique<TestTreeNode>();
  pNode->AppendFirstChild(pOther.get());
  EXPECT_DEATH(pNode->InsertBefore(pNode.get(), pOther.get()), "");
}

TEST(TreeNode, InsertOtherAfterSelf) {
  auto pNode = pdfium::MakeUnique<TestTreeNode>();
  auto pOther = pdfium::MakeUnique<TestTreeNode>();
  pNode->AppendFirstChild(pOther.get());
  EXPECT_DEATH(pNode->InsertBefore(pOther.get(), pNode.get()), "");
}

TEST(TreeNode, RemoveParentless) {
  auto pNode = pdfium::MakeUnique<TestTreeNode>();
  EXPECT_DEATH(pNode->GetParent()->RemoveChild(pNode.get()), "");
}

TEST(TreeNode, RemoveFromWrongParent) {
  auto pGoodParent = pdfium::MakeUnique<TestTreeNode>();
  auto pBadParent = pdfium::MakeUnique<TestTreeNode>();
  auto pNode = pdfium::MakeUnique<TestTreeNode>();
  pGoodParent->AppendFirstChild(pNode.get());
  EXPECT_DEATH(pBadParent->RemoveChild(pNode.get()), "");
}

TEST(TreeNode, SafeRemove) {
  auto pParent = pdfium::MakeUnique<TestTreeNode>();
  auto pChild = pdfium::MakeUnique<TestTreeNode>();
  pParent->AppendFirstChild(pChild.get());
  pChild->RemoveSelfIfParented();
  EXPECT_EQ(nullptr, pParent->GetFirstChild());
  EXPECT_EQ(nullptr, pChild->GetParent());
}

TEST(TreeNode, SafeRemoveParentless) {
  auto pNode = pdfium::MakeUnique<TestTreeNode>();
  pNode->RemoveSelfIfParented();
  EXPECT_EQ(nullptr, pNode->GetParent());
}

TEST(TreeNode, RemoveAllChildren) {
  auto pParent = pdfium::MakeUnique<TestTreeNode>();
  pParent->RemoveAllChildren();
  EXPECT_EQ(nullptr, pParent->GetFirstChild());

  auto p0 = pdfium::MakeUnique<TestTreeNode>();
  auto p1 = pdfium::MakeUnique<TestTreeNode>();
  auto p2 = pdfium::MakeUnique<TestTreeNode>();
  auto p3 = pdfium::MakeUnique<TestTreeNode>();
  pParent->AppendLastChild(p0.get());
  pParent->AppendLastChild(p1.get());
  pParent->AppendLastChild(p2.get());
  pParent->AppendLastChild(p3.get());
  pParent->RemoveAllChildren();
  EXPECT_EQ(nullptr, pParent->GetFirstChild());
}

TEST(TreeNode, NthChild) {
  auto pParent = pdfium::MakeUnique<TestTreeNode>();
  EXPECT_EQ(nullptr, pParent->GetNthChild(-1));
  EXPECT_EQ(nullptr, pParent->GetNthChild(0));

  auto p0 = pdfium::MakeUnique<TestTreeNode>();
  auto p1 = pdfium::MakeUnique<TestTreeNode>();
  auto p2 = pdfium::MakeUnique<TestTreeNode>();
  auto p3 = pdfium::MakeUnique<TestTreeNode>();
  pParent->AppendLastChild(p0.get());
  pParent->AppendLastChild(p1.get());
  pParent->AppendLastChild(p2.get());
  pParent->AppendLastChild(p3.get());
  EXPECT_EQ(nullptr, pParent->GetNthChild(-1));
  EXPECT_EQ(p0.get(), pParent->GetNthChild(0));
  EXPECT_EQ(p1.get(), pParent->GetNthChild(1));
  EXPECT_EQ(p2.get(), pParent->GetNthChild(2));
  EXPECT_EQ(p3.get(), pParent->GetNthChild(3));
  EXPECT_EQ(nullptr, pParent->GetNthChild(4));
  pParent->RemoveAllChildren();
}

TEST(TreeNode, AppendFirstChild) {
  auto parent = pdfium::MakeUnique<TestTreeNode>();
  auto child0 = pdfium::MakeUnique<TestTreeNode>();
  auto child1 = pdfium::MakeUnique<TestTreeNode>();
  parent->AppendFirstChild(child0.get());
  EXPECT_EQ(child0.get(), parent->GetFirstChild());
  parent->AppendFirstChild(child1.get());
  EXPECT_EQ(child1.get(), parent->GetFirstChild());
  EXPECT_EQ(child1.get(), parent->GetNthChild(0));
  EXPECT_EQ(child0.get(), parent->GetNthChild(1));
}

TEST(TreeNode, RemoveChild) {
  auto parent = pdfium::MakeUnique<TestTreeNode>();
  auto child0 = pdfium::MakeUnique<TestTreeNode>();
  auto child1 = pdfium::MakeUnique<TestTreeNode>();

  parent->AppendFirstChild(child0.get());
  parent->AppendLastChild(child1.get());
  EXPECT_EQ(child0.get(), parent->GetFirstChild());
  EXPECT_EQ(child1.get(), parent->GetLastChild());
  parent->RemoveChild(child0.get());
  EXPECT_EQ(child1.get(), parent->GetFirstChild());
  EXPECT_EQ(child1.get(), parent->GetLastChild());
  parent->RemoveChild(child1.get());
  EXPECT_EQ(nullptr, parent->GetFirstChild());
  EXPECT_EQ(nullptr, parent->GetLastChild());

  parent->AppendFirstChild(child0.get());
  parent->AppendLastChild(child1.get());
  EXPECT_EQ(child0.get(), parent->GetFirstChild());
  EXPECT_EQ(child1.get(), parent->GetLastChild());
  parent->RemoveChild(child1.get());
  EXPECT_EQ(child0.get(), parent->GetFirstChild());
  EXPECT_EQ(child0.get(), parent->GetLastChild());
  parent->RemoveChild(child0.get());
  EXPECT_EQ(nullptr, parent->GetFirstChild());
  EXPECT_EQ(nullptr, parent->GetLastChild());
}

}  // namespace fxcrt
