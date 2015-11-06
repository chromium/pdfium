// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "../../testing/embedder_test.h"
#include "../../testing/fx_string_testhelpers.h"
#include "core/include/fxcrt/fx_string.h"
#include "public/fpdf_doc.h"
#include "public/fpdfview.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFDocEmbeddertest : public EmbedderTest {};

TEST_F(FPDFDocEmbeddertest, DestGetPageIndex) {
  EXPECT_TRUE(OpenDocument("testing/resources/named_dests.pdf"));

  // NULL FPDF_DEST case.
  EXPECT_EQ(0U, FPDFDest_GetPageIndex(document(), nullptr));

  // Page number directly in item from Dests NameTree.
  FPDF_DEST dest = FPDF_GetNamedDestByName(document(), "First");
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(1U, FPDFDest_GetPageIndex(document(), dest));

  // Page number via object reference in item from Dests NameTree.
  dest = FPDF_GetNamedDestByName(document(), "Next");
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(1U, FPDFDest_GetPageIndex(document(), dest));

  // Page number directly in item from Dests dictionary.
  dest = FPDF_GetNamedDestByName(document(), "FirstAlternate");
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(11U, FPDFDest_GetPageIndex(document(), dest));

  // Invalid object reference in item from Dests NameTree.
  dest = FPDF_GetNamedDestByName(document(), "LastAlternate");
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(0U, FPDFDest_GetPageIndex(document(), dest));
}

TEST_F(FPDFDocEmbeddertest, ActionGetFilePath) {
  EXPECT_TRUE(OpenDocument("testing/resources/launch_action.pdf"));

  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);

  // The target action is nearly the size of the whole page.
  FPDF_LINK link = FPDFLink_GetLinkAtPoint(page, 100, 100);
  ASSERT_TRUE(link);

  FPDF_ACTION action = FPDFLink_GetAction(link);
  ASSERT_TRUE(action);

  const char kExpectedResult[] = "test.pdf";
  const unsigned long kExpectedLength = sizeof(kExpectedResult);
  unsigned long bufsize = FPDFAction_GetFilePath(action, nullptr, 0);
  ASSERT_EQ(kExpectedLength, bufsize);

  char buf[kExpectedLength];
  EXPECT_EQ(bufsize, FPDFAction_GetFilePath(action, buf, bufsize));
  EXPECT_EQ(std::string(kExpectedResult), std::string(buf));

  FPDF_ClosePage(page);
}

TEST_F(FPDFDocEmbeddertest, NoBookmarks) {
  // Open a file with no bookmarks.
  EXPECT_TRUE(OpenDocument("testing/resources/named_dests.pdf"));

  // The non-existent top-level bookmark has no title.
  unsigned short buf[128];
  EXPECT_EQ(0, FPDFBookmark_GetTitle(nullptr, buf, sizeof(buf)));

  // The non-existent top-level bookmark has no children.
  EXPECT_EQ(nullptr, FPDFBookmark_GetFirstChild(document(), nullptr));
}

TEST_F(FPDFDocEmbeddertest, Bookmarks) {
  // Open a file with two bookmarks.
  EXPECT_TRUE(OpenDocument("testing/resources/bookmarks.pdf"));

  // The existent top-level bookmark has no title.
  unsigned short buf[128];
  EXPECT_EQ(0, FPDFBookmark_GetTitle(nullptr, buf, sizeof(buf)));

  FPDF_BOOKMARK child = FPDFBookmark_GetFirstChild(document(), nullptr);
  EXPECT_NE(nullptr, child);
  EXPECT_EQ(34, FPDFBookmark_GetTitle(child, buf, sizeof(buf)));
  EXPECT_EQ(CFX_WideString(L"A Good Beginning"),
            CFX_WideString::FromUTF16LE(buf, 16));

  EXPECT_EQ(nullptr, FPDFBookmark_GetFirstChild(document(), child));

  FPDF_BOOKMARK sibling = FPDFBookmark_GetNextSibling(document(), child);
  EXPECT_NE(nullptr, sibling);
  EXPECT_EQ(28, FPDFBookmark_GetTitle(sibling, buf, sizeof(buf)));
  EXPECT_EQ(CFX_WideString(L"A Good Ending"),
            CFX_WideString::FromUTF16LE(buf, 13));

  EXPECT_EQ(nullptr, FPDFBookmark_GetNextSibling(document(), sibling));
}
