// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "core/fxcrt/fx_system.h"
#include "public/fpdf_edit.h"
#include "public/fpdf_save.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class CPDFSecurityHandlerEmbeddertest : public EmbedderTest {};

TEST_F(CPDFSecurityHandlerEmbeddertest, Unencrypted) {
  ASSERT_TRUE(OpenDocument("about_blank.pdf"));
  EXPECT_EQ(0xFFFFFFFF, FPDF_GetDocPermissions(document()));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, UnencryptedWithPassword) {
  ASSERT_TRUE(OpenDocument("about_blank.pdf", "foobar"));
  EXPECT_EQ(0xFFFFFFFF, FPDF_GetDocPermissions(document()));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, NoPassword) {
  EXPECT_FALSE(OpenDocument("encrypted.pdf"));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, BadPassword) {
  EXPECT_FALSE(OpenDocument("encrypted.pdf", "tiger"));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, UserPassword) {
  ASSERT_TRUE(OpenDocument("encrypted.pdf", "1234"));
  EXPECT_EQ(0xFFFFF2C0, FPDF_GetDocPermissions(document()));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, OwnerPassword) {
  ASSERT_TRUE(OpenDocument("encrypted.pdf", "5678"));
  EXPECT_EQ(0xFFFFFFFC, FPDF_GetDocPermissions(document()));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, PasswordAfterGenerateSave) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_LINUX_
  const char md5[] = "e4a3701ca5b2a759e06455aa8d97d46e";
#elif _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  const char md5[] = "6951b6c9891dfe0332a5b1983e484400";
#else
  const char md5[] = "50985f3440d3f66c3b599ab138214015";
#endif  // _FXM_PLATFORM_ == _FXM_PLATFORM_LINUX_
  {
    ASSERT_TRUE(OpenDocument("encrypted.pdf", "5678", true));
    FPDF_PAGE page = LoadPage(0);
    ASSERT_TRUE(page);
    FPDF_PAGEOBJECT red_rect = FPDFPageObj_CreateNewRect(10, 10, 20, 20);
    ASSERT_TRUE(red_rect);
    EXPECT_TRUE(FPDFPath_SetFillColor(red_rect, 255, 0, 0, 255));
    EXPECT_TRUE(FPDFPath_SetDrawMode(red_rect, FPDF_FILLMODE_ALTERNATE, 0));
    FPDFPage_InsertObject(page, red_rect);
    FPDF_BITMAP page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap, 612, 792, md5);
    FPDFBitmap_Destroy(page_bitmap);
    EXPECT_TRUE(FPDFPage_GenerateContent(page));
    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
    UnloadPage(page);
  }
  std::string new_file = GetString();
  FPDF_FILEACCESS file_access;
  memset(&file_access, 0, sizeof(file_access));
  file_access.m_FileLen = new_file.size();
  file_access.m_GetBlock = GetBlockFromString;
  file_access.m_Param = &new_file;
  EXPECT_FALSE(FPDF_LoadCustomDocument(&file_access, nullptr));
  struct {
    const char* password;
    const unsigned long permissions;
  } tests[] = {{"1234", 0xFFFFF2C0}, {"5678", 0xFFFFFFFC}};
  for (const auto& test : tests) {
    TestSaved(612, 792, md5, test.password);
    EXPECT_EQ(test.permissions, FPDF_GetDocPermissions(m_SavedDocument));
    CloseSaved();
  }
}

TEST_F(CPDFSecurityHandlerEmbeddertest, NoPasswordVersion5) {
  ASSERT_FALSE(OpenDocument("bug_644.pdf"));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, BadPasswordVersion5) {
  ASSERT_FALSE(OpenDocument("bug_644.pdf", "tiger"));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, OwnerPasswordVersion5) {
  ASSERT_TRUE(OpenDocument("bug_644.pdf", "a"));
  EXPECT_EQ(0xFFFFFFFC, FPDF_GetDocPermissions(document()));
}

TEST_F(CPDFSecurityHandlerEmbeddertest, UserPasswordVersion5) {
  ASSERT_TRUE(OpenDocument("bug_644.pdf", "b"));
  EXPECT_EQ(0xFFFFFFFC, FPDF_GetDocPermissions(document()));
}
