// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_edit.h"

#include "public/cpp/fpdf_scopers.h"
#include "testing/embedder_test.h"
#include "testing/utils/file_util.h"

class PDFEditImgTest : public EmbedderTest {};

TEST_F(PDFEditImgTest, InsertObjectWithInvalidPage) {
  FPDF_DOCUMENT doc = FPDF_CreateNewDocument();
  FPDF_PAGE page = FPDFPage_New(doc, 0, 100, 100);
  EXPECT_EQ(0, FPDFPage_CountObjects(page));

  FPDFPage_InsertObject(nullptr, nullptr);
  EXPECT_EQ(0, FPDFPage_CountObjects(page));

  FPDFPage_InsertObject(page, nullptr);
  EXPECT_EQ(0, FPDFPage_CountObjects(page));

  FPDF_PAGEOBJECT page_image = FPDFPageObj_NewImageObj(doc);
  FPDFPage_InsertObject(nullptr, page_image);
  EXPECT_EQ(0, FPDFPage_CountObjects(page));

  FPDF_ClosePage(page);
  FPDF_CloseDocument(doc);
}

TEST_F(PDFEditImgTest, NewImageObj) {
  FPDF_DOCUMENT doc = FPDF_CreateNewDocument();
  FPDF_PAGE page = FPDFPage_New(doc, 0, 100, 100);
  EXPECT_EQ(0, FPDFPage_CountObjects(page));

  FPDF_PAGEOBJECT page_image = FPDFPageObj_NewImageObj(doc);
  FPDFPage_InsertObject(page, page_image);
  EXPECT_EQ(1, FPDFPage_CountObjects(page));
  EXPECT_TRUE(FPDFPage_GenerateContent(page));

  FPDF_ClosePage(page);
  FPDF_CloseDocument(doc);
}

TEST_F(PDFEditImgTest, NewImageObjGenerateContent) {
  FPDF_DOCUMENT doc = FPDF_CreateNewDocument();
  FPDF_PAGE page = FPDFPage_New(doc, 0, 100, 100);
  EXPECT_EQ(0, FPDFPage_CountObjects(page));

  constexpr int kBitmapSize = 50;
  FPDF_BITMAP bitmap = FPDFBitmap_Create(kBitmapSize, kBitmapSize, 0);
  FPDFBitmap_FillRect(bitmap, 0, 0, kBitmapSize, kBitmapSize, 0x00000000);
  EXPECT_EQ(kBitmapSize, FPDFBitmap_GetWidth(bitmap));
  EXPECT_EQ(kBitmapSize, FPDFBitmap_GetHeight(bitmap));

  FPDF_PAGEOBJECT page_image = FPDFPageObj_NewImageObj(doc);
  ASSERT_TRUE(FPDFImageObj_SetBitmap(&page, 0, page_image, bitmap));
  static constexpr FS_MATRIX kScaleBitmapMatrix{kBitmapSize, 0, 0,
                                                kBitmapSize, 0, 0};
  ASSERT_TRUE(FPDFPageObj_SetMatrix(page_image, &kScaleBitmapMatrix));
  FPDFPage_InsertObject(page, page_image);
  EXPECT_EQ(1, FPDFPage_CountObjects(page));
  EXPECT_TRUE(FPDFPage_GenerateContent(page));

  FPDFBitmap_Destroy(bitmap);
  FPDF_ClosePage(page);
  FPDF_CloseDocument(doc);
}

TEST_F(PDFEditImgTest, NewImageObjLoadJpeg) {
  ScopedFPDFDocument doc(FPDF_CreateNewDocument());
  ScopedFPDFPage page(FPDFPage_New(doc.get(), 0, 200, 200));
  ASSERT_TRUE(page);

  ScopedFPDFPageObject image(FPDFPageObj_NewImageObj(doc.get()));
  ASSERT_TRUE(image);

  FileAccessForTesting file_access("mona_lisa.jpg");
  FPDF_PAGE temp_page = page.get();
  EXPECT_TRUE(
      FPDFImageObj_LoadJpegFile(&temp_page, 1, image.get(), &file_access));

  ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(image.get()));
  EXPECT_TRUE(bitmap);
  EXPECT_EQ(120, FPDFBitmap_GetWidth(bitmap.get()));
  EXPECT_EQ(120, FPDFBitmap_GetHeight(bitmap.get()));
}

