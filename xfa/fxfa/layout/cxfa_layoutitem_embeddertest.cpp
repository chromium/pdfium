// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class CXFALayoutItemEmbedderTest : public EmbedderTest {};

#if defined(LEAK_SANITIZER)

// Leaks. See https://crbug.com/pdfium/1301
#define MAYBE_Bug_1301 DISABLED_Bug_1301

#else
#define MAYBE_Bug_1301 Bug_1301
#endif

TEST_F(CXFALayoutItemEmbedderTest, Bug_1265) {
  EXPECT_TRUE(OpenDocument("bug_1265.pdf"));
  FPDF_PAGE page0 = LoadPage(0);
  FPDF_PAGE page1 = LoadPage(1);
  EXPECT_NE(nullptr, page0);
  EXPECT_EQ(nullptr, page1);
  UnloadPage(page0);
}

TEST_F(CXFALayoutItemEmbedderTest, MAYBE_Bug_1301) {
  EXPECT_TRUE(OpenDocument("bug_1301.pdf"));
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
  EXPECT_TRUE(OpenDocument("bug_306123.pdf"));
  FPDF_PAGE page0 = LoadPage(0);
  FPDF_PAGE page1 = LoadPage(1);
  FPDF_PAGE page2 = LoadPage(2);
  EXPECT_NE(nullptr, page0);
  EXPECT_NE(nullptr, page1);
  EXPECT_EQ(nullptr, page2);
  UnloadPage(page0);
  UnloadPage(page1);
}
