// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/cpp/fpdf_scopers.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_environment.h"
#include "testing/gtest/include/gtest/gtest.h"

class FXGETextEmbedderTest : public EmbedderTest {
 public:
  void TearDown() override {
    EmbedderTest::TearDown();

    // TODO(tsepez): determine how this is changing the environment,
    // such that FPDFAnnotEmbedderTest.BUG_1206 will diff if run
    // after this.
    EmbedderTestEnvironment::GetInstance()->TearDown();
    EmbedderTestEnvironment::GetInstance()->SetUp();
  }
};

TEST_F(FXGETextEmbedderTest, BadItalic) {
  // Shouldn't crash.
  ASSERT_TRUE(OpenDocument("bug_601362.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ScopedFPDFBitmap bitmap = RenderLoadedPage(page);
  EXPECT_EQ(612, FPDFBitmap_GetWidth(bitmap.get()));
  EXPECT_EQ(792, FPDFBitmap_GetHeight(bitmap.get()));
  UnloadPage(page);
}
