// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/tree_node.h"

#include <memory>

#include "testing/gtest/include/gtest/gtest.h"

namespace fxcrt {

class TestTreeNode : public TreeNode<TestTreeNode> {};

// NOTE: Successful cases are covered via RetainedTreeNode tests.
// These tests check that we trip CHECKS given bad calls.

TEST(TreeNode, SelfAppendFirstChild) {
  auto pNode = std::make_unique<TestTreeNode>();
  EXPECT_DEATH(pNode->AppendFirstChild(pNode.get()), "");
}

TEST(TreeNode, SelfAppendLastChild) {
  auto pNode = std::make_unique<TestTreeNode>();
  EXPECT_DEATH(pNode->AppendLastChild(pNode.get()), "");
}

TEST(TreeNode, SelfInsertBeforeOther) {
  auto pNode = std::make_unique<TestTreeNode>();
  auto pOther = std::make_unique<TestTreeNode>();
  pNode->AppendFirstChild(pOther.get());
  EXPECT_DEATH(pNode->InsertBefore(pNode.get(), pOther.get()), "");
}

TEST(TreeNode, InsertOtherBeforeSelf) {
  auto pNode = std::make_unique<TestTreeNode>();
  auto pOther = std::make_unique<TestTreeNode>();
  pNode->AppendFirstChild(pOther.get());
  EXPECT_DEATH(pNode->InsertBefore(pOther.get(), pNode.get()), "");
}

TEST(TreeNode, SelfInsertAfterOther) {
  auto pNode = std::make_unique<TestTreeNode>();
  auto pOther = std::make_unique<TestTreeNode>();
  pNode->AppendFirstChild(pOther.get());
  EXPECT_DEATH(pNode->InsertBefore(pNode.get(), pOther.get()), "");
}

TEST(TreeNode, InsertOtherAfterSelf) {
  auto pNode = std::make_unique<TestTreeNode>();
  auto pOther = std::make_unique<TestTreeNode>();
  pNode->AppendFirstChild(pOther.get());
  EXPECT_DEATH(pNode->InsertBefore(pOther.get(), pNode.get()), "");
}

TEST(TreeNode, RemoveParentless) {
  auto pNode = std::make_unique<TestTreeNode>();
  EXPECT_DEATH(pNode->GetParent()->RemoveChild(pNode.get()), "");
}

TEST(TreeNode, RemoveFromWrongParent) {
  auto pGoodParent = std::make_unique<TestTreeNode>();
  auto pBadParent = std::make_unique<TestTreeNode>();
  auto pNode = std::make_unique<TestTreeNode>();
  pGoodParent->AppendFirstChild(pNode.get());
  EXPECT_DEATH(pBadParent->RemoveChild(pNode.get()), "");
}

TEST(TreeNode, SafeRemove) {
  auto pParent = std::make_unique<TestTreeNode>();
  auto pChild = std::make_unique<TestTreeNode>();
  pParent->AppendFirstChild(pChild.get());
  pChild->RemoveSelfIfParented();
  EXPECT_FALSE(pParent->GetFirstChild());
  EXPECT_FALSE(pChild->GetParent());
}

TEST(TreeNode, SafeRemoveParentless) {
  auto pNode = std::make_unique<TestTreeNode>();
  pNode->RemoveSelfIfParented();
  EXPECT_FALSE(pNode->GetParent());
}

TEST(TreeNode, RemoveAllChildren) {
  auto pParent = std::make_unique<TestTreeNode>();
  pParent->RemoveAllChildren();
  EXPECT_FALSE(pParent->GetFirstChild());

  auto p0 = std::make_unique<TestTreeNode>();
  auto p1 = std::make_unique<TestTreeNode>();
  auto p2 = std::make_unique<TestTreeNode>();
  auto p3 = std::make_unique<TestTreeNode>();
  pParent->AppendLastChild(p0.get());
  pParent->AppendLastChild(p1.get());
  pParent->AppendLastChild(p2.get());
  pParent->AppendLastChild(p3.get());
  pParent->RemoveAllChildren();
  EXPECT_FALSE(pParent->GetFirstChild());
}

TEST(TreeNode, NthChild) {
  auto pParent = std::make_unique<TestTreeNode>();
  EXPECT_FALSE(pParent->GetNthChild(-1));
  EXPECT_FALSE(pParent->GetNthChild(0));

  auto p0 = std::make_unique<TestTreeNode>();
  auto p1 = std::make_unique<TestTreeNode>();
  auto p2 = std::make_unique<TestTreeNode>();
  auto p3 = std::make_unique<TestTreeNode>();
  pParent->AppendLastChild(p0.get());
  pParent->AppendLastChild(p1.get());
  pParent->AppendLastChild(p2.get());
  pParent->AppendLastChild(p3.get());
  EXPECT_FALSE(pParent->GetNthChild(-1));
  EXPECT_EQ(p0.get(), pParent->GetNthChild(0));
  EXPECT_EQ(p1.get(), pParent->GetNthChild(1));
  EXPECT_EQ(p2.get(), pParent->GetNthChild(2));
  EXPECT_EQ(p3.get(), pParent->GetNthChild(3));
  EXPECT_FALSE(pParent->GetNthChild(4));
  pParent->RemoveAllChildren();
}

TEST(TreeNode, AppendFirstChild) {
  auto parent = std::make_unique<TestTreeNode>();
  auto child0 = std::make_unique<TestTreeNode>();
  auto child1 = std::make_unique<TestTreeNode>();
  parent->AppendFirstChild(child0.get());
  EXPECT_EQ(child0.get(), parent->GetFirstChild());
  parent->AppendFirstChild(child1.get());
  EXPECT_EQ(child1.get(), parent->GetFirstChild());
  EXPECT_EQ(child1.get(), parent->GetNthChild(0));
  EXPECT_EQ(child0.get(), parent->GetNthChild(1));
}

TEST(TreeNode, RemoveChild) {
  auto parent = std::make_unique<TestTreeNode>();
  auto child0 = std::make_unique<TestTreeNode>();
  auto child1 = std::make_unique<TestTreeNode>();

  parent->AppendFirstChild(child0.get());
  parent->AppendLastChild(child1.get());
  EXPECT_EQ(child0.get(), parent->GetFirstChild());
  EXPECT_EQ(child1.get(), parent->GetLastChild());
  parent->RemoveChild(child0.get());
  EXPECT_EQ(child1.get(), parent->GetFirstChild());
  EXPECT_EQ(child1.get(), parent->GetLastChild());
  parent->RemoveChild(child1.get());
  EXPECT_FALSE(parent->GetFirstChild());
  EXPECT_FALSE(parent->GetLastChild());

  parent->AppendFirstChild(child0.get());
  parent->AppendLastChild(child1.get());
  EXPECT_EQ(child0.get(), parent->GetFirstChild());
  EXPECT_EQ(child1.get(), parent->GetLastChild());
  parent->RemoveChild(child1.get());
  EXPECT_EQ(child0.get(), parent->GetFirstChild());
  EXPECT_EQ(child0.get(), parent->GetLastChild());
  parent->RemoveChild(child0.get());
  EXPECT_FALSE(parent->GetFirstChild());
  EXPECT_FALSE(parent->GetLastChild());
}

}  // namespace fxcrt
