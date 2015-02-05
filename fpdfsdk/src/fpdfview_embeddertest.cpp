// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>
#include <string>

#include "../../testing/embedder_test.h"
#include "../../fpdfsdk/include/fpdfview.h"
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
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
  EXPECT_EQ(612.0, FPDF_GetPageWidth(page));
  EXPECT_EQ(792.0, FPDF_GetPageHeight(page));
  UnloadPage(page);
  EXPECT_EQ(nullptr, LoadPage(1));
}

TEST_F(FPDFViewEmbeddertest, ViewerRef) {
  EXPECT_TRUE(OpenDocument("testing/resources/about_blank.pdf"));
  EXPECT_TRUE(FPDF_VIEWERREF_GetPrintScaling(document()));
  EXPECT_EQ(1, FPDF_VIEWERREF_GetNumCopies(document()));
  EXPECT_EQ(DuplexUndefined, FPDF_VIEWERREF_GetDuplex(document()));
}

TEST_F(FPDFViewEmbeddertest, NamedDests) {
  EXPECT_TRUE(OpenDocument("testing/resources/named_dests.pdf"));
  long buffer_size;
  char fixed_buffer[512];
  FPDF_DEST dest;

  // Query the size of the first item.
  buffer_size = 2000000; // Absurdly large, check not used for this case.
  dest = FPDF_GetNamedDest(document(), 0, nullptr, buffer_size);
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(12u, buffer_size);

  // Try to retrieve the first item with too small a buffer.
  buffer_size = 10;
  dest = FPDF_GetNamedDest(document(), 0, fixed_buffer, buffer_size);
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(-1, buffer_size);

  // Try to retrieve the first item with correctly sized buffer. Item is
  // taken from Dests NameTree in named_dests.pdf.
  buffer_size = 12;
  dest = FPDF_GetNamedDest(document(), 0, fixed_buffer, buffer_size);
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(12u, buffer_size);
  EXPECT_EQ(std::string("F\0i\0r\0s\0t\0\0\0", 12),
            std::string(fixed_buffer, buffer_size));

  // Try to retrieve the second item with ample buffer. Item is taken
  // from Dests NameTree but has a sub-dictionary in named_dests.pdf.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 1, fixed_buffer, buffer_size);
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(10u, buffer_size);
  EXPECT_EQ(std::string("N\0e\0x\0t\0\0\0", 10),
            std::string(fixed_buffer, buffer_size));

  // Try to retrieve third item with ample buffer. Item is taken
  // from Dests NameTree but has a bad sub-dictionary in named_dests.pdf.
  // in named_dests.pdf).
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 2, fixed_buffer, buffer_size);
  EXPECT_EQ(nullptr, dest);
  EXPECT_EQ(sizeof(fixed_buffer), buffer_size);  // unmodified.

  // Try to retrieve the forth item with ample buffer. Item is taken
  // from Dests NameTree but has a vale of the wrong type in named_dests.pdf.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 3, fixed_buffer, buffer_size);
  EXPECT_EQ(nullptr, dest);
  EXPECT_EQ(sizeof(fixed_buffer), buffer_size);  // unmodified.

  // Try to retrieve fifth item with ample buffer. Item taken from the
  // old-style Dests dictionary object in named_dests.pdf.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 4, fixed_buffer, buffer_size);
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(30u, buffer_size);
  EXPECT_EQ(
      std::string("F\0i\0r\0s\0t\0A\0l\0t\0e\0r\0n\0a\0t\0e\0\0\0", 30),
      std::string(fixed_buffer, buffer_size));

  // Try to retrieve sixth item with ample buffer. Item istaken from the
  // old-style Dests dictionary object but has a sub-dictionary in
  // named_dests.pdf.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 5, fixed_buffer, buffer_size);
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(28u, buffer_size);
  EXPECT_EQ(
      std::string("L\0a\0s\0t\0A\0l\0t\0e\0r\0n\0a\0t\0e\0\0\0", 28),
      std::string(fixed_buffer, buffer_size));

  // Try to retrieve non-existent item with ample buffer.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 6, fixed_buffer, buffer_size);
  EXPECT_EQ(nullptr, dest);
  EXPECT_EQ(sizeof(fixed_buffer), buffer_size);  // unmodified.

  // Try to underflow/overflow the integer index.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), std::numeric_limits<int>::max(),
                           fixed_buffer, buffer_size);
  EXPECT_EQ(nullptr, dest);
  EXPECT_EQ(sizeof(fixed_buffer), buffer_size);  // unmodified.

  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), std::numeric_limits<int>::min(),
                           fixed_buffer, buffer_size);
  EXPECT_EQ(nullptr, dest);
  EXPECT_EQ(sizeof(fixed_buffer), buffer_size);  // unmodified.

  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), -1, fixed_buffer, buffer_size);
  EXPECT_EQ(nullptr, dest);
  EXPECT_EQ(sizeof(fixed_buffer), buffer_size);  // unmodified.
}

TEST_F(FPDFViewEmbeddertest, NamedDestsByName) {
  EXPECT_TRUE(OpenDocument("testing/resources/named_dests.pdf"));

  // Null pointer returns NULL.
  FPDF_DEST dest = FPDF_GetNamedDestByName(document(), nullptr);
  EXPECT_EQ(nullptr, dest);

  // Empty string returns NULL.
  dest = FPDF_GetNamedDestByName(document(), "");
  EXPECT_EQ(nullptr, dest);

  // Item from Dests NameTree.
  dest = FPDF_GetNamedDestByName(document(), "First");
  EXPECT_NE(nullptr, dest);

  long ignore_len = 0;
  FPDF_DEST dest_by_index =
      FPDF_GetNamedDest(document(), 0, nullptr, ignore_len);
  EXPECT_EQ(dest_by_index, dest);

  // Item from Dests dictionary.
  dest = FPDF_GetNamedDestByName(document(), "FirstAlternate");
  EXPECT_NE(nullptr, dest);

  ignore_len = 0;
  dest_by_index = FPDF_GetNamedDest(document(), 4, nullptr, ignore_len);
  EXPECT_EQ(dest_by_index, dest);

  // Bad value type for item from Dests NameTree array.
  dest = FPDF_GetNamedDestByName(document(), "WrongType");
  EXPECT_EQ(nullptr, dest);

  // No such destination in either Dest NameTree or dictionary.
  dest = FPDF_GetNamedDestByName(document(), "Bogus");
  EXPECT_EQ(nullptr, dest);
}

// The following tests pass if the document opens without crashing.
TEST_F(FPDFViewEmbeddertest, Crasher_113) {
  EXPECT_TRUE(OpenDocument("testing/resources/bug_113.pdf"));
}

TEST_F(FPDFViewEmbeddertest, Crasher_451830) {
  EXPECT_TRUE(OpenDocument("testing/resources/bug_451830.pdf"));
}

TEST_F(FPDFViewEmbeddertest, Crasher_452455) {
  EXPECT_TRUE(OpenDocument("testing/resources/bug_452455.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
}

TEST_F(FPDFViewEmbeddertest, Crasher3) {
  EXPECT_TRUE(OpenDocument("testing/resources/bug_454695.pdf"));
}
