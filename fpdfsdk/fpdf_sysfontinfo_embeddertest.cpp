// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_sysfontinfo.h"

#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

class FPDFSysFontInfoEmbedderTest : public EmbedderTest {
 public:
  FPDFSysFontInfoEmbedderTest() = default;
  ~FPDFSysFontInfoEmbedderTest() override = default;

  void SetUp() override {
    EmbedderTest::SetUp();
    font_info_ = FPDF_GetDefaultSystemFontInfo();
    ASSERT_TRUE(font_info_);
    FPDF_SetSystemFontInfo(font_info_);
  }

  void TearDown() override {
    EmbedderTest::TearDown();
    FPDF_FreeDefaultSystemFontInfo(font_info_);
  }

  FPDF_SYSFONTINFO* font_info_;
};

}  // namespace

TEST_F(FPDFSysFontInfoEmbedderTest, DefaultSystemFontInfo) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ASSERT_EQ(1, FPDF_GetPageCount(document()));

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Not checking the rendering because it will depend on the fonts installed.
    ScopedFPDFBitmap bitmap = RenderPage(page);
    ASSERT_EQ(200, FPDFBitmap_GetWidth(bitmap.get()));
    ASSERT_EQ(200, FPDFBitmap_GetHeight(bitmap.get()));
  }

  UnloadPage(page);
}
