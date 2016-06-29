// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFRenderPatternEmbeddertest : public EmbedderTest {};

#if defined(_SKIA_SUPPORT_)
#define MAYBE_LoadError_547706 DISABLED_LoadError_547706
#else
#define MAYBE_LoadError_547706 LoadError_547706
#endif
TEST_F(FPDFRenderPatternEmbeddertest, MAYBE_LoadError_547706) {
  // Test shading where object is a dictionary instead of a stream.
  EXPECT_TRUE(OpenDocument("bug_547706.pdf"));
  FPDF_PAGE page = LoadPage(0);
  FPDF_BITMAP bitmap = RenderPage(page);
  FPDFBitmap_Destroy(bitmap);
  UnloadPage(page);
}
