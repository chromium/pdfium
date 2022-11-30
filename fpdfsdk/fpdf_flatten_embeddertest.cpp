// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "build/build_config.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "public/fpdf_flatten.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

class FPDFFlattenEmbedderTest : public EmbedderTest {};

}  // namespace

TEST_F(FPDFFlattenEmbedderTest, FlatNothing) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);
  EXPECT_EQ(FLATTEN_NOTHINGTODO, FPDFPage_Flatten(page, FLAT_NORMALDISPLAY));
  UnloadPage(page);
}

TEST_F(FPDFFlattenEmbedderTest, FlatNormal) {
  ASSERT_TRUE(OpenDocument("annotiter.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);
  EXPECT_EQ(FLATTEN_SUCCESS, FPDFPage_Flatten(page, FLAT_NORMALDISPLAY));
  UnloadPage(page);
}

TEST_F(FPDFFlattenEmbedderTest, FlatPrint) {
  ASSERT_TRUE(OpenDocument("annotiter.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);
  EXPECT_EQ(FLATTEN_SUCCESS, FPDFPage_Flatten(page, FLAT_PRINT));
  UnloadPage(page);
}

TEST_F(FPDFFlattenEmbedderTest, BUG_861842) {
  const char* checkbox_checksum = []() {
    if (CFX_DefaultRenderDevice::SkiaIsDefaultRenderer())
      return "252fd5f2299cc16e5a07565df7c30565";
#if BUILDFLAG(IS_APPLE)
    return "6aafcb2d98da222964bcdbf5aa1f4f1f";
#else
    return "594265790b81df2d93120d33b72a6ada";
#endif
  }();

  ASSERT_TRUE(OpenDocument("bug_861842.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
  CompareBitmap(bitmap.get(), 100, 120, checkbox_checksum);

  EXPECT_EQ(FLATTEN_SUCCESS, FPDFPage_Flatten(page, FLAT_PRINT));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  UnloadPage(page);

  // TODO(crbug.com/861842): This should not render blank.
  static constexpr char kBlankPageHash[] = "48400809c3862dae64b0cd00d51057a4";
  VerifySavedDocument(100, 120, kBlankPageHash);
}

TEST_F(FPDFFlattenEmbedderTest, BUG_889099) {
  const char* page_checksum = []() {
    if (CFX_DefaultRenderDevice::SkiaIsDefaultRenderer())
      return "73678f308625e22f31940e9f732b68bf";
#if BUILDFLAG(IS_APPLE)
    return "049ed3f1e21fc72f929af3410c64bc8f";
#else
    return "3db87245e3f4e37f4cb18654bbe22d97";
#endif
  }();
  const char* flattened_page_checksum = []() {
#if BUILDFLAG(IS_APPLE)
    if (!CFX_DefaultRenderDevice::SkiaIsDefaultRenderer())
      return "41debc60cf2a8f74c710ec6082d77b18";
#endif
    return "0832157462ea70fbbf053e14b1d6457f";
  }();

  ASSERT_TRUE(OpenDocument("bug_889099.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // The original document has a malformed media box; the height is -400.
  ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
  CompareBitmap(bitmap.get(), 300, 400, page_checksum);

  EXPECT_EQ(FLATTEN_SUCCESS, FPDFPage_Flatten(page, FLAT_PRINT));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  UnloadPage(page);

  VerifySavedDocument(300, 400, flattened_page_checksum);
}

TEST_F(FPDFFlattenEmbedderTest, BUG_890322) {
  const char* checksum = []() {
    if (CFX_DefaultRenderDevice::SkiaIsDefaultRenderer())
      return "793689536cf64fe792c2f241888c0cf3";
    return "6c674642154408e877d88c6c082d67e9";
  }();
  ASSERT_TRUE(OpenDocument("bug_890322.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
  CompareBitmap(bitmap.get(), 200, 200, checksum);

  EXPECT_EQ(FLATTEN_SUCCESS, FPDFPage_Flatten(page, FLAT_PRINT));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  UnloadPage(page);

  VerifySavedDocument(200, 200, checksum);
}

TEST_F(FPDFFlattenEmbedderTest, BUG_896366) {
  const char* checksum = []() {
    if (CFX_DefaultRenderDevice::SkiaIsDefaultRenderer())
      return "c3cccfadc4c5249e6aa0675e511fa4c3";
    return "f71ab085c52c8445ae785eca3ec858b1";
  }();
  ASSERT_TRUE(OpenDocument("bug_896366.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
  CompareBitmap(bitmap.get(), 612, 792, checksum);

  EXPECT_EQ(FLATTEN_SUCCESS, FPDFPage_Flatten(page, FLAT_PRINT));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  UnloadPage(page);

  VerifySavedDocument(612, 792, checksum);
}
