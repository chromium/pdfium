// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "public/fpdf_edit.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_constants.h"
#include "testing/gtest/include/gtest/gtest.h"

using pdfium::RectanglesChecksum;

using FPDFEditPathEmbedderTest = EmbedderTest;

namespace {

constexpr int kExpectedRectangleWidth = 200;
constexpr int kExpectedRectangleHeight = 300;

const char* RectanglesAndTriangleChecksum() {
  return CFX_DefaultRenderDevice::UseSkiaRenderer()
             ? "89b85ca2749a98320518531cf365b010"
             : "8bb78ca28f1e0ab9d36c0745ae0f58bb";
}

ScopedFPDFPageObject CreateBlackTriangle() {
  ScopedFPDFPageObject path(FPDFPageObj_CreateNewPath(100, 50));
  EXPECT_TRUE(FPDFPageObj_SetFillColor(path.get(), 0, 0, 0, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(path.get(), FPDF_FILLMODE_ALTERNATE, 0));
  EXPECT_TRUE(FPDFPath_LineTo(path.get(), 100, 75));
  EXPECT_TRUE(FPDFPath_LineTo(path.get(), 75, 75));
  EXPECT_TRUE(FPDFPath_Close(path.get()));
  return path;
}

}  // namespace

TEST_F(FPDFEditPathEmbedderTest, VerifyCorrectColoursReturned) {
  constexpr int kObjectCount = 256;
  CreateEmptyDocument();
  FPDF_PAGE page = FPDFPage_New(document(), 0, 612, 792);

  for (size_t i = 0; i < kObjectCount; ++i) {
    FPDF_PAGEOBJECT path = FPDFPageObj_CreateNewPath(400, 100);
    EXPECT_TRUE(FPDFPageObj_SetFillColor(path, i, i, i, i));
    EXPECT_TRUE(FPDFPageObj_SetStrokeColor(path, i, i, i, i));
    EXPECT_TRUE(FPDFPath_SetDrawMode(path, FPDF_FILLMODE_ALTERNATE, 0));
    EXPECT_TRUE(FPDFPath_LineTo(path, 400, 200));
    EXPECT_TRUE(FPDFPath_LineTo(path, 300, 100));
    EXPECT_TRUE(FPDFPath_Close(path));

    FPDFPage_InsertObject(page, path);
  }

  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDF_ClosePage(page);
  page = nullptr;

  ASSERT_TRUE(OpenSavedDocument());
  page = LoadSavedPage(0);
  ASSERT_TRUE(page);

  for (size_t i = 0; i < kObjectCount; ++i) {
    FPDF_PAGEOBJECT path = FPDFPage_GetObject(page, i);
    ASSERT_TRUE(path);

    EXPECT_EQ(FPDF_PAGEOBJ_PATH, FPDFPageObj_GetType(path));

    unsigned int r;
    unsigned int g;
    unsigned int b;
    unsigned int a;
    ASSERT_TRUE(FPDFPageObj_GetFillColor(path, &r, &g, &b, &a));
    EXPECT_EQ(i, r);
    EXPECT_EQ(i, g);
    EXPECT_EQ(i, b);
    EXPECT_EQ(i, a);

    ASSERT_TRUE(FPDFPageObj_GetStrokeColor(path, &r, &g, &b, &a));
    EXPECT_EQ(i, r);
    EXPECT_EQ(i, g);
    EXPECT_EQ(i, b);
    EXPECT_EQ(i, a);
  }

  CloseSavedPage(page);
  CloseSavedDocument();
}

TEST_F(FPDFEditPathEmbedderTest, GetAndSetMatrixForPath) {
  OpenDocument("rectangles_double_flipped.pdf");
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedRectangleWidth,
                  kExpectedRectangleHeight, RectanglesChecksum());
  }

  FPDF_PAGEOBJECT path = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(path);
  ASSERT_EQ(FPDF_PAGEOBJ_PATH, FPDFPageObj_GetType(path));

  FS_MATRIX matrix;
  ASSERT_TRUE(FPDFPageObj_GetMatrix(path, &matrix));
  EXPECT_FLOAT_EQ(1.0f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(-1.0f, matrix.d);
  EXPECT_FLOAT_EQ(0.0f, matrix.e);
  EXPECT_FLOAT_EQ(300.0f, matrix.f);

  ASSERT_TRUE(FPDFPageObj_SetMatrix(path, &matrix));
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedRectangleWidth,
                  kExpectedRectangleHeight, RectanglesChecksum());
  }

  ASSERT_TRUE(FPDFPage_GenerateContent(page.get()));
  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedRectangleWidth,
                  kExpectedRectangleHeight, RectanglesChecksum());
  }


  VerifySavedDocument(kExpectedRectangleWidth, kExpectedRectangleHeight,
                      RectanglesChecksum());
}

