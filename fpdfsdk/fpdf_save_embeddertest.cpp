// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>
#include <string>

#include "core/fxcrt/fx_string.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_edit.h"
#include "public/fpdf_ppo.h"
#include "public/fpdf_save.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_constants.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/gmock/include/gmock/gmock-matchers.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::HasSubstr;
using testing::Not;
using testing::StartsWith;

class FPDFSaveEmbedderTest : public EmbedderTest {};

TEST_F(FPDFSaveEmbedderTest, SaveSimpleDoc) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  EXPECT_THAT(GetString(), StartsWith("%PDF-1.7\r\n"));
  EXPECT_EQ(805u, GetString().size());
}

TEST_F(FPDFSaveEmbedderTest, SaveSimpleDocWithVersion) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_TRUE(FPDF_SaveWithVersion(document(), this, 0, 14));
  EXPECT_THAT(GetString(), StartsWith("%PDF-1.4\r\n"));
  EXPECT_EQ(805u, GetString().size());
}

TEST_F(FPDFSaveEmbedderTest, SaveSimpleDocWithBadVersion) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_TRUE(FPDF_SaveWithVersion(document(), this, 0, -1));
  EXPECT_THAT(GetString(), StartsWith("%PDF-1.7\r\n"));

  ClearString();
  EXPECT_TRUE(FPDF_SaveWithVersion(document(), this, 0, 0));
  EXPECT_THAT(GetString(), StartsWith("%PDF-1.7\r\n"));

  ClearString();
  EXPECT_TRUE(FPDF_SaveWithVersion(document(), this, 0, 18));
  EXPECT_THAT(GetString(), StartsWith("%PDF-1.7\r\n"));
}

TEST_F(FPDFSaveEmbedderTest, SaveSimpleDocIncremental) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_TRUE(FPDF_SaveWithVersion(document(), this, FPDF_INCREMENTAL, 14));
  // Version gets taken as-is from input document.
  EXPECT_THAT(GetString(), StartsWith("%PDF-1.7\n%\xa0\xf2\xa4\xf4"));
  // Additional output produced vs. non incremental.
  // Check that the size is larger than the old, broken incremental save size.
  EXPECT_GT(GetString().size(), 985u);
}

TEST_F(FPDFSaveEmbedderTest, SaveSimpleDocNoIncremental) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_TRUE(FPDF_SaveWithVersion(document(), this, FPDF_NO_INCREMENTAL, 14));
  EXPECT_THAT(GetString(), StartsWith("%PDF-1.4\r\n"));
  EXPECT_EQ(805u, GetString().size());
}

TEST_F(FPDFSaveEmbedderTest, SaveSimpleDocRemoveSecurity) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_TRUE(FPDF_SaveWithVersion(document(), this, FPDF_REMOVE_SECURITY, 14));
  EXPECT_THAT(GetString(), StartsWith("%PDF-1.4\r\n"));
  EXPECT_EQ(805u, GetString().size());
}

TEST_F(FPDFSaveEmbedderTest, SaveSimpleDocBadFlags) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_TRUE(FPDF_SaveWithVersion(document(), this, 999999, 14));
  EXPECT_THAT(GetString(), StartsWith("%PDF-1.4\r\n"));
  EXPECT_EQ(805u, GetString().size());
}

TEST_F(FPDFSaveEmbedderTest, SaveCopiedDoc) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));

  ScopedPage page = LoadScopedPage(0);
  EXPECT_TRUE(page);

  ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
  EXPECT_TRUE(output_doc);
  EXPECT_TRUE(FPDF_ImportPages(output_doc.get(), document(), "1", 0));
  EXPECT_TRUE(FPDF_SaveAsCopy(output_doc.get(), this, 0));
}

