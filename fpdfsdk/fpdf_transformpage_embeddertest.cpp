// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_transformpage.h"
#include "testing/embedder_test.h"

class FPDFTransformEmbedderTest : public EmbedderTest {};

TEST_F(FPDFTransformEmbedderTest, GetBoundingBoxes) {
  ASSERT_TRUE(OpenDocument("cropped_text.pdf"));
  ASSERT_EQ(4, FPDF_GetPageCount(document()));

  {
    FPDF_PAGE page = LoadPage(1);
    ASSERT_TRUE(page);

    float mediabox_left;
    float mediabox_bottom;
    float mediabox_right;
    float mediabox_top;
    EXPECT_TRUE(FPDFPage_GetMediaBox(page, &mediabox_left, &mediabox_bottom,
                                     &mediabox_right, &mediabox_top));
    EXPECT_EQ(-50, mediabox_left);
    EXPECT_EQ(-50, mediabox_bottom);
    EXPECT_EQ(200, mediabox_right);
    EXPECT_EQ(200, mediabox_top);

    float cropbox_left;
    float cropbox_bottom;
    float cropbox_right;
    float cropbox_top;
    EXPECT_TRUE(FPDFPage_GetCropBox(page, &cropbox_left, &cropbox_bottom,
                                    &cropbox_right, &cropbox_top));
    EXPECT_EQ(50, cropbox_left);
    EXPECT_EQ(50, cropbox_bottom);
    EXPECT_EQ(150, cropbox_right);
    EXPECT_EQ(150, cropbox_top);

    UnloadPage(page);
  }

  {
    FPDF_PAGE page = LoadPage(3);
    ASSERT_TRUE(page);

    float mediabox_left;
    float mediabox_bottom;
    float mediabox_right;
    float mediabox_top;
    EXPECT_TRUE(FPDFPage_GetMediaBox(page, &mediabox_left, &mediabox_bottom,
                                     &mediabox_right, &mediabox_top));
    EXPECT_EQ(0, mediabox_left);
    EXPECT_EQ(0, mediabox_bottom);
    EXPECT_EQ(200, mediabox_right);
    EXPECT_EQ(200, mediabox_top);

    float cropbox_left;
    float cropbox_bottom;
    float cropbox_right;
    float cropbox_top;
    EXPECT_TRUE(FPDFPage_GetCropBox(page, &cropbox_left, &cropbox_bottom,
                                    &cropbox_right, &cropbox_top));
    EXPECT_EQ(150, cropbox_left);
    EXPECT_EQ(150, cropbox_bottom);
    EXPECT_EQ(60, cropbox_right);
    EXPECT_EQ(60, cropbox_top);

    EXPECT_FALSE(FPDFPage_GetCropBox(page, nullptr, &cropbox_bottom,
                                     &cropbox_right, &cropbox_top));
    EXPECT_FALSE(FPDFPage_GetCropBox(page, &cropbox_left, nullptr,
                                     &cropbox_right, &cropbox_top));
    EXPECT_FALSE(FPDFPage_GetCropBox(page, &cropbox_left, &cropbox_bottom,
                                     nullptr, &cropbox_top));
    EXPECT_FALSE(FPDFPage_GetCropBox(page, &cropbox_left, &cropbox_bottom,
                                     &cropbox_right, nullptr));
    EXPECT_FALSE(FPDFPage_GetCropBox(page, nullptr, nullptr, nullptr, nullptr));

    UnloadPage(page);
  }
}

TEST_F(FPDFTransformEmbedderTest, NoCropBox) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ASSERT_EQ(1, FPDF_GetPageCount(document()));

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  float left = -1.0f;
  float bottom = -2.0f;
  float right = 3.0f;
  float top = 0.0f;
  EXPECT_FALSE(FPDFPage_GetCropBox(page, &left, &bottom, &right, &top));
  EXPECT_EQ(-1.0f, left);
  EXPECT_EQ(-2.0f, bottom);
  EXPECT_EQ(3.0f, right);
  EXPECT_EQ(0.0f, top);

  UnloadPage(page);
}
