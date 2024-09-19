// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/cpp/fpdf_scopers.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class JBig2EmbedderTest : public EmbedderTest {};

#if defined(PDF_USE_SKIA)
// TODO(crbug.com/pdfium/11): Fix this test and enable.
#define MAYBE_Bug631912 DISABLED_Bug631912
#else
#define MAYBE_Bug631912 Bug631912
#endif
TEST_F(JBig2EmbedderTest, MAYBE_Bug631912) {
  // Test jbig2 image in PDF file can be loaded successfully.
  // Should not crash.
  ASSERT_TRUE(OpenDocument("bug_631912.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
  CompareBitmap(bitmap.get(), 691, 432, "726c2b8c89df0ab40627322d1dddd521");
}
