// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/parser/xfa_basic_data.h"

#include <utility>

#include "testing/gtest/include/gtest/gtest.h"

namespace {

void DoElementAttrTestCase(XFA_Element elem,
                           XFA_Attribute attr,
                           std::pair<XFA_Element, XFA_Attribute>* so_far) {
  auto curr = std::make_pair(elem, attr);
  EXPECT_LT(*so_far, curr) << " for " << static_cast<int>(elem) << ", "
                           << static_cast<int>(attr);
  *so_far = curr;
}

}  // namespace

TEST(XFABasicDataTest, GetAttributeByName) {
  Optional<XFA_ATTRIBUTEINFO> result = XFA_GetAttributeByName(L"");
  EXPECT_FALSE(result.has_value());

  result = XFA_GetAttributeByName(L"nonesuch");
  EXPECT_FALSE(result.has_value());

  result = XFA_GetAttributeByName(L"h");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(XFA_Attribute::H, result.value().attribute);

  result = XFA_GetAttributeByName(L"short");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(XFA_Attribute::Short, result.value().attribute);

  result = XFA_GetAttributeByName(L"decipherOnly");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(XFA_Attribute::DecipherOnly, result.value().attribute);
}

TEST(XFABasicDataTest, GetAttributeValueByName) {
  Optional<XFA_AttributeValue> result = XFA_GetAttributeValueByName(L"");
  EXPECT_FALSE(result.has_value());

  result = XFA_GetAttributeValueByName(L"nonesuch");
  EXPECT_FALSE(result.has_value());

  result = XFA_GetAttributeValueByName(L"*");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(XFA_AttributeValue::Asterisk, result.value());

  result = XFA_GetAttributeValueByName(L"visible");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(XFA_AttributeValue::Visible, result.value());

  result = XFA_GetAttributeValueByName(L"lowered");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(XFA_AttributeValue::Lowered, result.value());
}

TEST(XFABasicDataText, ElementAttributeOrder) {
  std::pair<XFA_Element, XFA_Attribute> so_far = {};
#undef ELEM_ATTR____
#define ELEM_ATTR____(a, b, c) \
  DoElementAttrTestCase(XFA_Element::a, XFA_Attribute::b, &so_far);
#include "xfa/fxfa/parser/element_attributes.inc"
#undef ELEM_ATTR____
}
