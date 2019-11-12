// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_system.h"
#include "public/fpdf_edit.h"
#include "testing/embedder_test.h"

class FPDFEditPageEmbedderTest : public EmbedderTest {};

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_Rotation DISABLED_Rotation
#else
#define MAYBE_Rotation Rotation
#endif
TEST_F(FPDFEditPageEmbedderTest, MAYBE_Rotation) {
  const char kOriginalMD5[] = "0a90de37f52127619c3dfb642b5fa2fe";
  const char kRotatedMD5[] = "d599429574ff0dcad3bc898ea8b874ca";

  {
    ASSERT_TRUE(OpenDocument("rectangles.pdf"));
    FPDF_PAGE page = LoadPage(0);
    ASSERT_TRUE(page);

    {
      // Render the page as is.
      EXPECT_EQ(0, FPDFPage_GetRotation(page));
      const int page_width = static_cast<int>(FPDF_GetPageWidth(page));
      const int page_height = static_cast<int>(FPDF_GetPageHeight(page));
      EXPECT_EQ(200, page_width);
      EXPECT_EQ(300, page_height);
      ScopedFPDFBitmap bitmap = RenderLoadedPage(page);
      CompareBitmap(bitmap.get(), page_width, page_height, kOriginalMD5);
    }

    FPDFPage_SetRotation(page, 1);

    {
      // Render the page after rotation.
      // Note that the change affects the rendering, as expected.
      // It behaves just like the case below, rather than the case above.
      EXPECT_EQ(1, FPDFPage_GetRotation(page));
      const int page_width = static_cast<int>(FPDF_GetPageWidth(page));
      const int page_height = static_cast<int>(FPDF_GetPageHeight(page));
      EXPECT_EQ(300, page_width);
      EXPECT_EQ(200, page_height);
      ScopedFPDFBitmap bitmap = RenderLoadedPage(page);
      CompareBitmap(bitmap.get(), page_width, page_height, kRotatedMD5);
    }

    UnloadPage(page);
  }

  {
    // Save a copy, open the copy, and render it.
    // Note that it renders the rotation.
    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
    ASSERT_TRUE(OpenSavedDocument());
    FPDF_PAGE saved_page = LoadSavedPage(0);
    ASSERT_TRUE(saved_page);

    EXPECT_EQ(1, FPDFPage_GetRotation(saved_page));
    const int page_width = static_cast<int>(FPDF_GetPageWidth(saved_page));
    const int page_height = static_cast<int>(FPDF_GetPageHeight(saved_page));
    EXPECT_EQ(300, page_width);
    EXPECT_EQ(200, page_height);
    ScopedFPDFBitmap bitmap = RenderSavedPage(saved_page);
    CompareBitmap(bitmap.get(), page_width, page_height, kRotatedMD5);

    CloseSavedPage(saved_page);
    CloseSavedDocument();
  }
}

TEST_F(FPDFEditPageEmbedderTest, HasTransparencyImage) {
  constexpr int kExpectedObjectCount = 39;
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(kExpectedObjectCount, FPDFPage_CountObjects(page));

  for (int i = 0; i < kExpectedObjectCount; ++i) {
    FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, i);
    EXPECT_FALSE(FPDFPageObj_HasTransparency(obj));

    FPDFPageObj_SetFillColor(obj, 255, 0, 0, 127);
    EXPECT_TRUE(FPDFPageObj_HasTransparency(obj));
  }

  UnloadPage(page);
}

TEST_F(FPDFEditPageEmbedderTest, HasTransparencyInvalid) {
  EXPECT_FALSE(FPDFPageObj_HasTransparency(nullptr));
}

TEST_F(FPDFEditPageEmbedderTest, HasTransparencyPath) {
  constexpr int kExpectedObjectCount = 8;
  EXPECT_TRUE(OpenDocument("rectangles.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(kExpectedObjectCount, FPDFPage_CountObjects(page));

  for (int i = 0; i < kExpectedObjectCount; ++i) {
    FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, i);
    EXPECT_FALSE(FPDFPageObj_HasTransparency(obj));

    FPDFPageObj_SetStrokeColor(obj, 63, 63, 0, 127);
    EXPECT_TRUE(FPDFPageObj_HasTransparency(obj));
  }

  UnloadPage(page);
}

TEST_F(FPDFEditPageEmbedderTest, HasTransparencyText) {
  constexpr int kExpectedObjectCount = 2;
  EXPECT_TRUE(OpenDocument("text_render_mode.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(kExpectedObjectCount, FPDFPage_CountObjects(page));

  for (int i = 0; i < kExpectedObjectCount; ++i) {
    FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, i);
    EXPECT_FALSE(FPDFPageObj_HasTransparency(obj));

    FPDFPageObj_SetBlendMode(obj, "Lighten");
    EXPECT_TRUE(FPDFPageObj_HasTransparency(obj));
  }

  UnloadPage(page);
}

TEST_F(FPDFEditPageEmbedderTest, GetFillAndStrokeForImage) {
  constexpr int kExpectedObjectCount = 39;
  constexpr int kImageObjectsStartIndex = 33;
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  ASSERT_EQ(kExpectedObjectCount, FPDFPage_CountObjects(page));

  for (int i = kImageObjectsStartIndex; i < kExpectedObjectCount; ++i) {
    FPDF_PAGEOBJECT image = FPDFPage_GetObject(page, i);
    ASSERT_TRUE(image);
    EXPECT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(image));

    unsigned int r;
    unsigned int g;
    unsigned int b;
    unsigned int a;
    EXPECT_FALSE(FPDFPageObj_GetFillColor(image, &r, &g, &b, &a));
    EXPECT_FALSE(FPDFPageObj_GetStrokeColor(image, &r, &g, &b, &a));
  }

  UnloadPage(page);
}
