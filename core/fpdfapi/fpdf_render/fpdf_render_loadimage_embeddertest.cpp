// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFRenderLoadImageEmbeddertest : public EmbedderTest {};

#if defined(_SKIA_SUPPORT_)
#define MAYBE_Bug_554151 DISABLED_Bug_554151
#else
#define MAYBE_Bug_554151 Bug_554151
#endif
TEST_F(FPDFRenderLoadImageEmbeddertest, MAYBE_Bug_554151) {
  // Test scanline downsampling with a BitsPerComponent of 4.
  // Should not crash.
  EXPECT_TRUE(OpenDocument("bug_554151.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
  FPDF_BITMAP bitmap = RenderPage(page);
  FPDFBitmap_Destroy(bitmap);
  UnloadPage(page);
}

TEST_F(FPDFRenderLoadImageEmbeddertest, Bug_557223) {
  // Should not crash
  EXPECT_TRUE(OpenDocument("bug_557223.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
  FPDF_BITMAP bitmap = RenderPage(page);
  FPDFBitmap_Destroy(bitmap);
  UnloadPage(page);
}

TEST_F(FPDFRenderLoadImageEmbeddertest, Bug_603518) {
  // Should not crash
  EXPECT_TRUE(OpenDocument("bug_603518.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
  FPDF_BITMAP bitmap = RenderPage(page);
  FPDFBitmap_Destroy(bitmap);
  UnloadPage(page);
}