TEST_F(PDFEditImgTest, NewImageObjLoadJpegInline) {
  ScopedFPDFDocument doc(FPDF_CreateNewDocument());
  ScopedFPDFPage page(FPDFPage_New(doc.get(), 0, 200, 200));
  ASSERT_TRUE(page);

  ScopedFPDFPageObject image(FPDFPageObj_NewImageObj(doc.get()));
  ASSERT_TRUE(image);

  FileAccessForTesting file_access("mona_lisa.jpg");
  FPDF_PAGE temp_page = page.get();
  EXPECT_TRUE(FPDFImageObj_LoadJpegFileInline(&temp_page, 1, image.get(),
                                              &file_access));

  ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(image.get()));
  EXPECT_TRUE(bitmap);
  EXPECT_EQ(120, FPDFBitmap_GetWidth(bitmap.get()));
  EXPECT_EQ(120, FPDFBitmap_GetHeight(bitmap.get()));
}

TEST_F(PDFEditImgTest, SetBitmap) {
  ScopedFPDFDocument doc(FPDF_CreateNewDocument());
  ScopedFPDFPage page(FPDFPage_New(doc.get(), 0, 100, 100));
  ScopedFPDFPageObject image(FPDFPageObj_NewImageObj(doc.get()));
  ScopedFPDFBitmap bitmap(FPDFBitmap_Create(100, 100, 0));

  FPDF_PAGE page_ptr = page.get();
  FPDF_PAGE* pages = &page_ptr;
  EXPECT_TRUE(FPDFImageObj_SetBitmap(nullptr, 1, image.get(), bitmap.get()));
  EXPECT_TRUE(FPDFImageObj_SetBitmap(pages, 0, image.get(), bitmap.get()));
  EXPECT_FALSE(FPDFImageObj_SetBitmap(pages, 1, nullptr, bitmap.get()));
  EXPECT_FALSE(FPDFImageObj_SetBitmap(pages, 1, image.get(), nullptr));
}

TEST_F(PDFEditImgTest, GetSetImageMatrix) {
  FPDF_DOCUMENT doc = FPDF_CreateNewDocument();
  FPDF_PAGEOBJECT image = FPDFPageObj_NewImageObj(doc);

  FS_MATRIX matrix;
  EXPECT_FALSE(FPDFPageObj_GetMatrix(nullptr, nullptr));
  EXPECT_FALSE(FPDFPageObj_GetMatrix(nullptr, &matrix));
  EXPECT_FALSE(FPDFPageObj_GetMatrix(image, nullptr));

  EXPECT_TRUE(FPDFPageObj_GetMatrix(image, &matrix));
  EXPECT_FLOAT_EQ(1.0f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(1.0f, matrix.d);
  EXPECT_FLOAT_EQ(0.0f, matrix.e);
  EXPECT_FLOAT_EQ(0.0f, matrix.f);

  static constexpr FS_MATRIX kMatrix{1, 2, 3, 4, 5, 6};
  EXPECT_TRUE(FPDFPageObj_SetMatrix(image, &kMatrix));
  EXPECT_TRUE(FPDFPageObj_GetMatrix(image, &matrix));
  EXPECT_FLOAT_EQ(1.0f, matrix.a);
  EXPECT_FLOAT_EQ(2.0f, matrix.b);
  EXPECT_FLOAT_EQ(3.0f, matrix.c);
  EXPECT_FLOAT_EQ(4.0f, matrix.d);
  EXPECT_FLOAT_EQ(5.0f, matrix.e);
  EXPECT_FLOAT_EQ(6.0f, matrix.f);

  FPDFPageObj_Destroy(image);
  FPDF_CloseDocument(doc);
}
