// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_folderfontinfo.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {
constexpr char kArial[] = "Arial";
constexpr char kTimesNewRoman[] = "TimesNewRoman";
constexpr char kSymbol[] = "Symbol";
constexpr char kBookshelfSymbol7[] = "Bookshelf Symbol 7";
constexpr char kCalibri[] = "Calibri";
constexpr char kBookshelf[] = "Bookshelf";
}  // namespace

class CFX_FolderFontInfoTest : public ::testing::Test {
 public:
  CFX_FolderFontInfoTest() {
    auto arial_info = pdfium::MakeUnique<CFX_FolderFontInfo::FontFaceInfo>(
        /*filePath=*/"", kArial, /*fontTables=*/"",
        /*fontOffset=*/0, /*fileSize=*/0);
    arial_info->m_Charsets = 2;
    auto times_new_roman_info =
        pdfium::MakeUnique<CFX_FolderFontInfo::FontFaceInfo>(
            /*filePath=*/"", kTimesNewRoman, /*fontTables=*/"",
            /*fontOffset=*/0, /*fileSize=*/0);
    auto bookshelf_symbol7_info =
        pdfium::MakeUnique<CFX_FolderFontInfo::FontFaceInfo>(
            /*filePath=*/"", kBookshelfSymbol7, /*fontTables=*/"",
            /*fontOffset=*/0, /*fileSize=*/0);
    bookshelf_symbol7_info->m_Charsets = 2;
    auto symbol_info = pdfium::MakeUnique<CFX_FolderFontInfo::FontFaceInfo>(
        /*filePath=*/"", kSymbol, /*fontTables=*/"",
        /*fontOffset=*/0, /*fileSize=*/0);
    symbol_info->m_Charsets = 2;

    font_info_.m_FontList[kArial] = std::move(arial_info);
    font_info_.m_FontList[kTimesNewRoman] = std::move(times_new_roman_info);
    font_info_.m_FontList[kBookshelfSymbol7] =
        std::move(bookshelf_symbol7_info);
    font_info_.m_FontList[kSymbol] = std::move(symbol_info);
  }

  void* FindFont(int weight,
                 bool bItalic,
                 int charset,
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
  CFX_FolderFontInfo font_info_;
};

TEST_F(CFX_FolderFontInfoTest, TestFindFont) {
  // Find "Symbol" font
  void* font = FindFont(/*weight=*/0, /*bItalic=*/false, /*charset=*/2,
                        /*pitch_family=*/2, kSymbol, /*bMatchName=*/true);
  ASSERT_TRUE(font);
  EXPECT_EQ(GetFaceName(font), kSymbol);

  // Find "Calibri" font that is not present in the installed fonts
  EXPECT_FALSE(FindFont(/*weight=*/0, /*bItalic=*/false, /*charset=*/2,
                        /*pitch_family=*/2, kCalibri,
                        /*bMatchName=*/true));

  // Find the closest matching font to "Bookself" font that is present in the
  // installed fonts
  font = FindFont(/*weight=*/0, /*bItalic=*/false, /*charset=*/2,
                  /*pitch_family=*/2, kBookshelf, /*bMatchName=*/true);
  ASSERT_TRUE(font);
  EXPECT_EQ(GetFaceName(font), kBookshelfSymbol7);

  // Find "Symbol" font when name matching is false
  font = FindFont(/*weight=*/0, /*bItalic=*/false, /*charset=*/2,
                  /*pitch_family=*/2, kSymbol, /*bMatchName=*/false);
  ASSERT_TRUE(font);
  EXPECT_EQ(GetFaceName(font), kArial);
}