TEST_F(FPDFEditPathEmbedderTest, GetAndSetMatrixForFormWithPath) {
  OpenDocument("form_object_with_path.pdf");
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedRectangleWidth,
                  kExpectedRectangleHeight, RectanglesChecksum());
  }

  FPDF_PAGEOBJECT form = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(form);
  ASSERT_EQ(FPDF_PAGEOBJ_FORM, FPDFPageObj_GetType(form));

  FS_MATRIX matrix;
  ASSERT_TRUE(FPDFPageObj_GetMatrix(form, &matrix));
  EXPECT_FLOAT_EQ(2.0f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(-1.0f, matrix.d);
  EXPECT_FLOAT_EQ(0.0f, matrix.e);
  EXPECT_FLOAT_EQ(300.0f, matrix.f);

  ASSERT_TRUE(FPDFPageObj_SetMatrix(form, &matrix));
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedRectangleWidth,
                  kExpectedRectangleHeight, RectanglesChecksum());
  }

  FPDF_PAGEOBJECT path = FPDFFormObj_GetObject(form, 0);
  ASSERT_TRUE(path);
  ASSERT_EQ(FPDF_PAGEOBJ_PATH, FPDFPageObj_GetType(path));

  ASSERT_TRUE(FPDFPageObj_GetMatrix(path, &matrix));
  EXPECT_FLOAT_EQ(0.5f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(-1.0f, matrix.d);
  EXPECT_FLOAT_EQ(0.0f, matrix.e);
  EXPECT_FLOAT_EQ(300.0f, matrix.f);

  ASSERT_TRUE(FPDFPageObj_SetMatrix(path, &matrix));
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedRectangleWidth,
                  kExpectedRectangleHeight, RectanglesChecksum());
  }

  ASSERT_TRUE(FPDFPage_GenerateContent(page.get()));
  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedRectangleWidth,
                  kExpectedRectangleHeight, RectanglesChecksum());
  }


  VerifySavedDocument(kExpectedRectangleWidth, kExpectedRectangleHeight,
                      RectanglesChecksum());
}

TEST_F(FPDFEditPathEmbedderTest, AddPathToRectangles) {
  OpenDocument("rectangles.pdf");
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedRectangleWidth,
                  kExpectedRectangleHeight, RectanglesChecksum());
  }

  ScopedFPDFPageObject path = CreateBlackTriangle();
  ASSERT_TRUE(path);
  FPDFPage_InsertObject(page.get(), path.release());

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedRectangleWidth,
                  kExpectedRectangleHeight, RectanglesAndTriangleChecksum());
  }

  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedRectangleWidth,
                  kExpectedRectangleHeight, RectanglesAndTriangleChecksum());
  }


  VerifySavedDocument(kExpectedRectangleWidth, kExpectedRectangleHeight,
                      RectanglesAndTriangleChecksum());
}

TEST_F(FPDFEditPathEmbedderTest, AddPathToRectanglesWithLeakyCTM) {
  OpenDocument("rectangles_with_leaky_ctm.pdf");
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedRectangleWidth,
                  kExpectedRectangleHeight, RectanglesChecksum());
  }

  ScopedFPDFPageObject path = CreateBlackTriangle();
  ASSERT_TRUE(path);
  FPDFPage_InsertObject(page.get(), path.release());

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedRectangleWidth,
                  kExpectedRectangleHeight, RectanglesAndTriangleChecksum());
  }

  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedRectangleWidth,
                  kExpectedRectangleHeight, RectanglesAndTriangleChecksum());
  }


  VerifySavedDocument(kExpectedRectangleWidth, kExpectedRectangleHeight,
                      RectanglesAndTriangleChecksum());
}
