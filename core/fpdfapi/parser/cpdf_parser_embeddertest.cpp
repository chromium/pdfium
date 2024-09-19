// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_text.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class CPDFParserEmbedderTest : public EmbedderTest {};

TEST_F(CPDFParserEmbedderTest, LoadErrorBug454695) {
  // Test a dictionary with hex string instead of correct content.
  // Verify that the defective pdf shouldn't be opened correctly.
  EXPECT_FALSE(OpenDocument("bug_454695.pdf"));
}

TEST_F(CPDFParserEmbedderTest, Bug481363) {
  // Test colorspace object with malformed dictionary.
  ASSERT_TRUE(OpenDocument("bug_481363.pdf"));
  EXPECT_TRUE(LoadScopedPage(0));
}

TEST_F(CPDFParserEmbedderTest, Bug544880) {
  // Test self referencing /Pages object.
  ASSERT_TRUE(OpenDocument("bug_544880.pdf"));
  // Shouldn't crash. We don't check the return value here because we get the
  // the count from the "/Count 1" in the testcase (at the time of writing)
  // rather than the actual count (0).
  (void)GetPageCount();
}

TEST_F(CPDFParserEmbedderTest, Bug325a) {
  EXPECT_FALSE(OpenDocument("bug_325_a.pdf"));
}

TEST_F(CPDFParserEmbedderTest, Bug325b) {
  EXPECT_FALSE(OpenDocument("bug_325_b.pdf"));
}

TEST_F(CPDFParserEmbedderTest, Bug602650) {
  // Test the case that cross reference entries, which are well formed,
  // but do not match with the objects.
  ASSERT_TRUE(OpenDocument("bug_602650.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  EXPECT_TRUE(page);
  FPDF_TEXTPAGE text_page = FPDFText_LoadPage(page.get());
  EXPECT_TRUE(text_page);
  // The page should not be blank.
  EXPECT_LT(0, FPDFText_CountChars(text_page));

  FPDFText_ClosePage(text_page);
}

TEST_F(CPDFParserEmbedderTest, Bug757705) {
  ASSERT_TRUE(OpenDocument("bug_757705.pdf"));
}

TEST_F(CPDFParserEmbedderTest, LoadMainCrossRefTable) {
  ASSERT_TRUE(OpenDocumentLinearized("feature_linearized_loading.pdf"));
  // To check that main cross ref table is loaded correctly,will be enough to
  // check that the second page was correctly loaded. Because it is contains
  // crossrefs for second page.
  EXPECT_EQ(2, GetPageCount());
  ScopedEmbedderTestPage page = LoadScopedPage(1);
  EXPECT_TRUE(page);
  FPDF_TEXTPAGE text_page = FPDFText_LoadPage(page.get());
  EXPECT_TRUE(text_page);
  // The page should not be blank.
  EXPECT_LT(0, FPDFText_CountChars(text_page));
  FPDFText_ClosePage(text_page);
}

TEST_F(CPDFParserEmbedderTest, Bug828049) {
  ASSERT_TRUE(OpenDocument("bug_828049.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  EXPECT_TRUE(page);
}

// crbug.com/1191313
TEST_F(CPDFParserEmbedderTest, InvalidDictionaryKeys) {
  ASSERT_TRUE(OpenDocument("bad_dict_keys.pdf"));
}
