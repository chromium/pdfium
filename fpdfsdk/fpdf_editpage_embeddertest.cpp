// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_system.h"
#include "public/fpdf_edit.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_constants.h"

class FPDFEditPageEmbedderTest : public EmbedderTest {};

TEST_F(FPDFEditPageEmbedderTest, Rotation) {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  const char kRotatedMD5[] = "eded83f75f3d0332c584c416c571c0df";
#else
  const char kRotatedMD5[] = "d599429574ff0dcad3bc898ea8b874ca";
#endif

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
      CompareBitmap(bitmap.get(), page_width, page_height,
                    pdfium::kRectanglesChecksum);
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
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));
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
  ASSERT_TRUE(OpenDocument("text_render_mode.pdf"));
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

TEST_F(FPDFEditPageEmbedderTest, DashingArrayAndPhase) {
  {
    EXPECT_FALSE(FPDFPageObj_GetDashPhase(nullptr, nullptr));

    float phase = -1123.5f;
    EXPECT_FALSE(FPDFPageObj_GetDashPhase(nullptr, &phase));
    EXPECT_FLOAT_EQ(-1123.5f, phase);

    EXPECT_EQ(-1, FPDFPageObj_GetDashCount(nullptr));

    EXPECT_FALSE(FPDFPageObj_GetDashArray(nullptr, nullptr, 3));

    float get_array[] = {-1.0f, -1.0f, -1.0f};
    EXPECT_FALSE(FPDFPageObj_GetDashArray(nullptr, get_array, 3));
    for (int i = 0; i < 3; i++)
      EXPECT_FLOAT_EQ(-1.0f, get_array[i]);

    EXPECT_FALSE(FPDFPageObj_SetDashPhase(nullptr, 5.0f));
    EXPECT_FALSE(FPDFPageObj_SetDashArray(nullptr, nullptr, 3, 5.0f));

    float set_array[] = {1.0f, 2.0f, 3.0f};
    EXPECT_FALSE(FPDFPageObj_SetDashArray(nullptr, set_array, 3, 5.0f));
  }

  constexpr int kExpectedObjectCount = 3;
  ASSERT_TRUE(OpenDocument("dashed_lines.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  ASSERT_EQ(kExpectedObjectCount, FPDFPage_CountObjects(page));

  {
    FPDF_PAGEOBJECT path = FPDFPage_GetObject(page, 0);
    ASSERT_TRUE(path);
    EXPECT_EQ(FPDF_PAGEOBJ_PATH, FPDFPageObj_GetType(path));

    EXPECT_FALSE(FPDFPageObj_GetDashPhase(path, nullptr));
    EXPECT_FALSE(FPDFPageObj_GetDashArray(path, nullptr, 3));
    EXPECT_FALSE(FPDFPageObj_SetDashArray(path, nullptr, 3, 5.0f));

    float phase = -1123.5f;
    EXPECT_TRUE(FPDFPageObj_GetDashPhase(path, &phase));
    EXPECT_FLOAT_EQ(0.0f, phase);
    EXPECT_EQ(0, FPDFPageObj_GetDashCount(path));

    float get_array[] = {-1.0f, -1.0f, -1.0f};
    EXPECT_TRUE(FPDFPageObj_GetDashArray(path, get_array, 3));
    for (int i = 0; i < 3; i++)
      EXPECT_FLOAT_EQ(-1.0f, get_array[i]);
  }

  {
    FPDF_PAGEOBJECT path = FPDFPage_GetObject(page, 1);
    ASSERT_TRUE(path);
    EXPECT_EQ(FPDF_PAGEOBJ_PATH, FPDFPageObj_GetType(path));

    float phase = -1123.5f;
    EXPECT_TRUE(FPDFPageObj_GetDashPhase(path, &phase));
    EXPECT_LT(0.0f, phase);
    ASSERT_EQ(6, FPDFPageObj_GetDashCount(path));

    float dash_array[] = {-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f};
    ASSERT_TRUE(FPDFPageObj_GetDashArray(path, dash_array, 6));

    for (int i = 0; i < 6; i++)
      EXPECT_LT(0.0f, dash_array[i]);

    // the array is decreasing in value.
    for (int i = 0; i < 5; i++)
      EXPECT_GT(dash_array[i], dash_array[i + 1]);

    // modify phase
    EXPECT_TRUE(FPDFPageObj_SetDashPhase(path, 1.0f));

    phase = -1123.5f;
    EXPECT_TRUE(FPDFPageObj_GetDashPhase(path, &phase));
    EXPECT_FLOAT_EQ(1.0f, phase);

    // clear array
    EXPECT_TRUE(FPDFPageObj_SetDashArray(path, nullptr, 0, 0.0f));
    EXPECT_EQ(0, FPDFPageObj_GetDashCount(path));

    phase = -1123.5f;
    EXPECT_TRUE(FPDFPageObj_GetDashPhase(path, &phase));
    EXPECT_FLOAT_EQ(0.0f, phase);
  }

  {
    FPDF_PAGEOBJECT path = FPDFPage_GetObject(page, 2);
    ASSERT_TRUE(path);
    EXPECT_EQ(FPDF_PAGEOBJ_PATH, FPDFPageObj_GetType(path));

    float phase = -1123.5f;
    EXPECT_TRUE(FPDFPageObj_GetDashPhase(path, &phase));
    EXPECT_FLOAT_EQ(0.0f, phase);

    EXPECT_EQ(0, FPDFPageObj_GetDashCount(path));

    // `get_array` should be unmodified
    float get_array[] = {-1.0f, -1.0f, -1.0f, -1.0f};
    EXPECT_TRUE(FPDFPageObj_GetDashArray(path, get_array, 4));
    for (int i = 0; i < 4; i++)
      EXPECT_FLOAT_EQ(-1.0f, get_array[i]);

    // modify dash_array and phase
    const float set_array[] = {1.0f, 2.0f, 3.0f};
    EXPECT_TRUE(FPDFPageObj_SetDashArray(path, set_array, 3, 5.0f));

    phase = -1123.5f;
    EXPECT_TRUE(FPDFPageObj_GetDashPhase(path, &phase));
    EXPECT_FLOAT_EQ(5.0f, phase);
    ASSERT_EQ(3, FPDFPageObj_GetDashCount(path));

    ASSERT_TRUE(FPDFPageObj_GetDashArray(path, get_array, 4));

    // `get_array` should be modified only up to dash_count
    for (int i = 0; i < 3; i++)
      EXPECT_FLOAT_EQ(static_cast<float>(i + 1), get_array[i]);

    EXPECT_FLOAT_EQ(-1.0f, get_array[3]);

    // clear array
    EXPECT_TRUE(FPDFPageObj_SetDashArray(path, set_array, 0, 4.0f));
    EXPECT_EQ(0, FPDFPageObj_GetDashCount(path));

    phase = -1123.5f;
    EXPECT_TRUE(FPDFPageObj_GetDashPhase(path, &phase));
    EXPECT_FLOAT_EQ(4.0f, phase);
  }

  UnloadPage(page);
}
