// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>

#include "build/build_config.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fxcrt/fx_system.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_edit.h"
#include "public/fpdf_save.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

const char kAgeUTF8[] =
    "\xc3\xa2"
    "ge";
const char kAgeLatin1[] =
    "\xe2"
    "ge";

const char kHotelUTF8[] =
    "h"
    "\xc3\xb4"
    "tel";
const char kHotelLatin1[] =
    "h"
    "\xf4"
    "tel";

}  // namespace

class CPDFSecurityHandlerEmbedderTest : public EmbedderTest {
 protected:
  void OpenAndVerifyHelloWorldDocumentWithPassword(const char* filename,
                                                   const char* password) {
    ASSERT_TRUE(OpenDocumentWithPassword(filename, password));
    FPDF_PAGE page = LoadPage(0);
    VerifyHelloWorldPage(page);
    UnloadPage(page);
  }

  void VerifySavedHelloWorldDocumentWithPassword(const char* password) {
    ASSERT_TRUE(OpenSavedDocumentWithPassword(password));
    FPDF_PAGE page = LoadSavedPage(0);
    VerifyHelloWorldPage(page);
    CloseSavedPage(page);
    CloseSavedDocument();
  }

  void VerifySavedModifiedHelloWorldDocumentWithPassword(const char* password) {
    ASSERT_TRUE(OpenSavedDocumentWithPassword(password));
    FPDF_PAGE page = LoadSavedPage(0);
    VerifyModifiedHelloWorldPage(page);
    CloseSavedPage(page);
    CloseSavedDocument();
  }

  void RemoveTrailerIdFromDocument() {
    // This is cheating slightly to avoid a layering violation, since this file
    // cannot include fpdfsdk/cpdfsdk_helpers.h to get access to
    // CPDFDocumentFromFPDFDocument().
    CPDF_Document* doc = reinterpret_cast<CPDF_Document*>((document()));
    ASSERT_TRUE(doc);
    CPDF_Parser* parser = doc->GetParser();
    ASSERT_TRUE(parser);
    CPDF_Dictionary* trailer = parser->GetMutableTrailerForTesting();
    ASSERT_TRUE(trailer);
    ASSERT_TRUE(trailer->RemoveFor("ID"));
  }

  void RemoveGoodbyeObject() {
    FPDF_PAGE page = LoadPage(0);
    {
      ScopedFPDFPageObject goodbye_object(FPDFPage_GetObject(page, 1));
      ASSERT_TRUE(goodbye_object);
      ASSERT_TRUE(FPDFPage_RemoveObject(page, goodbye_object.get()));
    }
    ASSERT_TRUE(FPDFPage_GenerateContent(page));
    VerifyModifiedHelloWorldPage(page);
    UnloadPage(page);
  }

 private:
  void VerifyHelloWorldPage(FPDF_PAGE page) {
    ASSERT_TRUE(page);

#if defined(OS_WIN)
    const char kExpectedHash[] = "795b7ce1626931aa06af0fa23b7d80bb";
#elif defined(OS_MACOSX)
    const char kExpectedHash[] = "c38b75e16a13852aee3b97d77a0f0ee7";
#else
    const char kExpectedHash[] = "2baa4c0e1758deba1b9c908e1fbd04ed";
#endif

    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 200, kExpectedHash);
  }

  void VerifyModifiedHelloWorldPage(FPDF_PAGE page) {
    ASSERT_TRUE(page);

#if defined(OS_WIN)
    const char kExpectedHash[] = "93db13099042bafefb3c22a165bad684";
#elif defined(OS_MACOSX)
    const char kExpectedHash[] = "572b1022bb3e8f43dc671162fc62cf7f";
#else
    const char kExpectedHash[] = "93dcc09055f87a2792c8e3065af99a1b";
#endif

    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 200, kExpectedHash);
  }
};

TEST_F(CPDFSecurityHandlerEmbedderTest, Unencrypted) {
  ASSERT_TRUE(OpenDocument("about_blank.pdf"));
  EXPECT_EQ(0xFFFFFFFF, FPDF_GetDocPermissions(document()));
}

TEST_F(CPDFSecurityHandlerEmbedderTest, UnencryptedWithPassword) {
  ASSERT_TRUE(OpenDocumentWithPassword("about_blank.pdf", "foobar"));
  EXPECT_EQ(0xFFFFFFFF, FPDF_GetDocPermissions(document()));
}

TEST_F(CPDFSecurityHandlerEmbedderTest, NoPassword) {
  EXPECT_FALSE(OpenDocument("encrypted.pdf"));
}

