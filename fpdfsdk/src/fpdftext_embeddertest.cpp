// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "../../testing/embedder_test.h"
#include "../../fpdfsdk/include/fpdfview.h"
#include "../../fpdfsdk/include/fpdftext.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFTextEmbeddertest : public EmbedderTest {
};

// Test that the page has characters despite a bad stream length.
TEST_F(FPDFTextEmbeddertest, StreamLengthPastEndOfFile) {
  EXPECT_TRUE(OpenDocument("testing/resources/bug_57.pdf"));
  FPDF_FORMHANDLE form_handle = SetFormFillEnvironment();
  FPDF_PAGE page = LoadPage(0, form_handle);
  EXPECT_NE(nullptr, page);
  FPDF_TEXTPAGE textpage = FPDFText_LoadPage(page);
  EXPECT_NE(nullptr, textpage);
  EXPECT_EQ(13, FPDFText_CountChars(textpage));
  ClearFormFillEnvironment(form_handle);
}
