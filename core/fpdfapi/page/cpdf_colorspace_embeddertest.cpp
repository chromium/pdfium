// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class CPDF_ColorspaceEmbedderTest : public EmbedderTest {};

// Test passes if it does not crash.
TEST_F(CPDF_ColorspaceEmbedderTest, Bug_981288) {
  ASSERT_TRUE(OpenDocument("bug_981288.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  UnloadPage(page);
}
