// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_folderfontinfo.h"

#include <utility>

#include "core/fxcrt/fx_codepage.h"
#include "core/fxge/fx_font.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

constexpr char kArial[] = "Arial";
constexpr char kCourierNew[] = "Courier New";
constexpr char kTimesNewRoman[] = "TimesNewRoman";
constexpr char kSymbol[] = "Symbol";
constexpr char kBookshelfSymbol7[] = "Bookshelf Symbol 7";
constexpr char kCalibri[] = "Calibri";
constexpr char kBookshelf[] = "Bookshelf";
constexpr char kBook[] = "Book";
constexpr char kTofuBold[] = "Tofu, Bold Italic";
constexpr char kTofu[] = "Tofu";
constexpr char kLatoUltraBold[] = "Lato Ultra-Bold";
constexpr char kLato[] = "Lato";
constexpr char kOxygenSansSansBold[] = "Oxygen-Sans Sans-Bold";
constexpr char kOxygenSans[] = "Oxygen-Sans";
constexpr char kOxygen[] = "Oxygen";
constexpr char kComicSansMS[] = "Comic Sans MS";

}  // namespace

class CFXFolderFontInfoTest : public ::testing::Test {
 public:
  CFXFolderFontInfoTest() {
    AddDummyFont(kArial, CHARSET_FLAG_ANSI);
    AddDummyFont(kCourierNew, CHARSET_FLAG_ANSI);
    AddDummyFont(kTimesNewRoman, 0);
    AddDummyFont(kBookshelfSymbol7, CHARSET_FLAG_SYMBOL);
    AddDummyFont(kSymbol, CHARSET_FLAG_SYMBOL);
    AddDummyFont(kTofuBold, CHARSET_FLAG_SYMBOL);
    AddDummyFont(kLatoUltraBold, CHARSET_FLAG_ANSI);
    AddDummyFont(kOxygenSansSansBold, CHARSET_FLAG_ANSI);
    AddDummyFont(kComicSansMS, CHARSET_FLAG_ANSI);
  }

  void* FindFont(int weight,
                 bool bItalic,
                 FX_Charset charset,
                 int pitch_family,
                 const char* family,
                 bool bMatchName) {
    return font_info_.FindFont(weight, bItalic, charset, pitch_family, family,
                               bMatchName);
  }

  ByteString GetFaceName(void* font) {
    return static_cast<CFX_FolderFontInfo::FontFaceInfo*>(font)->m_FaceName;
  }

 private:
  void AddDummyFont(const char* font_name, uint32_t charsets) {
    auto info = std::make_unique<CFX_FolderFontInfo::FontFaceInfo>(
        /*filePath=*/"", font_name, /*fontTables=*/"",
        /*fontOffset=*/0, /*fileSize=*/0);
    info->m_Charsets = charsets;
    font_info_.m_FontList[font_name] = std::move(info);
  }

  CFX_FolderFontInfo font_info_;
};

TEST_F(CFXFolderFontInfoTest, TestFindFont) {
  // Find "Symbol" font
  void* font = FindFont(/*weight=*/0, /*bItalic=*/false, FX_Charset::kSymbol,
                        FXFONT_FF_ROMAN, kSymbol, /*bMatchName=*/true);
  ASSERT_TRUE(font);
  EXPECT_EQ(GetFaceName(font), kSymbol);

  // Find "Calibri" font that is not present in the installed fonts
  EXPECT_FALSE(FindFont(/*weight=*/0, /*bItalic=*/false, FX_Charset::kSymbol,
                        FXFONT_FF_ROMAN, kCalibri, /*bMatchName=*/true));

  // Find the closest matching font to "Bookshelf" font that is present in the
  // installed fonts
  font = FindFont(/*weight=*/0, /*bItalic=*/false, FX_Charset::kSymbol,
                  FXFONT_FF_ROMAN, kBookshelf, /*bMatchName=*/true);
  ASSERT_TRUE(font);
  EXPECT_EQ(GetFaceName(font), kBookshelfSymbol7);

  // Find "Book" font is expected to fail, because none of the installed fonts
  // is in the same font family.
  EXPECT_FALSE(FindFont(/*weight=*/0, /*bItalic=*/false, FX_Charset::kSymbol,
                        FXFONT_FF_ROMAN, kBook, /*bMatchName=*/true));

  // Find the closest matching font for "Tofu" in the installed fonts, which
  // has "," following the string "Tofu".
  font = FindFont(/*weight=*/0, /*bItalic=*/false, FX_Charset::kSymbol,
                  FXFONT_FF_ROMAN, kTofu, /*bMatchName=*/true);
  ASSERT_TRUE(font);
  EXPECT_EQ(GetFaceName(font), kTofuBold);

  // Find the closest matching font for "Lato" in the installed fonts, which
  // has a space character following the string "Lato".
  font = FindFont(/*weight=*/0, /*bItalic=*/false, FX_Charset::kANSI,
                  FXFONT_FF_ROMAN, kLato, /*bMatchName=*/true);
  ASSERT_TRUE(font);
  EXPECT_EQ(GetFaceName(font), kLatoUltraBold);

  // Find the closest matching font for "Oxygen" in the installed fonts,
  // which has "-" following the string "Oxygen".
  font = FindFont(/*weight=*/0, /*bItalic=*/false, FX_Charset::kANSI,
                  FXFONT_FF_ROMAN, kOxygen, /*bMatchName=*/true);
  ASSERT_TRUE(font);
  EXPECT_EQ(GetFaceName(font), kOxygenSansSansBold);

  // Find the closest matching font for "Oxygen-Sans" in the installed fonts,
  // to test matching a family name with "-".
  font = FindFont(/*weight=*/0, /*bItalic=*/false, FX_Charset::kANSI,
                  FXFONT_FF_ROMAN, kOxygenSans, /*bMatchName=*/true);
  ASSERT_TRUE(font);
  EXPECT_EQ(GetFaceName(font), kOxygenSansSansBold);

  // Find "Symbol" font when name matching is false
  font = FindFont(/*weight=*/0, /*bItalic=*/false, FX_Charset::kSymbol,
                  FXFONT_FF_ROMAN, kSymbol, /*bMatchName=*/false);
  ASSERT_TRUE(font);
  EXPECT_EQ(GetFaceName(font), kBookshelfSymbol7);

  font = FindFont(700, false, FX_Charset::kANSI, FXFONT_FF_FIXEDPITCH,
                  kComicSansMS, true);
  ASSERT_TRUE(font);
  EXPECT_EQ(GetFaceName(font), kComicSansMS);
}
