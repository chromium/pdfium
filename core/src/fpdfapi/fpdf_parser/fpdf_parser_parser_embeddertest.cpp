// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFParserEmbeddertest : public EmbedderTest {};

TEST_F(FPDFParserEmbeddertest, LoadError_454695) {
  // Test a dictionary with hex string instead of correct content.
  // Verify that the defective pdf shouldn't be opened correctly.
  EXPECT_FALSE(OpenDocument("bug_454695.pdf"));
}

TEST_F(FPDFParserEmbeddertest, Bug_481363) {
  // Test colorspace object with malformed dictionary.
  EXPECT_TRUE(OpenDocument("bug_481363.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
  UnloadPage(page);
}

TEST_F(FPDFParserEmbeddertest, Bug_544880) {
  // Test self referencing /Pages object.
  EXPECT_TRUE(OpenDocument("bug_544880.pdf"));
  // Shouldn't crash. We don't check the return value here because we get the
  // the count from the "/Count 1" in the testcase (at the time of writing)
  // rather than the actual count (0).
  (void)GetPageCount();
}

TEST_F(FPDFParserEmbeddertest, Feature_Linearized_Loading) {
  EXPECT_TRUE(OpenDocument("feature_linearized_loading.pdf", true));
}

TEST_F(FPDFParserEmbeddertest, Bug_325) {
  EXPECT_FALSE(OpenDocument("bug_325_a.pdf"));
  EXPECT_FALSE(OpenDocument("bug_325_b.pdf"));
}
