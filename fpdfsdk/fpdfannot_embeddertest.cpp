// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>
#include <vector>

#include "public/fpdf_annot.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFAnnotEmbeddertest : public EmbedderTest, public TestSaver {};

TEST_F(FPDFAnnotEmbeddertest, ExtractHighlightLongContent) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_long_content.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);

  // Check that there is a total of 1 annotation on its first page.
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));

  // Check that the annotation is of type "highlight".
  FPDF_ANNOTATION annot;
  ASSERT_TRUE(FPDFPage_GetAnnot(page, 0, &annot));
  EXPECT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnot_GetSubtype(annot));

  // Check that the annotation color is yellow.
  unsigned int R;
  unsigned int G;
  unsigned int B;
  unsigned int A;
  EXPECT_TRUE(
      FPDFAnnot_GetColor(annot, FPDFANNOT_COLORTYPE_Color, &R, &G, &B, &A));
  EXPECT_EQ(255u, R);
  EXPECT_EQ(255u, G);
  EXPECT_EQ(0u, B);
  EXPECT_EQ(255u, A);

  // Check that the author is correct.
  unsigned long len =
      FPDFAnnot_GetText(annot, FPDFANNOT_TEXTTYPE_Author, nullptr, 0);
  std::vector<char> buf(len);
  EXPECT_EQ(28u, FPDFAnnot_GetText(annot, FPDFANNOT_TEXTTYPE_Author, buf.data(),
                                   len));
  EXPECT_STREQ(L"Jae Hyun Park",
               GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                   .c_str());

  // Check that the content is correct.
  len = FPDFAnnot_GetText(annot, FPDFANNOT_TEXTTYPE_Contents, nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(2690u, FPDFAnnot_GetText(annot, FPDFANNOT_TEXTTYPE_Contents,
                                     buf.data(), len));
  const wchar_t contents[] =
      L"This is a note for that highlight annotation. Very long highlight "
      "annotation. Long long long Long long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long longLong long longLong long longLong long longLong long "
      "longLong long long. END";
  EXPECT_STREQ(contents,
               GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                   .c_str());

  // Check that the quadpoints are correct.
  FS_QUADPOINTSF quadpoints;
  ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot, &quadpoints));
  EXPECT_EQ(115.802643f, quadpoints.x1);
  EXPECT_EQ(718.913940f, quadpoints.y1);
  EXPECT_EQ(157.211182f, quadpoints.x4);
  EXPECT_EQ(706.264465f, quadpoints.y4);

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbeddertest, ExtractInkMultiple) {
  // Open a file with three annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);

  // Check that there is a total of 3 annotation on its first page.
  EXPECT_EQ(3, FPDFPage_GetAnnotCount(page));

  // Check that the third annotation is of type "ink".
  FPDF_ANNOTATION annot;
  ASSERT_TRUE(FPDFPage_GetAnnot(page, 2, &annot));
  EXPECT_EQ(FPDF_ANNOT_INK, FPDFAnnot_GetSubtype(annot));

  // Check that the annotation color is blue with opacity.
  unsigned int R;
  unsigned int G;
  unsigned int B;
  unsigned int A;
  EXPECT_TRUE(
      FPDFAnnot_GetColor(annot, FPDFANNOT_COLORTYPE_Color, &R, &G, &B, &A));
  EXPECT_EQ(0u, R);
  EXPECT_EQ(0u, G);
  EXPECT_EQ(255u, B);
  EXPECT_EQ(76u, A);

  // Check that there is no content.
  EXPECT_EQ(2u,
            FPDFAnnot_GetText(annot, FPDFANNOT_TEXTTYPE_Contents, nullptr, 0));

  // Check that the rectange coordinates are correct.
  // Note that upon rendering, the rectangle coordinates will be adjusted.
  FS_RECTF rect;
  ASSERT_TRUE(FPDFAnnot_GetRect(annot, &rect));
  EXPECT_EQ(351.820404f, rect.left);
  EXPECT_EQ(583.830688f, rect.bottom);
  EXPECT_EQ(475.336090f, rect.right);
  EXPECT_EQ(681.535034f, rect.top);

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbeddertest, AddIllegalSubtypeAnnotation) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_long_content.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);

  // Add an annotation with an illegal subtype.
  FPDF_ANNOTATION annot;
  ASSERT_FALSE(FPDFPage_CreateAnnot(page, -1, &annot));

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbeddertest, AddFirstTextAnnotation) {
  // Open a file with no annotation and load its first page.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);
  EXPECT_EQ(0, FPDFPage_GetAnnotCount(page));

  // Add an underline annotation to the page.
  FPDF_ANNOTATION annot;
  ASSERT_TRUE(FPDFPage_CreateAnnot(page, FPDF_ANNOT_TEXT, &annot));

  // Check that there is now 1 annotations on this page.
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));

  // Check that the subtype of the annotation is correct.
  EXPECT_EQ(FPDF_ANNOT_TEXT, FPDFAnnot_GetSubtype(annot));
  ASSERT_TRUE(FPDFPage_GetAnnot(page, 0, &annot));
  EXPECT_EQ(FPDF_ANNOT_TEXT, FPDFAnnot_GetSubtype(annot));

  // Set the color of the annotation.
  ASSERT_TRUE(
      FPDFAnnot_SetColor(annot, FPDFANNOT_COLORTYPE_Color, 51, 102, 153, 204));
  // Check that the color has been set correctly.
  unsigned int R;
  unsigned int G;
  unsigned int B;
  unsigned int A;
  EXPECT_TRUE(
      FPDFAnnot_GetColor(annot, FPDFANNOT_COLORTYPE_Color, &R, &G, &B, &A));
  EXPECT_EQ(51u, R);
  EXPECT_EQ(102u, G);
  EXPECT_EQ(153u, B);
  EXPECT_EQ(204u, A);

  // Change the color of the annotation.
  ASSERT_TRUE(
      FPDFAnnot_SetColor(annot, FPDFANNOT_COLORTYPE_Color, 204, 153, 102, 51));
  // Check that the color has been set correctly.
  EXPECT_TRUE(
      FPDFAnnot_GetColor(annot, FPDFANNOT_COLORTYPE_Color, &R, &G, &B, &A));
  EXPECT_EQ(204u, R);
  EXPECT_EQ(153u, G);
  EXPECT_EQ(102u, B);
  EXPECT_EQ(51u, A);

  // Set the annotation rectangle.
  FS_RECTF rect;
  EXPECT_FALSE(FPDFAnnot_GetRect(annot, &rect));
  rect.left = 35;
  rect.bottom = 150;
  rect.right = 53;
  rect.top = 165;
  ASSERT_TRUE(FPDFAnnot_SetRect(annot, rect));
  // Check that the annotation rectangle has been set correctly.
  ASSERT_TRUE(FPDFAnnot_GetRect(annot, &rect));
  EXPECT_EQ(35.f, rect.left);
  EXPECT_EQ(150.f, rect.bottom);
  EXPECT_EQ(53.f, rect.right);
  EXPECT_EQ(165.f, rect.top);

  // Set the content of the annotation.
  const wchar_t contents[] = L"Hello! This is a customized content.";
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text =
      GetFPDFWideString(contents);
  ASSERT_TRUE(
      FPDFAnnot_SetText(annot, FPDFANNOT_TEXTTYPE_Contents, text.get()));
  // Check that the content has been set correctly.
  unsigned long len =
      FPDFAnnot_GetText(annot, FPDFANNOT_TEXTTYPE_Contents, nullptr, 0);
  std::vector<char> buf(len);
  EXPECT_EQ(74u, FPDFAnnot_GetText(annot, FPDFANNOT_TEXTTYPE_Contents,
                                   buf.data(), len));
  EXPECT_STREQ(contents,
               GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                   .c_str());

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbeddertest, AddAndSaveUnderlineAnnotation) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_long_content.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);

  // Check that there is a total of one annotation on its first page, and verify
  // its quadpoints.
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));
  FPDF_ANNOTATION annot;
  ASSERT_TRUE(FPDFPage_GetAnnot(page, 0, &annot));
  FS_QUADPOINTSF quadpoints;
  ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot, &quadpoints));
  EXPECT_EQ(115.802643f, quadpoints.x1);
  EXPECT_EQ(718.913940f, quadpoints.y1);
  EXPECT_EQ(157.211182f, quadpoints.x4);
  EXPECT_EQ(706.264465f, quadpoints.y4);

  // Add an underline annotation to the page and set its quadpoints.
  ASSERT_TRUE(FPDFPage_CreateAnnot(page, FPDF_ANNOT_UNDERLINE, &annot));
  quadpoints.x1 = 140.802643f;
  quadpoints.x3 = 140.802643f;
  ASSERT_TRUE(FPDFAnnot_SetAttachmentPoints(annot, quadpoints));

  // Save the document, closing the page and document.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDF_ClosePage(page);

  // Open the saved document.
  std::string new_file = GetString();
  FPDF_FILEACCESS file_access;
  memset(&file_access, 0, sizeof(file_access));
  file_access.m_FileLen = new_file.size();
  file_access.m_GetBlock = GetBlockFromString;
  file_access.m_Param = &new_file;
  FPDF_DOCUMENT new_doc = FPDF_LoadCustomDocument(&file_access, nullptr);
  ASSERT_TRUE(new_doc);
  FPDF_PAGE new_page = FPDF_LoadPage(new_doc, 0);
  ASSERT_TRUE(new_page);

  // Check that the saved document has 2 annotations on the first page
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(new_page));

  // Check that the second annotation is an underline annotation and verify
  // its quadpoints.
  FPDF_ANNOTATION new_annot;
  ASSERT_TRUE(FPDFPage_GetAnnot(new_page, 1, &new_annot));
  EXPECT_EQ(FPDF_ANNOT_UNDERLINE, FPDFAnnot_GetSubtype(new_annot));
  FS_QUADPOINTSF new_quadpoints;
  ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(new_annot, &new_quadpoints));
  EXPECT_NEAR(quadpoints.x1, new_quadpoints.x1, 0.001f);
  EXPECT_NEAR(quadpoints.y1, new_quadpoints.y1, 0.001f);
  EXPECT_NEAR(quadpoints.x4, new_quadpoints.x4, 0.001f);
  EXPECT_NEAR(quadpoints.y4, new_quadpoints.y4, 0.001f);

  FPDF_ClosePage(new_page);
  FPDF_CloseDocument(new_doc);
}
