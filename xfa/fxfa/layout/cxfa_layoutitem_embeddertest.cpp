// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/xfa_js_embedder_test.h"

class CXFALayoutItemEmbedderTest : public XFAJSEmbedderTest {};

TEST_F(CXFALayoutItemEmbedderTest, Bug1265) {
  ASSERT_TRUE(OpenDocument("bug_1265.pdf"));
  ScopedEmbedderTestPage page0 = LoadScopedPage(0);
  ScopedEmbedderTestPage page1 = LoadScopedPage(1);
  EXPECT_TRUE(page0.get());
  EXPECT_FALSE(page1.get());
}

TEST_F(CXFALayoutItemEmbedderTest, Bug1301) {
  ASSERT_TRUE(OpenDocument("bug_1301.pdf"));
  ScopedEmbedderTestPage page0 = LoadScopedPage(0);
  ScopedEmbedderTestPage page1 = LoadScopedPage(1);
  ScopedEmbedderTestPage page2 = LoadScopedPage(2);
  EXPECT_TRUE(page0.get());
  EXPECT_TRUE(page1.get());
  EXPECT_FALSE(page2.get());
}

TEST_F(CXFALayoutItemEmbedderTest, Bug306123) {
  ASSERT_TRUE(OpenDocument("bug_306123.pdf"));
  ScopedEmbedderTestPage page0 = LoadScopedPage(0);
  ScopedEmbedderTestPage page1 = LoadScopedPage(1);
  ScopedEmbedderTestPage page2 = LoadScopedPage(2);
  EXPECT_TRUE(page0.get());
  EXPECT_TRUE(page1.get());
  EXPECT_FALSE(page2.get());
}

TEST_F(CXFALayoutItemEmbedderTest, BreakBeforeAfter) {
  static constexpr int kExpectedPageCount = 10;
  ASSERT_TRUE(OpenDocument("xfa/xfa_break_before_after.pdf"));
  for (int i = 0; i < kExpectedPageCount; ++i) {
    ScopedEmbedderTestPage page = LoadScopedPage(i);
    EXPECT_TRUE(page);
  }
}
