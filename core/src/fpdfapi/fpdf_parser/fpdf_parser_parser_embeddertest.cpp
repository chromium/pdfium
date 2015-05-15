// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "../../../testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFParserEmbeddertest : public EmbedderTest {
};

TEST_F(FPDFParserEmbeddertest, LoadError_454695) {
  // Test trailer dictionary with $$ze instead of Size.
  EXPECT_TRUE(OpenDocument("testing/resources/bug_454695.pdf"));
}

TEST_F(FPDFParserEmbeddertest, Bug_481363) {
  // Test colorspace object with malformed dictionary.
  EXPECT_TRUE(OpenDocument("testing/resources/bug_481363.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
  UnloadPage(page);
}