TEST_F(CPDFSecurityHandlerEmbedderTest, BadPassword) {
  EXPECT_FALSE(OpenDocumentWithPassword("encrypted.pdf", "tiger"));
}

TEST_F(CPDFSecurityHandlerEmbedderTest, UserPassword) {
  ASSERT_TRUE(OpenDocumentWithPassword("encrypted.pdf", "1234"));
  EXPECT_EQ(0xFFFFF2C0, FPDF_GetDocPermissions(document()));
}

TEST_F(CPDFSecurityHandlerEmbedderTest, OwnerPassword) {
  ASSERT_TRUE(OpenDocumentWithPassword("encrypted.pdf", "5678"));
  EXPECT_EQ(0xFFFFFFFC, FPDF_GetDocPermissions(document()));
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_PasswordAfterGenerateSave DISABLED_PasswordAfterGenerateSave
#else
#define MAYBE_PasswordAfterGenerateSave PasswordAfterGenerateSave
#endif
TEST_F(CPDFSecurityHandlerEmbedderTest, MAYBE_PasswordAfterGenerateSave) {
#if _FX_PLATFORM_ == _FX_PLATFORM_LINUX_
  const char md5[] = "7048dca58e2ed8f93339008b91e4eb4e";
#elif defined(OS_MACOSX)
  const char md5[] = "1ace03eb7c466c132aacf319cb9d69d3";
#else
  const char md5[] = "041c2fb541c8907cc22ce101b686c79e";
#endif  // _FX_PLATFORM_ == _FX_PLATFORM_LINUX_
  {
    ASSERT_TRUE(OpenDocumentWithOptions("encrypted.pdf", "5678",
                                        LinearizeOption::kMustLinearize,
                                        JavaScriptOption::kEnableJavaScript));
    FPDF_PAGE page = LoadPage(0);
    ASSERT_TRUE(page);
    FPDF_PAGEOBJECT red_rect = FPDFPageObj_CreateNewRect(10, 10, 20, 20);
    ASSERT_TRUE(red_rect);
    EXPECT_TRUE(FPDFPageObj_SetFillColor(red_rect, 255, 0, 0, 255));
    EXPECT_TRUE(FPDFPath_SetDrawMode(red_rect, FPDF_FILLMODE_ALTERNATE, 0));
    FPDFPage_InsertObject(page, red_rect);
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page);
    CompareBitmap(bitmap.get(), 612, 792, md5);
    EXPECT_TRUE(FPDFPage_GenerateContent(page));
    SetWholeFileAvailable();
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
    ASSERT_TRUE(OpenSavedDocumentWithPassword(test.password));
    FPDF_PAGE page = LoadSavedPage(0);
    VerifySavedRendering(page, 612, 792, md5);
    EXPECT_EQ(test.permissions, FPDF_GetDocPermissions(saved_document_));

    CloseSavedPage(page);
    CloseSavedDocument();
  }
}

TEST_F(CPDFSecurityHandlerEmbedderTest, NoPasswordVersion5) {
  ASSERT_FALSE(OpenDocument("bug_644.pdf"));
}

TEST_F(CPDFSecurityHandlerEmbedderTest, BadPasswordVersion5) {
  ASSERT_FALSE(OpenDocumentWithPassword("bug_644.pdf", "tiger"));
}

TEST_F(CPDFSecurityHandlerEmbedderTest, OwnerPasswordVersion5) {
  ASSERT_TRUE(OpenDocumentWithPassword("bug_644.pdf", "a"));
  EXPECT_EQ(0xFFFFFFFC, FPDF_GetDocPermissions(document()));
}

TEST_F(CPDFSecurityHandlerEmbedderTest, UserPasswordVersion5) {
  ASSERT_TRUE(OpenDocumentWithPassword("bug_644.pdf", "b"));
  EXPECT_EQ(0xFFFFFFFC, FPDF_GetDocPermissions(document()));
}

// Should not crash. https://crbug.com/pdfium/1436
TEST_F(CPDFSecurityHandlerEmbedderTest, BadOkeyVersion2) {
  EXPECT_FALSE(
      OpenDocumentWithPassword("encrypted_hello_world_r2_bad_okey.pdf", "a"));
}

// Should not crash. https://crbug.com/pdfium/1436
TEST_F(CPDFSecurityHandlerEmbedderTest, BadOkeyVersion3) {
  EXPECT_FALSE(
      OpenDocumentWithPassword("encrypted_hello_world_r3_bad_okey.pdf", "a"));
}

