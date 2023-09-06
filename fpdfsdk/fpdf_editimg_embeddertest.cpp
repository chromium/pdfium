// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_edit.h"

#include "public/cpp/fpdf_scopers.h"
#include "testing/embedder_test.h"
#include "testing/utils/file_util.h"

class PDFEditImgTest : public EmbedderTest {};

TEST_F(PDFEditImgTest, InsertObjectWithInvalidPage) {
  ScopedFPDFDocument doc(FPDF_CreateNewDocument());
  ScopedFPDFPage page(FPDFPage_New(doc.get(), 0, 100, 100));
  EXPECT_EQ(0, FPDFPage_CountObjects(page.get()));

  FPDFPage_InsertObject(nullptr, nullptr);
  EXPECT_EQ(0, FPDFPage_CountObjects(page.get()));

  FPDFPage_InsertObject(page.get(), nullptr);
  EXPECT_EQ(0, FPDFPage_CountObjects(page.get()));

  FPDFPage_InsertObject(nullptr, FPDFPageObj_NewImageObj(doc.get()));
  EXPECT_EQ(0, FPDFPage_CountObjects(page.get()));
}

TEST_F(PDFEditImgTest, NewImageObj) {
  ScopedFPDFDocument doc(FPDF_CreateNewDocument());
  ScopedFPDFPage page(FPDFPage_New(doc.get(), 0, 100, 100));
  EXPECT_EQ(0, FPDFPage_CountObjects(page.get()));

  FPDFPage_InsertObject(page.get(), FPDFPageObj_NewImageObj(doc.get()));
  EXPECT_EQ(1, FPDFPage_CountObjects(page.get()));
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
}

TEST_F(PDFEditImgTest, NewImageObjGenerateContent) {
  ScopedFPDFDocument doc(FPDF_CreateNewDocument());
  ScopedFPDFPage page(FPDFPage_New(doc.get(), 0, 100, 100));
  EXPECT_EQ(0, FPDFPage_CountObjects(page.get()));

  constexpr int kBitmapSize = 50;
  ScopedFPDFBitmap bitmap(FPDFBitmap_Create(kBitmapSize, kBitmapSize, 0));
  FPDFBitmap_FillRect(bitmap.get(), 0, 0, kBitmapSize, kBitmapSize, 0x00000000);
  EXPECT_EQ(kBitmapSize, FPDFBitmap_GetWidth(bitmap.get()));
  EXPECT_EQ(kBitmapSize, FPDFBitmap_GetHeight(bitmap.get()));

  ScopedFPDFPageObject page_image(FPDFPageObj_NewImageObj(doc.get()));
  FPDF_PAGE pages_array[] = {page.get()};
  ASSERT_TRUE(
      FPDFImageObj_SetBitmap(pages_array, 0, page_image.get(), bitmap.get()));
  static constexpr FS_MATRIX kScaleBitmapMatrix{kBitmapSize, 0, 0,
                                                kBitmapSize, 0, 0};
  ASSERT_TRUE(FPDFPageObj_SetMatrix(page_image.get(), &kScaleBitmapMatrix));
  FPDFPage_InsertObject(page.get(), page_image.release());
  EXPECT_EQ(1, FPDFPage_CountObjects(page.get()));
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
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
  ScopedFPDFDocument doc(FPDF_CreateNewDocument());
  ScopedFPDFPageObject image(FPDFPageObj_NewImageObj(doc.get()));

  FS_MATRIX matrix;
  EXPECT_FALSE(FPDFPageObj_GetMatrix(nullptr, nullptr));
  EXPECT_FALSE(FPDFPageObj_GetMatrix(nullptr, &matrix));
  EXPECT_FALSE(FPDFPageObj_GetMatrix(image.get(), nullptr));

  EXPECT_TRUE(FPDFPageObj_GetMatrix(image.get(), &matrix));
  EXPECT_FLOAT_EQ(1.0f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(1.0f, matrix.d);
  EXPECT_FLOAT_EQ(0.0f, matrix.e);
  EXPECT_FLOAT_EQ(0.0f, matrix.f);

  static constexpr FS_MATRIX kMatrix{1, 2, 3, 4, 5, 6};
  EXPECT_TRUE(FPDFPageObj_SetMatrix(image.get(), &kMatrix));
  EXPECT_TRUE(FPDFPageObj_GetMatrix(image.get(), &matrix));
  EXPECT_FLOAT_EQ(1.0f, matrix.a);
  EXPECT_FLOAT_EQ(2.0f, matrix.b);
  EXPECT_FLOAT_EQ(3.0f, matrix.c);
  EXPECT_FLOAT_EQ(4.0f, matrix.d);
  EXPECT_FLOAT_EQ(5.0f, matrix.e);
  EXPECT_FLOAT_EQ(6.0f, matrix.f);
}
