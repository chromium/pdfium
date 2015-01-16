// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "../../testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFViewEmbeddertest : public EmbedderTest {
};

TEST_F(FPDFViewEmbeddertest, Document) {
  EXPECT_TRUE(OpenDocument("testing/resources/about_blank.pdf"));
  EXPECT_EQ(1, GetPageCount());
  EXPECT_EQ(0, GetFirstPageNum());

  int version;
  EXPECT_TRUE(FPDF_GetFileVersion(document(), &version));
  EXPECT_EQ(14, version);

  EXPECT_EQ(0xFFFFFFFF, FPDF_GetDocPermissions(document()));
  EXPECT_EQ(-1, FPDF_GetSecurityHandlerRevision(document()));
}

TEST_F(FPDFViewEmbeddertest, Page) {
  EXPECT_TRUE(OpenDocument("testing/resources/about_blank.pdf"));
  FPDF_FORMHANDLE form_handle = SetFormFillEnvironment();
  FPDF_PAGE page = LoadPage(0, form_handle);
  EXPECT_NE(nullptr, page);
  EXPECT_EQ(612.0, FPDF_GetPageWidth(page));
  EXPECT_EQ(792.0, FPDF_GetPageHeight(page));
  UnloadPage(page, form_handle);
  EXPECT_EQ(nullptr, LoadPage(1, form_handle));
  ClearFormFillEnvironment(form_handle);
}

TEST_F(FPDFViewEmbeddertest, ViewerRef) {
  EXPECT_TRUE(OpenDocument("testing/resources/about_blank.pdf"));
  EXPECT_TRUE(FPDF_VIEWERREF_GetPrintScaling(document()));
  EXPECT_EQ(1, FPDF_VIEWERREF_GetNumCopies(document()));
  EXPECT_EQ(DuplexUndefined, FPDF_VIEWERREF_GetDuplex(document()));
}

