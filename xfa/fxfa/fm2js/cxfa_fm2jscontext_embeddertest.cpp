// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/xfa_js_embedder_test.h"

class FM2JSContextEmbedderTest : public XFAJSEmbedderTest {};

TEST_F(FM2JSContextEmbedderTest, TranslateEmpty) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  const char input[] = "";
  EXPECT_TRUE(Execute(input));
  // TODO(dsinclair): This should probably throw as a blank formcalc script
  // is invalid.
}

TEST_F(FM2JSContextEmbedderTest, TranslateNumber) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  const char input[] = "123";
  EXPECT_TRUE(Execute(input));

  CFXJSE_Value* value = GetValue();
  ASSERT_TRUE(value->IsNumber());
  EXPECT_EQ(123, value->ToInteger());
}

TEST_F(FM2JSContextEmbedderTest, Add) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  const char input[] = "123 + 456";
  EXPECT_TRUE(Execute(input));

  CFXJSE_Value* value = GetValue();
  ASSERT_TRUE(value->IsNumber());
  EXPECT_EQ(579, value->ToInteger());
}
