// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_sysfontinfo.h"

#include <string>
#include <vector>

#include "build/build_config.h"
#include "core/fxcrt/compiler_specific.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_environment.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

extern "C" {

void FakeRelease(FPDF_SYSFONTINFO* pThis) {}
void FakeEnumFonts(FPDF_SYSFONTINFO* pThis, void* pMapper) {}

void* FakeMapFont(FPDF_SYSFONTINFO* pThis,
                  int weight,
                  FPDF_BOOL bItalic,
                  int charset,
                  int pitch_family,
                  const char* face,
                  FPDF_BOOL* bExact) {
  // Any non-null return will do.
  return pThis;
}

void* FakeGetFont(FPDF_SYSFONTINFO* pThis, const char* face) {
  // Any non-null return will do.
  return pThis;
}

unsigned long FakeGetFontData(FPDF_SYSFONTINFO* pThis,
                              void* hFont,
                              unsigned int table,
                              unsigned char* buffer,
                              unsigned long buf_size) {
  return 0;
}

unsigned long FakeGetFaceName(FPDF_SYSFONTINFO* pThis,
                              void* hFont,
                              char* buffer,
                              unsigned long buf_size) {
  return 0;
}

int FakeGetFontCharset(FPDF_SYSFONTINFO* pThis, void* hFont) {
  return 1;
}

void FakeDeleteFont(FPDF_SYSFONTINFO* pThis, void* hFont) {}

}  // extern "C"

class FPDFUnavailableSysFontInfoEmbedderTest : public EmbedderTest {
 public:
  FPDFUnavailableSysFontInfoEmbedderTest() = default;
  ~FPDFUnavailableSysFontInfoEmbedderTest() override = default;

  void SetUp() override {
    EmbedderTest::SetUp();
    font_info_.version = 1;
    font_info_.Release = FakeRelease;
    font_info_.EnumFonts = FakeEnumFonts;
    font_info_.MapFont = FakeMapFont;
    font_info_.GetFont = FakeGetFont;
    font_info_.GetFontData = FakeGetFontData;
    font_info_.GetFaceName = FakeGetFaceName;
    font_info_.GetFontCharset = FakeGetFontCharset;
    font_info_.DeleteFont = FakeDeleteFont;
    FPDF_SetSystemFontInfo(&font_info_);
  }

  void TearDown() override {
    FPDF_SetSystemFontInfo(nullptr);
    EmbedderTest::TearDown();

    // Bouncing the library is the only reliable way to fully undo the initial
    // FPDF_SetSystemFontInfo() call at the moment.
    EmbedderTestEnvironment::GetInstance()->TearDown();
    EmbedderTestEnvironment::GetInstance()->SetUp();
  }

  FPDF_SYSFONTINFO font_info_;
};

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

    // After releasing `font_info_` from PDFium, it is safe to free it.
    FPDF_SetSystemFontInfo(nullptr);
    FPDF_FreeDefaultSystemFontInfo(font_info_);

    // Bouncing the library is the only reliable way to fully undo the initial
    // FPDF_SetSystemFontInfo() call at the moment.
    EmbedderTestEnvironment::GetInstance()->TearDown();

    EmbedderTestEnvironment::GetInstance()->SetUp();
  }

  FPDF_SYSFONTINFO* font_info_;
};

}  // namespace

TEST_F(FPDFUnavailableSysFontInfoEmbedderTest, Bug972518) {
  ASSERT_TRUE(OpenDocument("bug_972518.pdf"));
  ASSERT_EQ(1, FPDF_GetPageCount(document()));

  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
}

TEST_F(FPDFSysFontInfoEmbedderTest, DefaultSystemFontInfo) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ASSERT_EQ(1, FPDF_GetPageCount(document()));

  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // Not checking the rendering because it will depend on the fonts installed.
    ScopedFPDFBitmap bitmap = RenderPage(page.get());
    ASSERT_EQ(200, FPDFBitmap_GetWidth(bitmap.get()));
    ASSERT_EQ(200, FPDFBitmap_GetHeight(bitmap.get()));
  }
}

TEST_F(FPDFSysFontInfoEmbedderTest, DefaultTTFMap) {
  static constexpr int kExpectedCharsets[] = {
      FXFONT_ANSI_CHARSET,        FXFONT_SHIFTJIS_CHARSET,
      FXFONT_HANGEUL_CHARSET,     FXFONT_GB2312_CHARSET,
      FXFONT_CHINESEBIG5_CHARSET, FXFONT_ARABIC_CHARSET,
      FXFONT_CYRILLIC_CHARSET,    FXFONT_EASTERNEUROPEAN_CHARSET,
  };
  std::vector<int> charsets;

  const FPDF_CharsetFontMap* cfmap = FPDF_GetDefaultTTFMap();
  ASSERT_TRUE(cfmap);

  // Stop at either end mark.
  while (cfmap->charset != -1 && cfmap->fontname) {
    charsets.push_back(cfmap->charset);
    // SAFETY: requires FPDF_GetDefaultTTFMap() to provide a sentinel.
    UNSAFE_BUFFERS(++cfmap);
  }

  // Confirm end marks only occur as a pair.
  EXPECT_EQ(cfmap->charset, -1);
  EXPECT_EQ(cfmap->fontname, nullptr);

  EXPECT_THAT(charsets, testing::UnorderedElementsAreArray(kExpectedCharsets));
}

TEST_F(FPDFSysFontInfoEmbedderTest, DefaultTTFMapCountAndEntries) {
  static constexpr int kExpectedCharsets[] = {
      FXFONT_ANSI_CHARSET,
      FXFONT_GB2312_CHARSET,
      FXFONT_CHINESEBIG5_CHARSET,
      FXFONT_SHIFTJIS_CHARSET,
      FXFONT_HANGEUL_CHARSET,
      FXFONT_CYRILLIC_CHARSET,
      FXFONT_EASTERNEUROPEAN_CHARSET,
      FXFONT_ARABIC_CHARSET,
  };
  static const std::string kExpectedFontNames[] = {
      "Helvetica", "SimSun", "MingLiU", "MS Gothic", "Batang", "Arial",
#if BUILDFLAG(IS_WIN)
      "Tahoma",
#else
      "Arial",
#endif
      "Arial",
  };
  std::vector<int> charsets;
  std::vector<const char*> font_names;

  const size_t count = FPDF_GetDefaultTTFMapCount();
  for (size_t i = 0; i < count; ++i) {
    const FPDF_CharsetFontMap* entry = FPDF_GetDefaultTTFMapEntry(i);
    ASSERT_TRUE(entry);
    charsets.push_back(entry->charset);
    font_names.push_back(entry->fontname);
  }

  EXPECT_THAT(charsets, testing::ElementsAreArray(kExpectedCharsets));
  EXPECT_THAT(font_names, testing::ElementsAreArray(kExpectedFontNames));

  // Test out of bound indices.
  EXPECT_FALSE(FPDF_GetDefaultTTFMapEntry(count));
  EXPECT_FALSE(FPDF_GetDefaultTTFMapEntry(9999));
}
