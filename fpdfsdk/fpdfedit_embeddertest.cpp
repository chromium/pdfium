// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>

#include "public/fpdf_edit.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gmock/include/gmock/gmock-matchers.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

class FPDFEditEmbeddertest : public EmbedderTest, public TestSaver {};

namespace {

const char kExpectedPDF[] =
    "%PDF-1.7\r\n"
    "%\xA1\xB3\xC5\xD7\r\n"
    "1 0 obj\r\n"
    "<</Pages 2 0 R /Type/Catalog>>\r\n"
    "endobj\r\n"
    "2 0 obj\r\n"
    "<</Count 1/Kids\\[ 4 0 R \\]/Type/Pages>>\r\n"
    "endobj\r\n"
    "3 0 obj\r\n"
    "<</CreationDate\\(D:.*\\)/Creator\\(PDFium\\)>>\r\n"
    "endobj\r\n"
    "4 0 obj\r\n"
    "<</Contents 5 0 R /MediaBox\\[ 0 0 640 480\\]"
    "/Parent 2 0 R /Resources<<>>/Rotate 0/Type/Page"
    ">>\r\n"
    "endobj\r\n"
    "5 0 obj\r\n"
    "<</Filter/FlateDecode/Length 8>>stream\r\n"
    // Character '_' is matching '\0' (see comment below).
    "x\x9C\x3____\x1\r\n"
    "endstream\r\n"
    "endobj\r\n"
    "xref\r\n"
    "0 6\r\n"
    "0000000000 65535 f\r\n"
    "0000000017 00000 n\r\n"
    "0000000066 00000 n\r\n"
    "0000000122 00000 n\r\n"
    "0000000192 00000 n\r\n"
    "0000000301 00000 n\r\n"
    "trailer\r\n"
    "<<\r\n"
    "/Root 1 0 R\r\n"
    "/Info 3 0 R\r\n"
    "/Size 6/ID\\[<.*><.*>\\]>>\r\n"
    "startxref\r\n"
    "379\r\n"
    "%%EOF\r\n";

int GetBlockFromString(void* param,
                       unsigned long pos,
                       unsigned char* buf,
                       unsigned long size) {
  std::string* new_file = static_cast<std::string*>(param);
  if (!new_file || pos + size < pos)
    return 0;

  unsigned long file_size = new_file->size();
  if (pos + size > file_size)
    return 0;

  memcpy(buf, new_file->data() + pos, size);
  return 1;
}

}  // namespace

TEST_F(FPDFEditEmbeddertest, EmptyCreation) {
  EXPECT_TRUE(CreateEmptyDocument());
  FPDF_PAGE page = FPDFPage_New(document(), 0, 640.0, 480.0);
  EXPECT_NE(nullptr, page);
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // The MatchesRegexp doesn't support embedded NUL ('\0') characters. They are
  // replaced by '_' for the purpose of the test.
  std::string result = GetString();
  std::replace(result.begin(), result.end(), '\0', '_');
  EXPECT_THAT(result, testing::MatchesRegex(
                          std::string(kExpectedPDF, sizeof(kExpectedPDF))));
  FPDF_ClosePage(page);
}

// Regression test for https://crbug.com/667012
TEST_F(FPDFEditEmbeddertest, RasterizePDF) {
  const char kAllBlackMd5sum[] = "5708fc5c4a8bd0abde99c8e8f0390615";

  // Get the bitmap for the original document/
  FPDF_BITMAP orig_bitmap;
  {
    EXPECT_TRUE(OpenDocument("black.pdf"));
    FPDF_PAGE orig_page = LoadPage(0);
    EXPECT_NE(nullptr, orig_page);
    orig_bitmap = RenderPage(orig_page);
    CompareBitmap(orig_bitmap, 612, 792, kAllBlackMd5sum);
    UnloadPage(orig_page);
  }

  // Create a new document from |orig_bitmap| and save it.
  {
    FPDF_DOCUMENT temp_doc = FPDF_CreateNewDocument();
    FPDF_PAGE temp_page = FPDFPage_New(temp_doc, 0, 612, 792);

    // Add the bitmap to an image object and add the image object to the output
    // page.
    FPDF_PAGEOBJECT temp_img = FPDFPageObj_NewImgeObj(temp_doc);
    EXPECT_TRUE(FPDFImageObj_SetBitmap(&temp_page, 1, temp_img, orig_bitmap));
    EXPECT_TRUE(FPDFImageObj_SetMatrix(temp_img, 612, 0, 0, 792, 0, 0));
    FPDFPage_InsertObject(temp_page, temp_img);
    EXPECT_TRUE(FPDFPage_GenerateContent(temp_page));
    EXPECT_TRUE(FPDF_SaveAsCopy(temp_doc, this, 0));
    FPDF_ClosePage(temp_page);
    FPDF_CloseDocument(temp_doc);
  }
  FPDFBitmap_Destroy(orig_bitmap);

  // Get the generated content. Make sure it is at least as big as the original
  // PDF.
  std::string new_file = GetString();
  EXPECT_GT(new_file.size(), 923U);

  // Read |new_file| in, and verify its rendered bitmap.
  {
    FPDF_FILEACCESS file_access;
    memset(&file_access, 0, sizeof(file_access));
    file_access.m_FileLen = new_file.size();
    file_access.m_GetBlock = GetBlockFromString;
    file_access.m_Param = &new_file;

    FPDF_DOCUMENT new_doc = FPDF_LoadCustomDocument(&file_access, nullptr);
    EXPECT_EQ(1, FPDF_GetPageCount(document_));
    FPDF_PAGE new_page = FPDF_LoadPage(new_doc, 0);
    EXPECT_NE(nullptr, new_page);
    int width = static_cast<int>(FPDF_GetPageWidth(new_page));
    int height = static_cast<int>(FPDF_GetPageHeight(new_page));
    int alpha = FPDFPage_HasTransparency(new_page) ? 1 : 0;
    FPDF_BITMAP new_bitmap = FPDFBitmap_Create(width, height, alpha);
    FPDF_DWORD fill_color = alpha ? 0x00000000 : 0xFFFFFFFF;
    FPDFBitmap_FillRect(new_bitmap, 0, 0, width, height, fill_color);
    FPDF_RenderPageBitmap(new_bitmap, new_page, 0, 0, width, height, 0, 0);
    CompareBitmap(new_bitmap, 612, 792, kAllBlackMd5sum);
    FPDF_ClosePage(new_page);
    FPDF_CloseDocument(new_doc);
    FPDFBitmap_Destroy(new_bitmap);
  }
}
