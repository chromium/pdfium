// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_searchex.h"
#include "testing/embedder_test.h"

class FPDFSearchExEmbedderTest : public EmbedderTest {};

TEST_F(FPDFSearchExEmbedderTest, GetCharIndexFromTextIndex) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFTextPage textpage(FPDFText_LoadPage(page));
    ASSERT_TRUE(textpage);

    EXPECT_EQ(-2, FPDFText_GetCharIndexFromTextIndex(textpage.get(), -2));
    EXPECT_EQ(-1, FPDFText_GetCharIndexFromTextIndex(textpage.get(), -1));
    EXPECT_EQ(0, FPDFText_GetCharIndexFromTextIndex(textpage.get(), 0));
    EXPECT_EQ(1, FPDFText_GetCharIndexFromTextIndex(textpage.get(), 1));
    EXPECT_EQ(2, FPDFText_GetCharIndexFromTextIndex(textpage.get(), 2));
    EXPECT_EQ(5, FPDFText_GetCharIndexFromTextIndex(textpage.get(), 5));
    EXPECT_EQ(10, FPDFText_GetCharIndexFromTextIndex(textpage.get(), 10));
    EXPECT_EQ(29, FPDFText_GetCharIndexFromTextIndex(textpage.get(), 29));
    EXPECT_EQ(-1, FPDFText_GetCharIndexFromTextIndex(textpage.get(), 30));
  }

  UnloadPage(page);
}

TEST_F(FPDFSearchExEmbedderTest,
       GetCharIndexFromTextIndexWithNonPrintableChar) {
  ASSERT_TRUE(OpenDocument("bug_1139.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFTextPage textpage(FPDFText_LoadPage(page));
    ASSERT_TRUE(textpage);

    EXPECT_EQ(-1, FPDFText_GetCharIndexFromTextIndex(textpage.get(), -2));
    EXPECT_EQ(0, FPDFText_GetCharIndexFromTextIndex(textpage.get(), -1));
    EXPECT_EQ(1, FPDFText_GetCharIndexFromTextIndex(textpage.get(), 0));
    EXPECT_EQ(2, FPDFText_GetCharIndexFromTextIndex(textpage.get(), 1));
    EXPECT_EQ(3, FPDFText_GetCharIndexFromTextIndex(textpage.get(), 2));
    EXPECT_EQ(6, FPDFText_GetCharIndexFromTextIndex(textpage.get(), 5));
    EXPECT_EQ(11, FPDFText_GetCharIndexFromTextIndex(textpage.get(), 10));
    EXPECT_EQ(30, FPDFText_GetCharIndexFromTextIndex(textpage.get(), 29));
    EXPECT_EQ(-1, FPDFText_GetCharIndexFromTextIndex(textpage.get(), 30));
    EXPECT_EQ(-1, FPDFText_GetCharIndexFromTextIndex(textpage.get(), 31));
  }

  UnloadPage(page);
}

TEST_F(FPDFSearchExEmbedderTest, GetCharIndexFromTextIndexInvalid) {
  EXPECT_EQ(-1, FPDFText_GetCharIndexFromTextIndex(nullptr, -2));
  EXPECT_EQ(-1, FPDFText_GetCharIndexFromTextIndex(nullptr, -1));
  EXPECT_EQ(-1, FPDFText_GetCharIndexFromTextIndex(nullptr, 0));
  EXPECT_EQ(-1, FPDFText_GetCharIndexFromTextIndex(nullptr, 1));
  EXPECT_EQ(-1, FPDFText_GetCharIndexFromTextIndex(nullptr, 2));
}

TEST_F(FPDFSearchExEmbedderTest, GetTextIndexFromCharIndex) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFTextPage textpage(FPDFText_LoadPage(page));
    ASSERT_TRUE(textpage);

    EXPECT_EQ(-1, FPDFText_GetTextIndexFromCharIndex(textpage.get(), -2));
    EXPECT_EQ(-1, FPDFText_GetTextIndexFromCharIndex(textpage.get(), -1));
    EXPECT_EQ(0, FPDFText_GetTextIndexFromCharIndex(textpage.get(), 0));
    EXPECT_EQ(1, FPDFText_GetTextIndexFromCharIndex(textpage.get(), 1));
    EXPECT_EQ(2, FPDFText_GetTextIndexFromCharIndex(textpage.get(), 2));
    EXPECT_EQ(5, FPDFText_GetTextIndexFromCharIndex(textpage.get(), 5));
    EXPECT_EQ(10, FPDFText_GetTextIndexFromCharIndex(textpage.get(), 10));
    EXPECT_EQ(29, FPDFText_GetTextIndexFromCharIndex(textpage.get(), 29));
    EXPECT_EQ(-1, FPDFText_GetTextIndexFromCharIndex(textpage.get(), 30));
  }

  UnloadPage(page);
}

TEST_F(FPDFSearchExEmbedderTest,
       GetTextIndexFromCharIndexWithNonPrintableChar) {
  ASSERT_TRUE(OpenDocument("bug_1139.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFTextPage textpage(FPDFText_LoadPage(page));
    ASSERT_TRUE(textpage);

    EXPECT_EQ(-1, FPDFText_GetTextIndexFromCharIndex(textpage.get(), -2));
    EXPECT_EQ(-1, FPDFText_GetTextIndexFromCharIndex(textpage.get(), -1));
    EXPECT_EQ(-1, FPDFText_GetTextIndexFromCharIndex(textpage.get(), 0));
    EXPECT_EQ(0, FPDFText_GetTextIndexFromCharIndex(textpage.get(), 1));
    EXPECT_EQ(1, FPDFText_GetTextIndexFromCharIndex(textpage.get(), 2));
    EXPECT_EQ(4, FPDFText_GetTextIndexFromCharIndex(textpage.get(), 5));
    EXPECT_EQ(9, FPDFText_GetTextIndexFromCharIndex(textpage.get(), 10));
    EXPECT_EQ(28, FPDFText_GetTextIndexFromCharIndex(textpage.get(), 29));
    EXPECT_EQ(29, FPDFText_GetTextIndexFromCharIndex(textpage.get(), 30));
    EXPECT_EQ(-1, FPDFText_GetTextIndexFromCharIndex(textpage.get(), 31));
  }

  UnloadPage(page);
}

TEST_F(FPDFSearchExEmbedderTest, GetTextIndexFromCharIndexInvalid) {
  EXPECT_EQ(-1, FPDFText_GetTextIndexFromCharIndex(nullptr, -2));
  EXPECT_EQ(-1, FPDFText_GetTextIndexFromCharIndex(nullptr, -1));
  EXPECT_EQ(-1, FPDFText_GetTextIndexFromCharIndex(nullptr, 0));
  EXPECT_EQ(-1, FPDFText_GetTextIndexFromCharIndex(nullptr, 1));
  EXPECT_EQ(-1, FPDFText_GetTextIndexFromCharIndex(nullptr, 2));
}