TEST_F(FPDFSaveEmbedderTest, Bug42271133) {
  ASSERT_TRUE(OpenDocument("bug_42271133.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Arbitrarily remove the first page object.
  auto text_object = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(text_object);
  ASSERT_TRUE(FPDFPage_RemoveObject(page.get(), text_object));
  FPDFPageObj_Destroy(text_object);

  // Regenerate dirty stream and save the document.
  ASSERT_TRUE(FPDFPage_GenerateContent(page.get()));
  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // Reload saved document.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  // Assert path fill color is not changed to black.
  auto path_obj = FPDFPage_GetObject(saved_page, 0);
  ASSERT_TRUE(path_obj);
  unsigned int r;
  unsigned int g;
  unsigned int b;
  unsigned int a;
  ASSERT_TRUE(FPDFPageObj_GetFillColor(path_obj, &r, &g, &b, &a));
  EXPECT_EQ(180u, r);
  EXPECT_EQ(180u, g);
  EXPECT_EQ(180u, b);

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFSaveEmbedderTest, SaveLinearizedDoc) {
  const int kPageCount = 3;
  std::array<std::string, kPageCount> original_md5;

  ASSERT_TRUE(OpenDocument("linearized.pdf"));
  for (int i = 0; i < kPageCount; ++i) {
    ScopedPage page = LoadScopedPage(i);
    ASSERT_TRUE(page);
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    EXPECT_EQ(612, FPDFBitmap_GetWidth(bitmap.get()));
    EXPECT_EQ(792, FPDFBitmap_GetHeight(bitmap.get()));
    original_md5[i] = HashBitmap(bitmap.get());
  }

  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  EXPECT_THAT(GetString(), StartsWith("%PDF-1.6\r\n"));
  EXPECT_THAT(GetString(), HasSubstr("/Root "));
  EXPECT_THAT(GetString(), HasSubstr("/Info "));
  EXPECT_THAT(GetString(), HasSubstr("/Size 37"));
  EXPECT_THAT(GetString(), HasSubstr("35 0 obj"));
  EXPECT_THAT(GetString(), HasSubstr("36 0 obj"));
  EXPECT_THAT(GetString(), Not(HasSubstr("37 0 obj")));
  EXPECT_THAT(GetString(), Not(HasSubstr("38 0 obj")));
  EXPECT_EQ(7986u, GetString().size());

  // Make sure new document renders the same as the old one.
  ASSERT_TRUE(OpenSavedDocument());
  for (int i = 0; i < kPageCount; ++i) {
    FPDF_PAGE page = LoadSavedPage(i);
    ASSERT_TRUE(page);
    ScopedFPDFBitmap bitmap = RenderSavedPage(page);
    EXPECT_EQ(original_md5[i], HashBitmap(bitmap.get()));
    CloseSavedPage(page);
  }
  CloseSavedDocument();
}

TEST_F(FPDFSaveEmbedderTest, Bug1409) {
  ASSERT_TRUE(OpenDocument("jpx_lzw.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  while (FPDFPage_CountObjects(page.get()) > 0) {
    ScopedFPDFPageObject object(FPDFPage_GetObject(page.get(), 0));
    ASSERT_TRUE(object);
    ASSERT_TRUE(FPDFPage_RemoveObject(page.get(), object.get()));
  }
  ASSERT_TRUE(FPDFPage_GenerateContent(page.get()));

  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // The new document should render as empty.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  ScopedFPDFBitmap bitmap = RenderSavedPage(saved_page);
  EXPECT_EQ(pdfium::kBlankPage612By792Checksum, HashBitmap(bitmap.get()));
  CloseSavedPage(saved_page);
  CloseSavedDocument();

  EXPECT_THAT(GetString(), StartsWith("%PDF-1.7\r\n"));
  EXPECT_THAT(GetString(), HasSubstr("/Root "));
  EXPECT_THAT(GetString(), Not(HasSubstr("/Image")));
  EXPECT_LT(GetString().size(), 600u);
}

#ifdef PDF_ENABLE_XFA
TEST_F(FPDFSaveEmbedderTest, SaveXFADoc) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  EXPECT_THAT(GetString(), StartsWith("%PDF-1.7\r\n"));
  ASSERT_TRUE(OpenSavedDocument());
  // TODO(tsepez): check for XFA forms in document
  CloseSavedDocument();
}
#endif  // PDF_ENABLE_XFA

TEST_F(FPDFSaveEmbedderTest, Bug342) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  EXPECT_THAT(GetString(), HasSubstr("0000000000 65535 f\r\n"));
  EXPECT_THAT(GetString(), Not(HasSubstr("0000000000 65536 f\r\n")));
}

TEST_F(FPDFSaveEmbedderTest, Bug905142) {
  ASSERT_TRUE(OpenDocument("bug_905142.pdf"));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  EXPECT_THAT(GetString(), HasSubstr("/Length 0"));
}

// Should not trigger a DCHECK() failure in CFX_FileBufferArchive.
// Fails because the PDF is malformed.
TEST_F(FPDFSaveEmbedderTest, Bug1328389) {
  ASSERT_TRUE(OpenDocument("bug_1328389.pdf"));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  EXPECT_THAT(GetString(), HasSubstr("/Foo/"));
}

TEST_F(FPDFSaveEmbedderTest, IncrementalSaveWithModifications) {
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));

  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Get the original bitmap for comparison
  ScopedFPDFBitmap original_bitmap = RenderLoadedPage(page.get());
  std::string original_md5 = HashBitmap(original_bitmap.get());

  // Count text objects on a page
  auto count_text_objects = [](FPDF_PAGE page) {
    int object_count = FPDFPage_CountObjects(page);
    int text_count = 0;
    for (int i = 0; i < object_count; ++i) {
      FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, i);
      if (FPDFPageObj_GetType(obj) == FPDF_PAGEOBJ_TEXT) {
        ++text_count;
      }
    }
    return text_count;
  };

  // Verify the original PDF does not have any text objects
  EXPECT_EQ(0, count_text_objects(page.get()));

  // Add a new text object to modify the page.
  ScopedFPDFPageObject text_object(FPDFPageObj_NewTextObj(
      document(), "Arial", 12.0f));
  ScopedFPDFWideString text = GetFPDFWideString(L"Test Incremental Save");
  FPDFText_SetText(text_object.get(), text.get());
  FPDFPageObj_Transform(text_object.get(), 1, 0, 0, 1, 100, 100);
  FPDFPage_InsertObject(page.get(), text_object.release());
  ASSERT_TRUE(FPDFPage_GenerateContent(page.get()));

  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, FPDF_INCREMENTAL));

  // Verify the saved document
  // Count occurrences of key markers
  auto count_occurrences = [](const std::string& str, const std::string& substr) {
    size_t count = 0;
    size_t pos = 0;
    while ((pos = str.find(substr, pos)) != std::string::npos) {
      ++count;
      pos += substr.size();
    }
    return count;
  };

  // Should contain incremental save markers (original + incremental)
  std::string saved_content = GetString();
  EXPECT_EQ(2u, count_occurrences(saved_content, "trailer"));
  // In incremental PDF saving, /Prev points to the previous xref table's offset.
  // Since we're doing only one incremental save operation, there's only one
  // /Prev entry pointing to the original PDF's xref table.
  EXPECT_EQ(1u, count_occurrences(saved_content, "/Prev"));
  EXPECT_EQ(2u, count_occurrences(saved_content, "startxref"));
  EXPECT_EQ(2u, count_occurrences(saved_content, "%%EOF"));

  // Load the saved document and verify the modification is visible
  ScopedSavedDoc saved_doc = OpenScopedSavedDocument();
  ASSERT_TRUE(saved_doc);
  ScopedSavedPage saved_page = LoadScopedSavedPage(0);
  ASSERT_TRUE(saved_page);

  // The rendered output should be different from the original
  ScopedFPDFBitmap saved_bitmap = RenderSavedPage(saved_page.get());
  std::string saved_md5 = HashBitmap(saved_bitmap.get());
  EXPECT_NE(original_md5, saved_md5);

  // Verify the text object exists after the save
  EXPECT_EQ(1, count_text_objects(saved_page.get()));
}
