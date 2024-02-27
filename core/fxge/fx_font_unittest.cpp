// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "core/fxcrt/check.h"
#include "core/fxge/cfx_folderfontinfo.h"
#include "core/fxge/cfx_fontmapper.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/fx_font.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/utils/path_service.h"

TEST(FXFontTest, UnicodeFromAdobeName) {
  EXPECT_EQ(static_cast<wchar_t>(0x0000), UnicodeFromAdobeName("nonesuch"));
  EXPECT_EQ(static_cast<wchar_t>(0x0000), UnicodeFromAdobeName(""));
  EXPECT_EQ(static_cast<wchar_t>(0x00b6), UnicodeFromAdobeName("paragraph"));
  EXPECT_EQ(static_cast<wchar_t>(0x00d3), UnicodeFromAdobeName("Oacute"));
  EXPECT_EQ(static_cast<wchar_t>(0x00fe), UnicodeFromAdobeName("thorn"));
  EXPECT_EQ(static_cast<wchar_t>(0x0384), UnicodeFromAdobeName("tonos"));
  EXPECT_EQ(static_cast<wchar_t>(0x2022), UnicodeFromAdobeName("bullet"));
}

TEST(FXFontTest, AdobeNameFromUnicode) {
  EXPECT_EQ("", AdobeNameFromUnicode(0x0000));
  EXPECT_EQ("divide", AdobeNameFromUnicode(0x00f7));
  EXPECT_EQ("Lslash", AdobeNameFromUnicode(0x0141));
  EXPECT_EQ("tonos", AdobeNameFromUnicode(0x0384));
  EXPECT_EQ("afii57513", AdobeNameFromUnicode(0x0691));
  EXPECT_EQ("angkhankhuthai", AdobeNameFromUnicode(0x0e5a));
  EXPECT_EQ("Euro", AdobeNameFromUnicode(0x20ac));
}

TEST(FXFontTest, ReadFontNameFromMicrosoftEntries) {
  std::string test_data_dir;
  PathService::GetTestDataDir(&test_data_dir);
  DCHECK(!test_data_dir.empty());

  CFX_FontMapper font_mapper(nullptr);

  {
    // |folder_font_info| has to be deallocated before the |font_mapper| or we
    // run into UnownedPtr class issues with ASAN.
    CFX_FolderFontInfo folder_font_info;
    folder_font_info.AddPath(
        (test_data_dir + PATH_SEPARATOR + "font_tests").c_str());

    font_mapper.SetSystemFontInfo(
        CFX_GEModule::Get()->GetPlatform()->CreateDefaultSystemFontInfo());
    ASSERT_TRUE(folder_font_info.EnumFontList(&font_mapper));
  }

  ASSERT_EQ(1u, font_mapper.GetFaceSize());
  ASSERT_EQ("Test", font_mapper.GetFaceName(0));
}
