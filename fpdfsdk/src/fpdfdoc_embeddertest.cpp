// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "../../testing/embedder_test.h"
#include "../../fpdfsdk/include/fpdfview.h"
#include "../../fpdfsdk/include/fpdfdoc.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFDocEmbeddertest : public EmbedderTest {
};

TEST_F(FPDFDocEmbeddertest, DestGetPageIndex) {
  EXPECT_TRUE(OpenDocument("testing/resources/named_dests.pdf"));

  // NULL FPDF_DEST case.
  EXPECT_EQ(0, FPDFDest_GetPageIndex(document(), nullptr));

  // Page number directly in item from Dests NameTree.
  FPDF_DEST dest = FPDF_GetNamedDestByName(document(), "First");
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(1, FPDFDest_GetPageIndex(document(), dest));

  // Page number via object reference in item from Dests NameTree.
  dest = FPDF_GetNamedDestByName(document(), "Next");
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(1, FPDFDest_GetPageIndex(document(), dest));

  // Page number directly in item from Dests dictionary.
  dest = FPDF_GetNamedDestByName(document(), "FirstAlternate");
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(11, FPDFDest_GetPageIndex(document(), dest));

  // Invalid object reference in item from Dests NameTree.
  dest = FPDF_GetNamedDestByName(document(), "LastAlternate");
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(0, FPDFDest_GetPageIndex(document(), dest));
}
