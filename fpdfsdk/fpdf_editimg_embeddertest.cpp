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
  ASSERT_TRUE(FPDFBitmap_FillRect(bitmap.get(), 0, 0, kBitmapSize, kBitmapSize,
                                  0x00000000));
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
  CreateEmptyDocumentWithoutFormFillEnvironment();
  constexpr int kPageWidth = 200;
  constexpr int kPageHeight = 200;
  ScopedFPDFPage page(FPDFPage_New(document(), 0, kPageWidth, kPageHeight));
  ASSERT_TRUE(page);

  ScopedFPDFPageObject image(FPDFPageObj_NewImageObj(document()));
  ASSERT_TRUE(image);

  FileAccessForTesting file_access("mona_lisa.jpg");
  FPDF_PAGE temp_page = page.get();
  EXPECT_TRUE(
      FPDFImageObj_LoadJpegFile(&temp_page, 1, image.get(), &file_access));

  constexpr int kImageWidth = 120;
  constexpr int kImageHeight = 120;
  const char kImageChecksum[] = "58589c36b3b27a0058f5ca1fbed4d5e5";
  const char kPageChecksum[] = "52b3a852f39c5fa9143e59d805dcb343";
  {
    ScopedFPDFBitmap image_bitmap(FPDFImageObj_GetBitmap(image.get()));
    CompareBitmap(image_bitmap.get(), kImageWidth, kImageHeight,
                  kImageChecksum);
  }

  FPDFImageObj_SetMatrix(image.get(), kImageWidth, 0, 0, kImageHeight, 0, 0);
  FPDFPage_InsertObject(page.get(), image.release());
  FPDFPage_GenerateContent(page.get());
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), kPageWidth, kPageHeight, kPageChecksum);
  }

  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedDocument(kPageWidth, kPageHeight, kPageChecksum);
}

TEST_F(PDFEditImgTest, NewImageObjLoadJpegInline) {
  CreateEmptyDocumentWithoutFormFillEnvironment();
  constexpr int kPageWidth = 200;
  constexpr int kPageHeight = 200;
  ScopedFPDFPage page(FPDFPage_New(document(), 0, kPageWidth, kPageHeight));
  ASSERT_TRUE(page);

  ScopedFPDFPageObject image(FPDFPageObj_NewImageObj(document()));
  ASSERT_TRUE(image);

  FileAccessForTesting file_access("mona_lisa.jpg");
  FPDF_PAGE temp_page = page.get();
  EXPECT_TRUE(FPDFImageObj_LoadJpegFileInline(&temp_page, 1, image.get(),
                                              &file_access));

  constexpr int kImageWidth = 120;
  constexpr int kImageHeight = 120;
  const char kImageChecksum[] = "58589c36b3b27a0058f5ca1fbed4d5e5";
  const char kPageChecksum[] = "52b3a852f39c5fa9143e59d805dcb343";
  {
    ScopedFPDFBitmap image_bitmap(FPDFImageObj_GetBitmap(image.get()));
    CompareBitmap(image_bitmap.get(), kImageWidth, kImageHeight,
                  kImageChecksum);
  }

  FPDFImageObj_SetMatrix(image.get(), kImageWidth, 0, 0, kImageHeight, 0, 0);
  FPDFPage_InsertObject(page.get(), image.release());
  FPDFPage_GenerateContent(page.get());
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), kPageWidth, kPageHeight, kPageChecksum);
  }

  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedDocument(kPageWidth, kPageHeight, kPageChecksum);
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

TEST_F(PDFEditImgTest, Bug2132) {
  constexpr int kExpectedWidth = 200;
  constexpr int kExpectedHeight = 300;
  constexpr char kExpectedChecksum[] = "617b1d57c30c516beee86e0781ff7810";

  OpenDocument("bug_2132.pdf");
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedWidth, kExpectedHeight,
                  kExpectedChecksum);
  }

  FPDF_PAGEOBJECT image = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(image);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(image));

  FS_MATRIX matrix;
  ASSERT_TRUE(FPDFPageObj_GetMatrix(image, &matrix));
  EXPECT_FLOAT_EQ(60.0f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(30.0f, matrix.d);
  EXPECT_FLOAT_EQ(0.0f, matrix.e);
  EXPECT_FLOAT_EQ(270.0f, matrix.f);

  ASSERT_TRUE(FPDFPageObj_SetMatrix(image, &matrix));
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedWidth, kExpectedHeight,
                  kExpectedChecksum);
  }

  ASSERT_TRUE(FPDFPage_GenerateContent(page.get()));
  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedWidth, kExpectedHeight,
                  kExpectedChecksum);
  }


  VerifySavedDocument(kExpectedWidth, kExpectedHeight, kExpectedChecksum);
}

TEST_F(PDFEditImgTest, GetAndSetMatrixForFormWithImage) {
  constexpr int kExpectedWidth = 200;
  constexpr int kExpectedHeight = 300;
  constexpr char kExpectedChecksum[] = "fcb9007fd901d2052e2bd1c147b82800";

  OpenDocument("form_object_with_image.pdf");
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedWidth, kExpectedHeight,
                  kExpectedChecksum);
  }

  FPDF_PAGEOBJECT form = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(form);
  ASSERT_EQ(FPDF_PAGEOBJ_FORM, FPDFPageObj_GetType(form));

  FS_MATRIX matrix;
  ASSERT_TRUE(FPDFPageObj_GetMatrix(form, &matrix));
  EXPECT_FLOAT_EQ(60.0f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(30.0f, matrix.d);
  EXPECT_FLOAT_EQ(0.0f, matrix.e);
  EXPECT_FLOAT_EQ(270.0f, matrix.f);

  ASSERT_TRUE(FPDFPageObj_SetMatrix(form, &matrix));
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedWidth, kExpectedHeight,
                  kExpectedChecksum);
  }

  FPDF_PAGEOBJECT image = FPDFFormObj_GetObject(form, 0);
  ASSERT_TRUE(image);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(image));

  ASSERT_TRUE(FPDFPageObj_GetMatrix(image, &matrix));
  EXPECT_FLOAT_EQ(1.0f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(1.0f, matrix.d);
  EXPECT_FLOAT_EQ(1.0f, matrix.e);
  EXPECT_FLOAT_EQ(0.0f, matrix.f);

  ASSERT_TRUE(FPDFPageObj_SetMatrix(image, &matrix));
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedWidth, kExpectedHeight,
                  kExpectedChecksum);
  }

  ASSERT_TRUE(FPDFPage_GenerateContent(page.get()));
  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedWidth, kExpectedHeight,
                  kExpectedChecksum);
  }


  VerifySavedDocument(kExpectedWidth, kExpectedHeight, kExpectedChecksum);
}
