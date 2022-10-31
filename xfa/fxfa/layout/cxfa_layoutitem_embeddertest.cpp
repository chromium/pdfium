// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/xfa_js_embedder_test.h"

class CXFALayoutItemEmbedderTest : public XFAJSEmbedderTest {};

TEST_F(CXFALayoutItemEmbedderTest, Bug_1265) {
  ASSERT_TRUE(OpenDocument("bug_1265.pdf"));
  FPDF_PAGE page0 = LoadPage(0);
  FPDF_PAGE page1 = LoadPage(1);
  EXPECT_TRUE(page0);
  EXPECT_FALSE(page1);
  UnloadPage(page0);
}

TEST_F(CXFALayoutItemEmbedderTest, Bug_1301) {
  ASSERT_TRUE(OpenDocument("bug_1301.pdf"));
  FPDF_PAGE page0 = LoadPage(0);
  FPDF_PAGE page1 = LoadPage(1);
  FPDF_PAGE page2 = LoadPage(2);
  EXPECT_TRUE(page0);
  EXPECT_TRUE(page1);
  EXPECT_FALSE(page2);
  UnloadPage(page0);
  UnloadPage(page1);
}

TEST_F(CXFALayoutItemEmbedderTest, Bug_306123) {
  ASSERT_TRUE(OpenDocument("bug_306123.pdf"));
  FPDF_PAGE page0 = LoadPage(0);
  FPDF_PAGE page1 = LoadPage(1);
  FPDF_PAGE page2 = LoadPage(2);
  EXPECT_TRUE(page0);
  EXPECT_TRUE(page1);
  EXPECT_FALSE(page2);
  UnloadPage(page0);
  UnloadPage(page1);
}

TEST_F(CXFALayoutItemEmbedderTest, BreakBeforeAfter) {
  static constexpr int kExpectedPageCount = 10;
  ASSERT_TRUE(OpenDocument("xfa/xfa_break_before_after.pdf"));
  for (int i = 0; i < kExpectedPageCount; ++i) {
    FPDF_PAGE page = LoadPage(i);
    EXPECT_TRUE(page);
    UnloadPage(page);
  }
}
