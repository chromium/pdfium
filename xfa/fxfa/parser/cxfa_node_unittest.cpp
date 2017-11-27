// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/parser/cxfa_node.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

TEST(CXFA_NodeTest, NameToAttribute) {
  EXPECT_EQ(XFA_Attribute::Unknown, CXFA_Node::NameToAttribute(L""));
  EXPECT_EQ(XFA_Attribute::Unknown, CXFA_Node::NameToAttribute(L"nonesuch"));
  EXPECT_EQ(XFA_Attribute::H, CXFA_Node::NameToAttribute(L"h"));
  EXPECT_EQ(XFA_Attribute::Short, CXFA_Node::NameToAttribute(L"short"));
  EXPECT_EQ(XFA_Attribute::DecipherOnly,
            CXFA_Node::NameToAttribute(L"decipherOnly"));
}
