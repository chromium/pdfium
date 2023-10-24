// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "public/fpdf_edit.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_constants.h"

class FPDFEditPageEmbedderTest : public EmbedderTest {};

TEST_F(FPDFEditPageEmbedderTest, Rotation) {
  const char* rotated_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "eded83f75f3d0332c584c416c571c0df";
    }
    return "d599429574ff0dcad3bc898ea8b874ca";
  }();

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
                    pdfium::RectanglesChecksum());
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
      CompareBitmap(bitmap.get(), page_width, page_height, rotated_checksum);
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
    CompareBitmap(bitmap.get(), page_width, page_height, rotated_checksum);

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

    // Pretend `get_array` has too few members.
    EXPECT_FALSE(FPDFPageObj_GetDashArray(path, get_array, 2));
    for (int i = 0; i < 4; i++)
      EXPECT_FLOAT_EQ(-1.0f, get_array[i]);

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

TEST_F(FPDFEditPageEmbedderTest, GetRotatedBoundsBadParameters) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, 0);
  ASSERT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(obj));

  FS_QUADPOINTSF quad;
  ASSERT_FALSE(FPDFPageObj_GetRotatedBounds(nullptr, nullptr));
  ASSERT_FALSE(FPDFPageObj_GetRotatedBounds(obj, nullptr));
  ASSERT_FALSE(FPDFPageObj_GetRotatedBounds(nullptr, &quad));

  UnloadPage(page);
}

TEST_F(FPDFEditPageEmbedderTest, GetBoundsForNormalText) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, 0);
  ASSERT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(obj));

  constexpr float kExpectedLeft = 20.348f;
  constexpr float kExpectedBottom = 48.164f;
  constexpr float kExpectedRight = 83.36f;
  constexpr float kExpectedTop = 58.328f;

  float left;
  float bottom;
  float right;
  float top;
  ASSERT_TRUE(FPDFPageObj_GetBounds(obj, &left, &bottom, &right, &top));
  EXPECT_FLOAT_EQ(kExpectedLeft, left);
  EXPECT_FLOAT_EQ(kExpectedBottom, bottom);
  EXPECT_FLOAT_EQ(kExpectedRight, right);
  EXPECT_FLOAT_EQ(kExpectedTop, top);

  FS_QUADPOINTSF quad;
  ASSERT_TRUE(FPDFPageObj_GetRotatedBounds(obj, &quad));
  EXPECT_FLOAT_EQ(kExpectedLeft, quad.x1);
  EXPECT_FLOAT_EQ(kExpectedBottom, quad.y1);
  EXPECT_FLOAT_EQ(kExpectedRight, quad.x2);
  EXPECT_FLOAT_EQ(kExpectedBottom, quad.y2);
  EXPECT_FLOAT_EQ(kExpectedRight, quad.x3);
  EXPECT_FLOAT_EQ(kExpectedTop, quad.y3);
  EXPECT_FLOAT_EQ(kExpectedLeft, quad.x4);
  EXPECT_FLOAT_EQ(kExpectedTop, quad.y4);

  UnloadPage(page);
}

