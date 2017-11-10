// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <string>

#include "public/fpdf_edit.h"
#include "public/fpdf_ppo.h"
#include "public/fpdf_save.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

namespace {

class FPDFPPOEmbeddertest : public EmbedderTest {};

int FakeBlockWriter(FPDF_FILEWRITE* pThis,
                    const void* pData,
                    unsigned long size) {
  return size;
}

}  // namespace

TEST_F(FPDFPPOEmbeddertest, NoViewerPreferences) {
  EXPECT_TRUE(OpenDocument("hello_world.pdf"));

  FPDF_DOCUMENT output_doc = FPDF_CreateNewDocument();
  EXPECT_TRUE(output_doc);
  EXPECT_FALSE(FPDF_CopyViewerPreferences(output_doc, document()));
  FPDF_CloseDocument(output_doc);
}

TEST_F(FPDFPPOEmbeddertest, ViewerPreferences) {
  EXPECT_TRUE(OpenDocument("viewer_ref.pdf"));

  FPDF_DOCUMENT output_doc = FPDF_CreateNewDocument();
  EXPECT_TRUE(output_doc);
  EXPECT_TRUE(FPDF_CopyViewerPreferences(output_doc, document()));
  FPDF_CloseDocument(output_doc);
}

TEST_F(FPDFPPOEmbeddertest, ImportPages) {
  ASSERT_TRUE(OpenDocument("viewer_ref.pdf"));

  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  FPDF_DOCUMENT output_doc = FPDF_CreateNewDocument();
  ASSERT_TRUE(output_doc);
  EXPECT_TRUE(FPDF_CopyViewerPreferences(output_doc, document()));
  EXPECT_TRUE(FPDF_ImportPages(output_doc, document(), "1", 0));
  EXPECT_EQ(1, FPDF_GetPageCount(output_doc));
  FPDF_CloseDocument(output_doc);

  UnloadPage(page);
}

TEST_F(FPDFPPOEmbeddertest, BadRepeatViewerPref) {
  ASSERT_TRUE(OpenDocument("repeat_viewer_ref.pdf"));

  FPDF_DOCUMENT output_doc = FPDF_CreateNewDocument();
  EXPECT_TRUE(output_doc);
  EXPECT_TRUE(FPDF_CopyViewerPreferences(output_doc, document()));

  FPDF_FILEWRITE writer;
  writer.version = 1;
  writer.WriteBlock = FakeBlockWriter;

  EXPECT_TRUE(FPDF_SaveAsCopy(output_doc, &writer, 0));
  FPDF_CloseDocument(output_doc);
}

TEST_F(FPDFPPOEmbeddertest, BadCircularViewerPref) {
  ASSERT_TRUE(OpenDocument("circular_viewer_ref.pdf"));

  FPDF_DOCUMENT output_doc = FPDF_CreateNewDocument();
  EXPECT_TRUE(output_doc);
  EXPECT_TRUE(FPDF_CopyViewerPreferences(output_doc, document()));

  FPDF_FILEWRITE writer;
  writer.version = 1;
  writer.WriteBlock = FakeBlockWriter;

  EXPECT_TRUE(FPDF_SaveAsCopy(output_doc, &writer, 0));
  FPDF_CloseDocument(output_doc);
}

TEST_F(FPDFPPOEmbeddertest, BadRanges) {
  EXPECT_TRUE(OpenDocument("viewer_ref.pdf"));

  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  FPDF_DOCUMENT output_doc = FPDF_CreateNewDocument();
  EXPECT_TRUE(output_doc);
  EXPECT_FALSE(FPDF_ImportPages(output_doc, document(), "clams", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc, document(), "0", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc, document(), "42", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc, document(), "1,2", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc, document(), "1-2", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc, document(), ",1", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc, document(), "1,", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc, document(), "1-", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc, document(), "-1", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc, document(), "-,0,,,1-", 0));
  FPDF_CloseDocument(output_doc);

  UnloadPage(page);
}

TEST_F(FPDFPPOEmbeddertest, GoodRanges) {
  EXPECT_TRUE(OpenDocument("viewer_ref.pdf"));

  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  FPDF_DOCUMENT output_doc = FPDF_CreateNewDocument();
  EXPECT_TRUE(output_doc);
  EXPECT_TRUE(FPDF_CopyViewerPreferences(output_doc, document()));
  EXPECT_TRUE(FPDF_ImportPages(output_doc, document(), "1,1,1,1", 0));
  EXPECT_TRUE(FPDF_ImportPages(output_doc, document(), "1-1", 0));
  EXPECT_EQ(5, FPDF_GetPageCount(output_doc));
  FPDF_CloseDocument(output_doc);

  UnloadPage(page);
}

