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

}  // namespace fxcrt