TEST_F(FPDFEditPageEmbedderTest, GetBoundsForRotatedText) {
  ASSERT_TRUE(OpenDocument("rotated_text.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, 0);
  ASSERT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(obj));

  constexpr float kExpectedLeft = 98.9478f;
  constexpr float kExpectedBottom = 78.2607f;
  constexpr float kExpectedRight = 126.32983f;
  constexpr float kExpectedTop = 105.64272f;

  float left;
  float bottom;
  float right;
  float top;
  ASSERT_TRUE(FPDFPageObj_GetBounds(obj, &left, &bottom, &right, &top));
  EXPECT_FLOAT_EQ(kExpectedLeft, left);
  EXPECT_FLOAT_EQ(kExpectedBottom, bottom);
  EXPECT_FLOAT_EQ(kExpectedRight, right);
  EXPECT_FLOAT_EQ(kExpectedTop, top);

  FS_QUADPOINTSF quad;
  ASSERT_TRUE(FPDFPageObj_GetRotatedBounds(obj, &quad));
  EXPECT_FLOAT_EQ(kExpectedLeft, quad.x1);
  EXPECT_FLOAT_EQ(98.4557f, quad.y1);
  EXPECT_FLOAT_EQ(119.14279f, quad.x2);
  EXPECT_FLOAT_EQ(kExpectedBottom, quad.y2);
  EXPECT_FLOAT_EQ(kExpectedRight, quad.x3);
  EXPECT_FLOAT_EQ(85.447739f, quad.y3);
  EXPECT_FLOAT_EQ(106.13486f, quad.x4);
  EXPECT_FLOAT_EQ(kExpectedTop, quad.y4);

  UnloadPage(page);
}

TEST_F(FPDFEditPageEmbedderTest, GetBoundsForNormalImage) {
  ASSERT_TRUE(OpenDocument("matte.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, 2);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));

  constexpr float kExpectedLeft = 0.0f;
  constexpr float kExpectedBottom = 90.0f;
  constexpr float kExpectedRight = 40.0f;
  constexpr float kExpectedTop = 150.0f;

  float left;
  float bottom;
  float right;
  float top;
  ASSERT_TRUE(FPDFPageObj_GetBounds(obj, &left, &bottom, &right, &top));
  EXPECT_FLOAT_EQ(kExpectedLeft, left);
  EXPECT_FLOAT_EQ(kExpectedBottom, bottom);
  EXPECT_FLOAT_EQ(kExpectedRight, right);
  EXPECT_FLOAT_EQ(kExpectedTop, top);

  FS_QUADPOINTSF quad;
  ASSERT_TRUE(FPDFPageObj_GetRotatedBounds(obj, &quad));
  EXPECT_FLOAT_EQ(kExpectedLeft, quad.x1);
  EXPECT_FLOAT_EQ(kExpectedBottom, quad.y1);
  EXPECT_FLOAT_EQ(kExpectedRight, quad.x2);
  EXPECT_FLOAT_EQ(kExpectedBottom, quad.y2);
  EXPECT_FLOAT_EQ(kExpectedRight, quad.x3);
  EXPECT_FLOAT_EQ(kExpectedTop, quad.y3);
  EXPECT_FLOAT_EQ(kExpectedLeft, quad.x4);
  EXPECT_FLOAT_EQ(kExpectedTop, quad.y4);

  UnloadPage(page);
}

TEST_F(FPDFEditPageEmbedderTest, GetBoundsForRotatedImage) {
  ASSERT_TRUE(OpenDocument("rotated_image.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, 0);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));

  constexpr float kExpectedLeft = 100.0f;
  constexpr float kExpectedBottom = 70.0f;
  constexpr float kExpectedRight = 170.0f;
  constexpr float kExpectedTop = 140.0f;

  float left;
  float bottom;
  float right;
  float top;
  ASSERT_TRUE(FPDFPageObj_GetBounds(obj, &left, &bottom, &right, &top));
  EXPECT_FLOAT_EQ(kExpectedLeft, left);
  EXPECT_FLOAT_EQ(kExpectedBottom, bottom);
  EXPECT_FLOAT_EQ(kExpectedRight, right);
  EXPECT_FLOAT_EQ(kExpectedTop, top);

  FS_QUADPOINTSF quad;
  ASSERT_TRUE(FPDFPageObj_GetRotatedBounds(obj, &quad));
  EXPECT_FLOAT_EQ(kExpectedLeft, quad.x1);
  EXPECT_FLOAT_EQ(100.0f, quad.y1);
  EXPECT_FLOAT_EQ(130.0f, quad.x2);
  EXPECT_FLOAT_EQ(kExpectedBottom, quad.y2);
  EXPECT_FLOAT_EQ(kExpectedRight, quad.x3);
  EXPECT_FLOAT_EQ(110.0f, quad.y3);
  EXPECT_FLOAT_EQ(140.0f, quad.x4);
  EXPECT_FLOAT_EQ(kExpectedTop, quad.y4);

  UnloadPage(page);
}

TEST_F(FPDFEditPageEmbedderTest, VerifyDashArraySaved) {
  constexpr float kDashArray[] = {2.5, 3.6};
  constexpr float kDashPhase = 1.2;

  CreateEmptyDocument();
  {
    ScopedFPDFPage page(FPDFPage_New(document(), 0, 612, 792));

    FPDF_PAGEOBJECT path = FPDFPageObj_CreateNewPath(400, 100);
    EXPECT_TRUE(FPDFPageObj_SetStrokeWidth(path, 2));
    EXPECT_TRUE(FPDFPageObj_SetStrokeColor(path, 255, 0, 0, 255));
    EXPECT_TRUE(FPDFPath_SetDrawMode(path, FPDF_FILLMODE_NONE, 1));
    EXPECT_TRUE(FPDFPath_LineTo(path, 200, 200));
    EXPECT_TRUE(FPDFPageObj_SetDashArray(path, kDashArray,
                                         std::size(kDashArray), kDashPhase));
    FPDFPage_InsertObject(page.get(), path);

    EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
    path = FPDFPage_GetObject(page.get(), 0);
    ASSERT_TRUE(path);
    ASSERT_EQ(2, FPDFPageObj_GetDashCount(path));

    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  }

  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE page = LoadSavedPage(0);
  ASSERT_TRUE(page);

  FPDF_PAGEOBJECT path = FPDFPage_GetObject(page, 0);
  ASSERT_TRUE(path);

  float dash_array[] = {0, 0};
  ASSERT_EQ(static_cast<int>(std::size(dash_array)),
            FPDFPageObj_GetDashCount(path));
  ASSERT_TRUE(
      FPDFPageObj_GetDashArray(path, dash_array, std::size(dash_array)));
  ASSERT_EQ(kDashArray[0], dash_array[0]);
  ASSERT_EQ(kDashArray[1], dash_array[1]);
  float dash_phase = 0;
  ASSERT_TRUE(FPDFPageObj_GetDashPhase(path, &dash_phase));
  ASSERT_EQ(kDashPhase, dash_phase);

  CloseSavedPage(page);
  CloseSavedDocument();
}