TEST_F(CPDFSecurityHandlerEmbedderTest, OwnerPasswordVersion2UTF8) {
  // The password is "age", where the 'a' has a circumflex. Encoding the
  // password as UTF-8 works.
  OpenAndVerifyHelloWorldDocumentWithPassword("encrypted_hello_world_r2.pdf",
                                              kAgeUTF8);
  EXPECT_EQ(2, FPDF_GetSecurityHandlerRevision(document()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveTrailerIdFromDocument();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  // With revision 2 and 3, the owner password is not tied to the document ID in
  // the trailer, so the owner password entry remains in the copy and is still
  // valid, even though the document ID has changed.
  // The user password is tied to the document ID, so without an existing ID,
  // the user password entry has to be regenerated with the owner password.
  // Since the user password was not used to decrypt the document, it cannot be
  // recovered. Thus only verify the owner password, which is now also the user
  // password.
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);

  ClearString();
  RemoveGoodbyeObject();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeUTF8);
}

TEST_F(CPDFSecurityHandlerEmbedderTest, OwnerPasswordVersion2Latin1) {
  // The same password encoded as Latin-1 also works at revision 2.
  OpenAndVerifyHelloWorldDocumentWithPassword("encrypted_hello_world_r2.pdf",
                                              kAgeLatin1);
  EXPECT_EQ(2, FPDF_GetSecurityHandlerRevision(document()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveTrailerIdFromDocument();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);

  ClearString();
  RemoveGoodbyeObject();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeUTF8);
}

TEST_F(CPDFSecurityHandlerEmbedderTest, OwnerPasswordVersion3UTF8) {
  // Same as OwnerPasswordVersion2UTF8 test above.
  OpenAndVerifyHelloWorldDocumentWithPassword("encrypted_hello_world_r3.pdf",
                                              kAgeUTF8);
  EXPECT_EQ(3, FPDF_GetSecurityHandlerRevision(document()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveTrailerIdFromDocument();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);

  ClearString();
  RemoveGoodbyeObject();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeUTF8);
}

TEST_F(CPDFSecurityHandlerEmbedderTest, OwnerPasswordVersion3Latin1) {
  // Same as OwnerPasswordVersion2Latin1 test above.
  OpenAndVerifyHelloWorldDocumentWithPassword("encrypted_hello_world_r3.pdf",
                                              kAgeLatin1);
  EXPECT_EQ(3, FPDF_GetSecurityHandlerRevision(document()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveTrailerIdFromDocument();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);

  ClearString();
  RemoveGoodbyeObject();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeUTF8);
}

