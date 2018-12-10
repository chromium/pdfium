// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/parser/xfa_basic_data.h"

#include "testing/gtest/include/gtest/gtest.h"

TEST(XFABasicDataTest, GetAttributeValueByName) {
  EXPECT_FALSE(XFA_GetAttributeValueByName(L"").has_value());
  EXPECT_FALSE(XFA_GetAttributeValueByName(L"nonesuch").has_value());

  Optional<XFA_AttributeValue> result = XFA_GetAttributeValueByName(L"*");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(XFA_AttributeValue::Asterisk, result.value());

  result = XFA_GetAttributeValueByName(L"visible");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(XFA_AttributeValue::Visible, result.value());

  result = XFA_GetAttributeValueByName(L"lowered");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(XFA_AttributeValue::Lowered, result.value());
}
