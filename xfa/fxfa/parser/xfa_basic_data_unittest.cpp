// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/parser/xfa_basic_data.h"

#include "testing/gtest/include/gtest/gtest.h"

TEST(XFABasicDataTest, GetAttributeValueByName) {
  EXPECT_FALSE(!!XFA_GetAttributeValueByName(L""));
  EXPECT_FALSE(!!XFA_GetAttributeValueByName(L"nonesuch"));
  EXPECT_EQ(XFA_AttributeValue::Asterisk, *XFA_GetAttributeValueByName(L"*"));
  EXPECT_EQ(XFA_AttributeValue::Visible,
            *XFA_GetAttributeValueByName(L"visible"));
  EXPECT_EQ(XFA_AttributeValue::Lowered,
            *XFA_GetAttributeValueByName(L"lowered"));
}