TEST_F(CPDFSecurityHandlerEmbedderTest, OwnerPasswordVersion5UTF8) {
  // Same as OwnerPasswordVersion2UTF8 test above.
  OpenAndVerifyHelloWorldDocumentWithPassword("encrypted_hello_world_r5.pdf",
                                              kAgeUTF8);
  EXPECT_EQ(5, FPDF_GetSecurityHandlerRevision(document()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveTrailerIdFromDocument();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveGoodbyeObject();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelUTF8);
}

TEST_F(CPDFSecurityHandlerEmbedderTest, OwnerPasswordVersion5Latin1) {
  // Same as OwnerPasswordVersion2Latin1 test above.
  OpenAndVerifyHelloWorldDocumentWithPassword("encrypted_hello_world_r5.pdf",
                                              kAgeLatin1);
  EXPECT_EQ(5, FPDF_GetSecurityHandlerRevision(document()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveTrailerIdFromDocument();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveGoodbyeObject();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelUTF8);
}

TEST_F(CPDFSecurityHandlerEmbedderTest, OwnerPasswordVersion6UTF8) {
  // Same as OwnerPasswordVersion2UTF8 test above.
  OpenAndVerifyHelloWorldDocumentWithPassword("encrypted_hello_world_r6.pdf",
                                              kAgeUTF8);
  EXPECT_EQ(6, FPDF_GetSecurityHandlerRevision(document()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveTrailerIdFromDocument();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveGoodbyeObject();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelUTF8);
}

TEST_F(CPDFSecurityHandlerEmbedderTest, OwnerPasswordVersion6Latin1) {
  // Same as OwnerPasswordVersion2Latin1 test above.
  OpenAndVerifyHelloWorldDocumentWithPassword("encrypted_hello_world_r6.pdf",
                                              kAgeLatin1);
  EXPECT_EQ(6, FPDF_GetSecurityHandlerRevision(document()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveTrailerIdFromDocument();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveGoodbyeObject();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelUTF8);
}

TEST_F(CPDFSecurityHandlerEmbedderTest, UserPasswordVersion2UTF8) {
  // The password is "hotel", where the 'o' has a circumflex. Encoding the
  // password as UTF-8 works.
  OpenAndVerifyHelloWorldDocumentWithPassword("encrypted_hello_world_r2.pdf",
                                              kHotelUTF8);
  EXPECT_EQ(2, FPDF_GetSecurityHandlerRevision(document()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveTrailerIdFromDocument();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  // Unlike the OwnerPasswordVersion2UTF8 test case, the user password was used
  // to decrypt the document, so it is available to regenerated the user
  // password entry. Thus it is possible to verify with both the unmodified
  // owner password, and the updated user password.
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveGoodbyeObject();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelUTF8);
}

TEST_F(CPDFSecurityHandlerEmbedderTest, UserPasswordVersion2Latin1) {
  // The same password encoded as Latin-1 also works at revision 2.
  OpenAndVerifyHelloWorldDocumentWithPassword("encrypted_hello_world_r2.pdf",
                                              kHotelLatin1);
  EXPECT_EQ(2, FPDF_GetSecurityHandlerRevision(document()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveTrailerIdFromDocument();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveGoodbyeObject();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelUTF8);
}

TEST_F(CPDFSecurityHandlerEmbedderTest, UserPasswordVersion3UTF8) {
  // Same as UserPasswordVersion2UTF8 test above.
  OpenAndVerifyHelloWorldDocumentWithPassword("encrypted_hello_world_r3.pdf",
                                              kHotelUTF8);
  EXPECT_EQ(3, FPDF_GetSecurityHandlerRevision(document()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveTrailerIdFromDocument();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveGoodbyeObject();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelUTF8);
}

TEST_F(CPDFSecurityHandlerEmbedderTest, UserPasswordVersion3Latin1) {
  // Same as UserPasswordVersion2Latin1 test above.
  OpenAndVerifyHelloWorldDocumentWithPassword("encrypted_hello_world_r3.pdf",
                                              kHotelLatin1);
  EXPECT_EQ(3, FPDF_GetSecurityHandlerRevision(document()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveTrailerIdFromDocument();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveGoodbyeObject();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelUTF8);
}

TEST_F(CPDFSecurityHandlerEmbedderTest, UserPasswordVersion5UTF8) {
  // Same as UserPasswordVersion2UTF8 test above.
  OpenAndVerifyHelloWorldDocumentWithPassword("encrypted_hello_world_r5.pdf",
                                              kHotelUTF8);
  EXPECT_EQ(5, FPDF_GetSecurityHandlerRevision(document()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveTrailerIdFromDocument();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveGoodbyeObject();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelUTF8);
}

TEST_F(CPDFSecurityHandlerEmbedderTest, UserPasswordVersion5Latin1) {
  // Same as UserPasswordVersion2Latin1 test above.
  OpenAndVerifyHelloWorldDocumentWithPassword("encrypted_hello_world_r5.pdf",
                                              kHotelLatin1);
  EXPECT_EQ(5, FPDF_GetSecurityHandlerRevision(document()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveTrailerIdFromDocument();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveGoodbyeObject();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelUTF8);
}

TEST_F(CPDFSecurityHandlerEmbedderTest, UserPasswordVersion6UTF8) {
  // Same as UserPasswordVersion2UTF8 test above.
  OpenAndVerifyHelloWorldDocumentWithPassword("encrypted_hello_world_r6.pdf",
                                              kHotelUTF8);
  EXPECT_EQ(6, FPDF_GetSecurityHandlerRevision(document()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveTrailerIdFromDocument();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveGoodbyeObject();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelUTF8);
}

TEST_F(CPDFSecurityHandlerEmbedderTest, UserPasswordVersion6Latin1) {
  // Same as UserPasswordVersion2Latin1 test above.
  OpenAndVerifyHelloWorldDocumentWithPassword("encrypted_hello_world_r6.pdf",
                                              kHotelLatin1);
  EXPECT_EQ(6, FPDF_GetSecurityHandlerRevision(document()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveTrailerIdFromDocument();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedHelloWorldDocumentWithPassword(kHotelUTF8);

  ClearString();
  RemoveGoodbyeObject();
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kAgeUTF8);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelLatin1);
  VerifySavedModifiedHelloWorldDocumentWithPassword(kHotelUTF8);
}