TEST_F(FPDFPPOEmbeddertest, BUG_664284) {
  EXPECT_TRUE(OpenDocument("bug_664284.pdf"));

  FPDF_PAGE page = LoadPage(0);
  ASSERT_NE(nullptr, page);

  FPDF_DOCUMENT output_doc = FPDF_CreateNewDocument();
  EXPECT_TRUE(output_doc);
  EXPECT_TRUE(FPDF_ImportPages(output_doc, document(), "1", 0));
  FPDF_CloseDocument(output_doc);

  UnloadPage(page);
}

TEST_F(FPDFPPOEmbeddertest, BUG_750568) {
  const char* const kHashes[] = {
      "64ad08132a1c5a166768298c8a578f57", "83b83e2f6bc80707d0a917c7634140b9",
      "913cd3723a451e4e46fbc2c05702d1ee", "81fb7cfd4860f855eb468f73dfeb6d60"};

  ASSERT_TRUE(OpenDocument("bug_750568.pdf"));
  ASSERT_EQ(4, FPDF_GetPageCount(document()));

  for (size_t i = 0; i < 4; ++i) {
    FPDF_PAGE page = LoadPage(i);
    ASSERT_NE(nullptr, page);

    FPDF_BITMAP bitmap = RenderPage(page);
    ASSERT_EQ(200, FPDFBitmap_GetWidth(bitmap));
    ASSERT_EQ(200, FPDFBitmap_GetHeight(bitmap));
    ASSERT_EQ(800, FPDFBitmap_GetStride(bitmap));

    std::string digest = HashBitmap(bitmap);
    FPDFBitmap_Destroy(bitmap);
    UnloadPage(page);
    EXPECT_EQ(kHashes[i], digest);
  }

  FPDF_DOCUMENT output_doc = FPDF_CreateNewDocument();
  ASSERT_TRUE(output_doc);
  EXPECT_TRUE(FPDF_ImportPages(output_doc, document(), "1,2,3,4", 0));
  ASSERT_EQ(4, FPDF_GetPageCount(output_doc));
  for (size_t i = 0; i < 4; ++i) {
    FPDF_PAGE page = FPDF_LoadPage(output_doc, i);
    ASSERT_NE(nullptr, page);

    FPDF_BITMAP bitmap = RenderPage(page);
    ASSERT_EQ(200, FPDFBitmap_GetWidth(bitmap));
    ASSERT_EQ(200, FPDFBitmap_GetHeight(bitmap));
    ASSERT_EQ(800, FPDFBitmap_GetStride(bitmap));

    std::string digest = HashBitmap(bitmap);
    FPDFBitmap_Destroy(bitmap);
    FPDF_ClosePage(page);
    EXPECT_EQ(kHashes[i], digest);
  }
  FPDF_CloseDocument(output_doc);
}

TEST_F(FPDFPPOEmbeddertest, ImportWithZeroLengthStream) {
  EXPECT_TRUE(OpenDocument("zero_length_stream.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_NE(nullptr, page);

  FPDF_BITMAP bitmap = RenderPage(page);
  ASSERT_EQ(200, FPDFBitmap_GetWidth(bitmap));
  ASSERT_EQ(200, FPDFBitmap_GetHeight(bitmap));
  ASSERT_EQ(800, FPDFBitmap_GetStride(bitmap));

  std::string digest = HashBitmap(bitmap);
  FPDFBitmap_Destroy(bitmap);
  FPDF_ClosePage(page);

  FPDF_DOCUMENT new_doc = FPDF_CreateNewDocument();
  EXPECT_TRUE(new_doc);
  EXPECT_TRUE(FPDF_ImportPages(new_doc, document(), "1", 0));

  EXPECT_EQ(1, FPDF_GetPageCount(new_doc));
  FPDF_PAGE new_page = FPDF_LoadPage(new_doc, 0);
  ASSERT_NE(nullptr, new_page);
  FPDF_BITMAP new_bitmap = RenderPage(new_page);
  ASSERT_EQ(200, FPDFBitmap_GetWidth(new_bitmap));
  ASSERT_EQ(200, FPDFBitmap_GetHeight(new_bitmap));
  ASSERT_EQ(800, FPDFBitmap_GetStride(new_bitmap));

  std::string new_digest = HashBitmap(new_bitmap);
  FPDFBitmap_Destroy(new_bitmap);
  FPDF_ClosePage(new_page);
  FPDF_CloseDocument(new_doc);

  EXPECT_EQ(digest, new_digest);
}
