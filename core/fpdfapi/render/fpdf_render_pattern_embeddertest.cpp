// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/cpp/fpdf_scopers.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_constants.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFRenderPatternEmbedderTest : public EmbedderTest {};

TEST_F(FPDFRenderPatternEmbedderTest, LoadError_547706) {
  // Test shading where object is a dictionary instead of a stream.
  ASSERT_TRUE(OpenDocument("bug_547706.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ScopedFPDFBitmap bitmap = RenderLoadedPage(page);
  CompareBitmap(bitmap.get(), 612, 792, pdfium::kBlankPage612By792Checksum);
  UnloadPage(page);
}
