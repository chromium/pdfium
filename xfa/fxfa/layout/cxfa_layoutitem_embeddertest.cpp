// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class CXFALayoutItemEmbedderTest : public EmbedderTest {};

TEST_F(CXFALayoutItemEmbedderTest, Bug_1265) {
  ASSERT_TRUE(OpenDocument("bug_1265.pdf"));
  FPDF_PAGE page0 = LoadPage(0);
  FPDF_PAGE page1 = LoadPage(1);
  EXPECT_NE(nullptr, page0);
  EXPECT_EQ(nullptr, page1);
  UnloadPage(page0);
}

TEST_F(CXFALayoutItemEmbedderTest, Bug_1301) {
  ASSERT_TRUE(OpenDocument("bug_1301.pdf"));
  FPDF_PAGE page0 = LoadPage(0);
  FPDF_PAGE page1 = LoadPage(1);
  FPDF_PAGE page2 = LoadPage(2);
  EXPECT_NE(nullptr, page0);
  EXPECT_NE(nullptr, page1);
  EXPECT_EQ(nullptr, page2);
  UnloadPage(page0);
  UnloadPage(page1);
}

TEST_F(CXFALayoutItemEmbedderTest, Bug_306123) {
  ASSERT_TRUE(OpenDocument("bug_306123.pdf"));
  FPDF_PAGE page0 = LoadPage(0);
  FPDF_PAGE page1 = LoadPage(1);
  FPDF_PAGE page2 = LoadPage(2);
  EXPECT_NE(nullptr, page0);
  EXPECT_NE(nullptr, page1);
  EXPECT_EQ(nullptr, page2);
  UnloadPage(page0);
  UnloadPage(page1);
}
