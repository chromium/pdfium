// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>
#include <vector>

#include "core/fxcrt/fx_system.h"
#include "public/fpdf_annot.h"
#include "public/fpdf_edit.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFAnnotEmbeddertest : public EmbedderTest {};

TEST_F(FPDFAnnotEmbeddertest, RenderAnnotWithOnlyRolloverAP) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_rollover_ap.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);

  // This annotation has a malformed appearance stream, which does not have its
  // normal appearance defined, only its rollover appearance. In this case, its
  // normal appearance should be generated, allowing the highlight annotation to
  // still display.
  FPDF_BITMAP bitmap = RenderPageWithFlags(page, form_handle(), FPDF_ANNOT);
  CompareBitmap(bitmap, 612, 792, "dc98f06da047bd8aabfa99562d2cbd1e");
  FPDFBitmap_Destroy(bitmap);

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbeddertest, ExtractHighlightLongContent) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_long_content.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);

  // Check that there is a total of 1 annotation on its first page.
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));

  // Check that the annotation is of type "highlight".
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, 0);
  ASSERT_TRUE(annot);
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
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> author_key =
      GetFPDFWideString(L"T");
  EXPECT_EQ(FPDF_OBJECT_STRING,
            FPDFAnnot_GetValueType(annot, author_key.get()));
  unsigned long len =
      FPDFAnnot_GetStringValue(annot, author_key.get(), nullptr, 0);
  std::vector<char> buf(len);
  EXPECT_EQ(28u,
            FPDFAnnot_GetStringValue(annot, author_key.get(), buf.data(), len));
  EXPECT_STREQ(L"Jae Hyun Park",
               GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                   .c_str());

  // Check that the content is correct.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> contents_key =
      GetFPDFWideString(L"Contents");
  EXPECT_EQ(FPDF_OBJECT_STRING,
            FPDFAnnot_GetValueType(annot, contents_key.get()));
  len = FPDFAnnot_GetStringValue(annot, contents_key.get(), nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(2690u, FPDFAnnot_GetStringValue(annot, contents_key.get(),
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
  FS_QUADPOINTSF quadpoints = FPDFAnnot_GetAttachmentPoints(annot);
  EXPECT_EQ(115.802643f, quadpoints.x1);
  EXPECT_EQ(718.913940f, quadpoints.y1);
  EXPECT_EQ(157.211182f, quadpoints.x4);
  EXPECT_EQ(706.264465f, quadpoints.y4);

  FPDFPage_CloseAnnot(annot);
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
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, 2);
  ASSERT_TRUE(annot);
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
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> contents_key =
      GetFPDFWideString(L"Contents");
  EXPECT_EQ(2u,
            FPDFAnnot_GetStringValue(annot, contents_key.get(), nullptr, 0));

  // Check that the rectange coordinates are correct.
  // Note that upon rendering, the rectangle coordinates will be adjusted.
  FS_RECTF rect = FPDFAnnot_GetRect(annot);
  EXPECT_EQ(351.820404f, rect.left);
  EXPECT_EQ(583.830688f, rect.bottom);
  EXPECT_EQ(475.336090f, rect.right);
  EXPECT_EQ(681.535034f, rect.top);

  FPDFPage_CloseAnnot(annot);
  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbeddertest, AddIllegalSubtypeAnnotation) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_long_content.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);

  // Add an annotation with an illegal subtype.
  ASSERT_FALSE(FPDFPage_CreateAnnot(page, -1));

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbeddertest, AddFirstTextAnnotation) {
  // Open a file with no annotation and load its first page.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);
  EXPECT_EQ(0, FPDFPage_GetAnnotCount(page));

  // Add a text annotation to the page.
  FPDF_ANNOTATION annot = FPDFPage_CreateAnnot(page, FPDF_ANNOT_TEXT);
  ASSERT_TRUE(annot);

  // Check that there is now 1 annotations on this page.
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));

  // Check that the subtype of the annotation is correct.
  EXPECT_EQ(FPDF_ANNOT_TEXT, FPDFAnnot_GetSubtype(annot));
  FPDFPage_CloseAnnot(annot);

  annot = FPDFPage_GetAnnot(page, 0);
  ASSERT_TRUE(annot);
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
  FS_RECTF rect = FPDFAnnot_GetRect(annot);
  EXPECT_EQ(0.f, rect.left);
  EXPECT_EQ(0.f, rect.right);
  rect.left = 35;
  rect.bottom = 150;
  rect.right = 53;
  rect.top = 165;
  ASSERT_TRUE(FPDFAnnot_SetRect(annot, &rect));
  // Check that the annotation rectangle has been set correctly.
  rect = FPDFAnnot_GetRect(annot);
  EXPECT_EQ(35.f, rect.left);
  EXPECT_EQ(150.f, rect.bottom);
  EXPECT_EQ(53.f, rect.right);
  EXPECT_EQ(165.f, rect.top);

  // Set the content of the annotation.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> contents_key =
      GetFPDFWideString(L"Contents");
  const wchar_t contents[] = L"Hello! This is a customized content.";
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text =
      GetFPDFWideString(contents);
  ASSERT_TRUE(FPDFAnnot_SetStringValue(annot, contents_key.get(), text.get()));
  // Check that the content has been set correctly.
  unsigned long len =
      FPDFAnnot_GetStringValue(annot, contents_key.get(), nullptr, 0);
  std::vector<char> buf(len);
  EXPECT_EQ(74u, FPDFAnnot_GetStringValue(annot, contents_key.get(), buf.data(),
                                          len));
  EXPECT_STREQ(contents,
               GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                   .c_str());

  FPDFPage_CloseAnnot(annot);
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
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, 0);
  ASSERT_TRUE(annot);
  FS_QUADPOINTSF quadpoints = FPDFAnnot_GetAttachmentPoints(annot);
  EXPECT_EQ(115.802643f, quadpoints.x1);
  EXPECT_EQ(718.913940f, quadpoints.y1);
  EXPECT_EQ(157.211182f, quadpoints.x4);
  EXPECT_EQ(706.264465f, quadpoints.y4);
  FPDFPage_CloseAnnot(annot);

  // Add an underline annotation to the page and set its quadpoints.
  annot = FPDFPage_CreateAnnot(page, FPDF_ANNOT_UNDERLINE);
  ASSERT_TRUE(annot);
  quadpoints.x1 = 140.802643f;
  quadpoints.x3 = 140.802643f;
  ASSERT_TRUE(FPDFAnnot_SetAttachmentPoints(annot, &quadpoints));
  FPDFPage_CloseAnnot(annot);

  // Save the document, closing the page and document.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDF_ClosePage(page);

  // Open the saved document.
  const char md5[] = "184b67b322edaee27994b3232544b8b3";
  TestSaved(612, 792, md5);

  // Check that the saved document has 2 annotations on the first page
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(m_SavedPage));

  // Check that the second annotation is an underline annotation and verify
  // its quadpoints.
  FPDF_ANNOTATION new_annot = FPDFPage_GetAnnot(m_SavedPage, 1);
  ASSERT_TRUE(new_annot);
  EXPECT_EQ(FPDF_ANNOT_UNDERLINE, FPDFAnnot_GetSubtype(new_annot));
  FS_QUADPOINTSF new_quadpoints = FPDFAnnot_GetAttachmentPoints(new_annot);
  EXPECT_NEAR(quadpoints.x1, new_quadpoints.x1, 0.001f);
  EXPECT_NEAR(quadpoints.y1, new_quadpoints.y1, 0.001f);
  EXPECT_NEAR(quadpoints.x4, new_quadpoints.x4, 0.001f);
  EXPECT_NEAR(quadpoints.y4, new_quadpoints.y4, 0.001f);

  FPDFPage_CloseAnnot(new_annot);
  CloseSaved();
}

TEST_F(FPDFAnnotEmbeddertest, ModifyRectQuadpointsWithAP) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_square_with_ap.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);
  EXPECT_EQ(4, FPDFPage_GetAnnotCount(page));

  // Retrieve the highlight annotation which has its AP stream already defined.
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, 0);
  ASSERT_TRUE(annot);
  EXPECT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnot_GetSubtype(annot));

  // Check that color cannot be set when an AP stream is defined already.
  EXPECT_FALSE(
      FPDFAnnot_SetColor(annot, FPDFANNOT_COLORTYPE_Color, 51, 102, 153, 204));

  // Check that when getting the attachment points, bounding box points are
  // returned since this is a markup annotation with AP defined.
  FS_QUADPOINTSF quadpoints = FPDFAnnot_GetAttachmentPoints(annot);
  EXPECT_NEAR(0.f, quadpoints.x1, 0.001f);
  EXPECT_NEAR(16.9955f, quadpoints.y1, 0.001f);
  EXPECT_NEAR(68.5953f, quadpoints.x4, 0.001f);
  EXPECT_NEAR(0.f, quadpoints.y4, 0.001f);

  // Check that when new attachment points define a smaller bounding box, the
  // bounding box does not get updated.
  quadpoints.x1 = 1.0f;
  quadpoints.x3 = 1.0f;
  ASSERT_TRUE(FPDFAnnot_SetAttachmentPoints(annot, &quadpoints));
  FS_QUADPOINTSF new_quadpoints = FPDFAnnot_GetAttachmentPoints(annot);
  EXPECT_NE(quadpoints.x1, new_quadpoints.x1);

  // Check that the bounding box gets updated successfully when valid attachment
  // points are set.
  quadpoints.x1 = 0.f;
  quadpoints.y1 = 721.792f;
  quadpoints.x2 = 133.055f;
  quadpoints.y2 = 721.792f;
  quadpoints.x3 = 0.f;
  quadpoints.x4 = 133.055f;
  ASSERT_TRUE(FPDFAnnot_SetAttachmentPoints(annot, &quadpoints));
  new_quadpoints = FPDFAnnot_GetAttachmentPoints(annot);
  EXPECT_EQ(quadpoints.x1, new_quadpoints.x1);
  EXPECT_EQ(quadpoints.y1, new_quadpoints.y1);
  EXPECT_EQ(quadpoints.x4, new_quadpoints.x4);
  EXPECT_EQ(quadpoints.y4, new_quadpoints.y4);

  // Check that when getting the annotation rectangle, rectangle points are
  // returned, but not bounding box points.
  FS_RECTF rect = FPDFAnnot_GetRect(annot);
  EXPECT_NEAR(67.7299f, rect.left, 0.001f);
  EXPECT_NEAR(704.296f, rect.bottom, 0.001f);
  EXPECT_NEAR(136.325f, rect.right, 0.001f);
  EXPECT_NEAR(721.292f, rect.top, 0.001f);

  // Check that the rectangle gets updated successfully when a valid rectangle
  // is set, and that the bounding box is not modified.
  rect.left = 0.f;
  rect.bottom = 0.f;
  rect.right = 134.055f;
  rect.top = 722.792f;
  ASSERT_TRUE(FPDFAnnot_SetRect(annot, &rect));
  FS_RECTF new_rect = FPDFAnnot_GetRect(annot);
  EXPECT_EQ(rect.right, new_rect.right);
  new_quadpoints = FPDFAnnot_GetAttachmentPoints(annot);
  EXPECT_NE(rect.right, new_quadpoints.x2);

  FPDFPage_CloseAnnot(annot);

  // Retrieve the square annotation which has its AP stream already defined.
  annot = FPDFPage_GetAnnot(page, 2);
  ASSERT_TRUE(annot);
  EXPECT_EQ(FPDF_ANNOT_SQUARE, FPDFAnnot_GetSubtype(annot));

  // Check that the rectangle and the bounding box get updated successfully when
  // a valid rectangle is set, since this is not a markup annotation.
  rect = FPDFAnnot_GetRect(annot);
  rect.right += 1.f;
  ASSERT_TRUE(FPDFAnnot_SetRect(annot, &rect));
  new_rect = FPDFAnnot_GetRect(annot);
  EXPECT_EQ(rect.right, new_rect.right);

  FPDFPage_CloseAnnot(annot);
  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbeddertest, RemoveAnnotation) {
  // Open a file with 3 annotations on its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);
  EXPECT_EQ(3, FPDFPage_GetAnnotCount(page));

  // Check that the annotations have the expected rectangle coordinates.
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, 0);
  FS_RECTF rect = FPDFAnnot_GetRect(annot);
  EXPECT_NEAR(86.1971f, rect.left, 0.001f);
  FPDFPage_CloseAnnot(annot);

  annot = FPDFPage_GetAnnot(page, 1);
  rect = FPDFAnnot_GetRect(annot);
  EXPECT_NEAR(149.8127f, rect.left, 0.001f);
  FPDFPage_CloseAnnot(annot);

  annot = FPDFPage_GetAnnot(page, 2);
  rect = FPDFAnnot_GetRect(annot);
  EXPECT_NEAR(351.8204f, rect.left, 0.001f);
  FPDFPage_CloseAnnot(annot);

  // Check that nothing happens when attempting to remove an annotation with an
  // out-of-bound index.
  EXPECT_FALSE(FPDFPage_RemoveAnnot(page, 4));
  EXPECT_FALSE(FPDFPage_RemoveAnnot(page, -1));
  EXPECT_EQ(3, FPDFPage_GetAnnotCount(page));

  // Remove the second annotation.
  EXPECT_TRUE(FPDFPage_RemoveAnnot(page, 1));
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));
  EXPECT_FALSE(FPDFPage_GetAnnot(page, 2));

  // Save the document, closing the page and document.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDF_ClosePage(page);

  // TODO(npm): TestSaved changes annot rect dimensions by 1??
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

  // Check that the saved document has 2 annotations on the first page.
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(new_page));

  // Check that the remaining 2 annotations are the original 1st and 3rd ones by
  // verifying their rectangle coordinates.
  annot = FPDFPage_GetAnnot(new_page, 0);
  rect = FPDFAnnot_GetRect(annot);
  EXPECT_NEAR(86.1971f, rect.left, 0.001f);
  FPDFPage_CloseAnnot(annot);

  annot = FPDFPage_GetAnnot(new_page, 1);
  rect = FPDFAnnot_GetRect(annot);
  EXPECT_NEAR(351.8204f, rect.left, 0.001f);
  FPDFPage_CloseAnnot(annot);
  FPDF_ClosePage(new_page);
  FPDF_CloseDocument(new_doc);
}

TEST_F(FPDFAnnotEmbeddertest, AddAndModifyPath) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  const char md5_original[] = "c35408717759562d1f8bf33d317483d2";
  const char md5_modified_path[] = "cf3cea74bd46497520ff6c4d1ea228c8";
  const char md5_two_paths[] = "e8994452fc4385337bae5522354e10ff";
  const char md5_new_annot[] = "ee5372b31fede117fc83b9384598aa25";
#elif _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  const char md5_original[] = "4f64add0190ede63f7bb9eb1e2e83edb";
  const char md5_modified_path[] = "681f0d0738dded0722e146f6c219bfac";
  const char md5_two_paths[] = "67c7e90fc3b64e20f6b69a1744f7f4f0";
  const char md5_new_annot[] = "262187984451bae2fe826067d68623ff";
#else
  const char md5_original[] = "02e1c6adff8fee4aeabd91c2c2e4be43";
  const char md5_modified_path[] = "87a78cbacd8509b961a67be56b5665a2";
  const char md5_two_paths[] = "76e985c18b73ceacf409f77f978176d4";
  const char md5_new_annot[] = "c95de7a9a1f61faca03d953961a319b9";
#endif

  // Open a file with two annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));

  // Check that the page renders correctly.
  FPDF_BITMAP bitmap = RenderPageWithFlags(page, form_handle_, FPDF_ANNOT);
  CompareBitmap(bitmap, 595, 842, md5_original);
  FPDFBitmap_Destroy(bitmap);

  // Retrieve the stamp annotation which has its AP stream already defined.
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, 0);
  ASSERT_TRUE(annot);

  // Check that this annotation has one path object and retrieve it.
  EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot));
  FPDF_PAGEOBJECT path = FPDFAnnot_GetObject(annot, 1);
  EXPECT_FALSE(path);
  path = FPDFAnnot_GetObject(annot, 0);
  EXPECT_EQ(FPDF_PAGEOBJ_PATH, FPDFPageObj_GetType(path));
  EXPECT_TRUE(path);

  // Modify the color of the path object.
  EXPECT_TRUE(FPDFPath_SetStrokeColor(path, 0, 0, 0, 255));
  EXPECT_TRUE(FPDFAnnot_UpdateObject(annot, path));

  // Check that the page with the modified annotation renders correctly.
  bitmap = RenderPageWithFlags(page, form_handle_, FPDF_ANNOT);
  CompareBitmap(bitmap, 595, 842, md5_modified_path);
  FPDFBitmap_Destroy(bitmap);

  // Add a second path object to the same annotation.
  FPDF_PAGEOBJECT dot = FPDFPageObj_CreateNewPath(7, 84);
  EXPECT_TRUE(FPDFPath_BezierTo(dot, 9, 86, 10, 87, 11, 88));
  EXPECT_TRUE(FPDFPath_SetStrokeColor(dot, 255, 0, 0, 100));
  EXPECT_TRUE(FPDFPath_SetStrokeWidth(dot, 14));
  EXPECT_TRUE(FPDFPath_SetDrawMode(dot, 0, 1));
  EXPECT_TRUE(FPDFAnnot_AppendObject(annot, dot));
  EXPECT_EQ(2, FPDFAnnot_GetObjectCount(annot));

  // Check that the page with an annotation with two paths renders correctly.
  bitmap = RenderPageWithFlags(page, form_handle_, FPDF_ANNOT);
  CompareBitmap(bitmap, 595, 842, md5_two_paths);
  FPDFBitmap_Destroy(bitmap);

  // Delete the newly added path object.
  EXPECT_TRUE(FPDFAnnot_RemoveObject(annot, 1));
  EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot));
  FPDFPage_CloseAnnot(annot);

  // Check that the page renders the same as before.
  bitmap = RenderPageWithFlags(page, form_handle_, FPDF_ANNOT);
  CompareBitmap(bitmap, 595, 842, md5_modified_path);
  FPDFBitmap_Destroy(bitmap);

  // Create another stamp annotation and set its annotation rectangle.
  annot = FPDFPage_CreateAnnot(page, FPDF_ANNOT_STAMP);
  ASSERT_TRUE(annot);
  FS_RECTF rect;
  rect.left = 200.f;
  rect.bottom = 400.f;
  rect.right = 500.f;
  rect.top = 600.f;
  EXPECT_TRUE(FPDFAnnot_SetRect(annot, &rect));

  // Add a new path to the annotation.
  FPDF_PAGEOBJECT check = FPDFPageObj_CreateNewPath(200, 500);
  EXPECT_TRUE(FPDFPath_LineTo(check, 300, 400));
  EXPECT_TRUE(FPDFPath_LineTo(check, 500, 600));
  EXPECT_TRUE(FPDFPath_MoveTo(check, 350, 550));
  EXPECT_TRUE(FPDFPath_LineTo(check, 450, 450));
  EXPECT_TRUE(FPDFPath_SetStrokeColor(check, 0, 255, 255, 180));
  EXPECT_TRUE(FPDFPath_SetStrokeWidth(check, 8.35f));
  EXPECT_TRUE(FPDFPath_SetDrawMode(check, 0, 1));
  EXPECT_TRUE(FPDFAnnot_AppendObject(annot, check));
  EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot));

  // Check that the annotation's bounding box came from its rectangle.
  FS_RECTF new_rect = FPDFAnnot_GetRect(annot);
  EXPECT_EQ(rect.left, new_rect.left);
  EXPECT_EQ(rect.bottom, new_rect.bottom);
  EXPECT_EQ(rect.right, new_rect.right);
  EXPECT_EQ(rect.top, new_rect.top);

  // Save the document, closing the page and document.
  FPDFPage_CloseAnnot(annot);
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDF_ClosePage(page);

  // Open the saved document.
  TestSaved(595, 842, md5_new_annot);

  // Check that the document has a correct count of annotations and objects.
  EXPECT_EQ(3, FPDFPage_GetAnnotCount(m_SavedPage));
  annot = FPDFPage_GetAnnot(m_SavedPage, 2);
  ASSERT_TRUE(annot);
  EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot));

  // Check that the new annotation's rectangle is as defined.
  new_rect = FPDFAnnot_GetRect(annot);
  EXPECT_EQ(rect.left, new_rect.left);
  EXPECT_EQ(rect.bottom, new_rect.bottom);
  EXPECT_EQ(rect.right, new_rect.right);
  EXPECT_EQ(rect.top, new_rect.top);

  FPDFPage_CloseAnnot(annot);
  CloseSaved();
}

TEST_F(FPDFAnnotEmbeddertest, ModifyAnnotationFlags) {
  // Open a file with an annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_rollover_ap.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);

  // Check that the page renders correctly.
  FPDF_BITMAP bitmap = RenderPageWithFlags(page, form_handle_, FPDF_ANNOT);
  CompareBitmap(bitmap, 612, 792, "dc98f06da047bd8aabfa99562d2cbd1e");
  FPDFBitmap_Destroy(bitmap);

  // Retrieve the annotation.
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, 0);
  ASSERT_TRUE(annot);

  // Check that the original flag values are as expected.
  int flags = FPDFAnnot_GetFlags(annot);
  EXPECT_FALSE(flags & FPDF_ANNOT_FLAG_HIDDEN);
  EXPECT_TRUE(flags & FPDF_ANNOT_FLAG_PRINT);

  // Set the HIDDEN flag.
  flags |= FPDF_ANNOT_FLAG_HIDDEN;
  EXPECT_TRUE(FPDFAnnot_SetFlags(annot, flags));
  flags = FPDFAnnot_GetFlags(annot);
  EXPECT_TRUE(flags & FPDF_ANNOT_FLAG_HIDDEN);
  EXPECT_TRUE(flags & FPDF_ANNOT_FLAG_PRINT);

  // Check that the page renders correctly without rendering the annotation.
  bitmap = RenderPageWithFlags(page, form_handle_, FPDF_ANNOT);
  CompareBitmap(bitmap, 612, 792, "1940568c9ba33bac5d0b1ee9558c76b3");
  FPDFBitmap_Destroy(bitmap);

  // Unset the HIDDEN flag.
  EXPECT_TRUE(FPDFAnnot_SetFlags(annot, FPDF_ANNOT_FLAG_NONE));
  EXPECT_FALSE(FPDFAnnot_GetFlags(annot));
  flags &= ~FPDF_ANNOT_FLAG_HIDDEN;
  EXPECT_TRUE(FPDFAnnot_SetFlags(annot, flags));
  flags = FPDFAnnot_GetFlags(annot);
  EXPECT_FALSE(flags & FPDF_ANNOT_FLAG_HIDDEN);
  EXPECT_TRUE(flags & FPDF_ANNOT_FLAG_PRINT);

  // Check that the page renders correctly as before.
  bitmap = RenderPageWithFlags(page, form_handle_, FPDF_ANNOT);
  CompareBitmap(bitmap, 612, 792, "dc98f06da047bd8aabfa99562d2cbd1e");
  FPDFBitmap_Destroy(bitmap);

  FPDFPage_CloseAnnot(annot);
  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbeddertest, AddAndModifyImage) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  const char md5_original[] = "c35408717759562d1f8bf33d317483d2";
  const char md5_new_image[] = "ff012f5697436dfcaec25b32d1333596";
  const char md5_modified_image[] = "86cf8cb2755a7a2046a543e66d9c1e61";
#elif _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  const char md5_original[] = "4f64add0190ede63f7bb9eb1e2e83edb";
  const char md5_new_image[] = "6fb176c20996cc554d0210d8c8b6138f";
  const char md5_modified_image[] = "546959714dfb0dcd7e7b00259e8d178c";
#else
  const char md5_original[] = "02e1c6adff8fee4aeabd91c2c2e4be43";
  const char md5_new_image[] = "e7658232abd8977cdc3367dd02aee04a";
  const char md5_modified_image[] = "f393432b9a9b452ea69022f46c8b3f75";
#endif

  // Open a file with two annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));

  // Check that the page renders correctly.
  FPDF_BITMAP bitmap = RenderPageWithFlags(page, form_handle_, FPDF_ANNOT);
  CompareBitmap(bitmap, 595, 842, md5_original);
  FPDFBitmap_Destroy(bitmap);

  // Create a stamp annotation and set its annotation rectangle.
  FPDF_ANNOTATION annot = FPDFPage_CreateAnnot(page, FPDF_ANNOT_STAMP);
  ASSERT_TRUE(annot);
  FS_RECTF rect;
  rect.left = 200.f;
  rect.bottom = 600.f;
  rect.right = 400.f;
  rect.top = 800.f;
  EXPECT_TRUE(FPDFAnnot_SetRect(annot, &rect));

  // Add a solid-color translucent image object to the new annotation.
  constexpr int kBitmapSize = 200;
  FPDF_BITMAP image_bitmap = FPDFBitmap_Create(kBitmapSize, kBitmapSize, 1);
  FPDFBitmap_FillRect(image_bitmap, 0, 0, kBitmapSize, kBitmapSize, 0xeeeecccc);
  EXPECT_EQ(kBitmapSize, FPDFBitmap_GetWidth(image_bitmap));
  EXPECT_EQ(kBitmapSize, FPDFBitmap_GetHeight(image_bitmap));
  FPDF_PAGEOBJECT image_object = FPDFPageObj_NewImageObj(document());
  ASSERT_TRUE(FPDFImageObj_SetBitmap(&page, 0, image_object, image_bitmap));
  ASSERT_TRUE(FPDFImageObj_SetMatrix(image_object, kBitmapSize, 0, 0,
                                     kBitmapSize, 0, 0));
  FPDFPageObj_Transform(image_object, 1, 0, 0, 1, 200, 600);
  EXPECT_TRUE(FPDFAnnot_AppendObject(annot, image_object));
  FPDFPage_CloseAnnot(annot);

  // Check that the page renders correctly with the new image object.
  bitmap = RenderPageWithFlags(page, form_handle_, FPDF_ANNOT);
  CompareBitmap(bitmap, 595, 842, md5_new_image);
  FPDFBitmap_Destroy(bitmap);

  // Retrieve the newly added stamp annotation and its image object.
  annot = FPDFPage_GetAnnot(page, 2);
  ASSERT_TRUE(annot);
  EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot));
  image_object = FPDFAnnot_GetObject(annot, 0);
  EXPECT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(image_object));

  // Modify the image in the new annotation.
  FPDFBitmap_FillRect(image_bitmap, 0, 0, kBitmapSize, kBitmapSize, 0xff000000);
  ASSERT_TRUE(FPDFImageObj_SetBitmap(&page, 0, image_object, image_bitmap));
  EXPECT_TRUE(FPDFAnnot_UpdateObject(annot, image_object));
  FPDFPage_CloseAnnot(annot);

  // Save the document, closing the page and document.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDF_ClosePage(page);

  // Test that the saved document renders the modified image object correctly.
  TestSaved(595, 842, md5_modified_image);

  FPDFBitmap_Destroy(image_bitmap);
  CloseSaved();
}

TEST_F(FPDFAnnotEmbeddertest, AddAndModifyText) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  const char md5_original[] = "c35408717759562d1f8bf33d317483d2";
  const char md5_new_text[] = "e5680ed048c2cfd9a1d27212cdf41286";
  const char md5_modified_text[] = "79f5cfb0b07caaf936f65f6a7a57ce77";
#elif _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  const char md5_original[] = "4f64add0190ede63f7bb9eb1e2e83edb";
  const char md5_new_text[] = "998abae4962f8f41e094e7612d8339fc";
  const char md5_modified_text[] = "e89b82ca4589b8f0b45fff42ca3a96a4";
#else
  const char md5_original[] = "02e1c6adff8fee4aeabd91c2c2e4be43";
  const char md5_new_text[] = "3fbbaec4d846ccf2be89e09daae0273d";
  const char md5_modified_text[] = "2ad0acaf2d8990bcdf48e1d12e6c44ad";
#endif

  // Open a file with two annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));

  // Check that the page renders correctly.
  FPDF_BITMAP bitmap = RenderPageWithFlags(page, form_handle_, FPDF_ANNOT);
  CompareBitmap(bitmap, 595, 842, md5_original);
  FPDFBitmap_Destroy(bitmap);

  // Create a stamp annotation and set its annotation rectangle.
  FPDF_ANNOTATION annot = FPDFPage_CreateAnnot(page, FPDF_ANNOT_STAMP);
  ASSERT_TRUE(annot);
  FS_RECTF rect;
  rect.left = 200.f;
  rect.bottom = 550.f;
  rect.right = 450.f;
  rect.top = 650.f;
  EXPECT_TRUE(FPDFAnnot_SetRect(annot, &rect));

  // Add a translucent text object to the new annotation.
  FPDF_PAGEOBJECT text_object =
      FPDFPageObj_NewTextObj(document(), "Arial", 12.0f);
  EXPECT_TRUE(text_object);
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text =
      GetFPDFWideString(L"I'm a translucent text laying on other text.");
  EXPECT_TRUE(FPDFText_SetText(text_object, text.get()));
  EXPECT_TRUE(FPDFText_SetFillColor(text_object, 0, 0, 255, 150));
  FPDFPageObj_Transform(text_object, 1, 0, 0, 1, 200, 600);
  EXPECT_TRUE(FPDFAnnot_AppendObject(annot, text_object));
  FPDFPage_CloseAnnot(annot);

  // Check that the page renders correctly with the new text object.
  bitmap = RenderPageWithFlags(page, form_handle_, FPDF_ANNOT);
  CompareBitmap(bitmap, 595, 842, md5_new_text);
  FPDFBitmap_Destroy(bitmap);

  // Retrieve the newly added stamp annotation and its text object.
  annot = FPDFPage_GetAnnot(page, 2);
  ASSERT_TRUE(annot);
  EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot));
  text_object = FPDFAnnot_GetObject(annot, 0);
  EXPECT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(text_object));

  // Modify the text in the new annotation.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> new_text =
      GetFPDFWideString(L"New text!");
  EXPECT_TRUE(FPDFText_SetText(text_object, new_text.get()));
  EXPECT_TRUE(FPDFAnnot_UpdateObject(annot, text_object));
  FPDFPage_CloseAnnot(annot);

  // Check that the page renders correctly with the modified text object.
  bitmap = RenderPageWithFlags(page, form_handle_, FPDF_ANNOT);
  CompareBitmap(bitmap, 595, 842, md5_modified_text);
  FPDFBitmap_Destroy(bitmap);

  // Remove the new annotation, and check that the page renders as before.
  EXPECT_TRUE(FPDFPage_RemoveAnnot(page, 2));
  bitmap = RenderPageWithFlags(page, form_handle_, FPDF_ANNOT);
  CompareBitmap(bitmap, 595, 842, md5_original);
  FPDFBitmap_Destroy(bitmap);

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbeddertest, GetSetStringValue) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);

  // Retrieve the first annotation.
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, 0);
  ASSERT_TRUE(annot);

  // Check that a non-existent key does not exist.
  EXPECT_FALSE(FPDFAnnot_HasKey(annot, GetFPDFWideString(L"none").get()));

  // Check that the string value of a non-string dictionary entry is empty.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> ap_key =
      GetFPDFWideString(L"AP");
  EXPECT_TRUE(FPDFAnnot_HasKey(annot, ap_key.get()));
  EXPECT_EQ(FPDF_OBJECT_REFERENCE, FPDFAnnot_GetValueType(annot, ap_key.get()));
  EXPECT_EQ(2u, FPDFAnnot_GetStringValue(annot, ap_key.get(), nullptr, 0));

  // Check that the string value of the hash is correct.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> hash_key =
      GetFPDFWideString(L"AAPL:Hash");
  EXPECT_EQ(FPDF_OBJECT_NAME, FPDFAnnot_GetValueType(annot, hash_key.get()));
  unsigned long len =
      FPDFAnnot_GetStringValue(annot, hash_key.get(), nullptr, 0);
  std::vector<char> buf(len);
  EXPECT_EQ(66u,
            FPDFAnnot_GetStringValue(annot, hash_key.get(), buf.data(), len));
  EXPECT_STREQ(L"395fbcb98d558681742f30683a62a2ad",
               GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                   .c_str());

  // Check that the string value of the modified date is correct.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> date_key =
      GetFPDFWideString(L"M");
  EXPECT_EQ(FPDF_OBJECT_NAME, FPDFAnnot_GetValueType(annot, hash_key.get()));
  len = FPDFAnnot_GetStringValue(annot, date_key.get(), nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(44u,
            FPDFAnnot_GetStringValue(annot, date_key.get(), buf.data(), len));
  EXPECT_STREQ(L"D:201706071721Z00'00'",
               GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                   .c_str());

  // Update the date entry for the annotation.
  const wchar_t new_date[] = L"D:201706282359Z00'00'";
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text =
      GetFPDFWideString(new_date);
  EXPECT_TRUE(FPDFAnnot_SetStringValue(annot, date_key.get(), text.get()));

  // Save the document, closing the page and document.
  FPDFPage_CloseAnnot(annot);
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDF_ClosePage(page);

  // Open the saved annotation.
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  const char md5[] = "c35408717759562d1f8bf33d317483d2";
#elif _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  const char md5[] = "4f64add0190ede63f7bb9eb1e2e83edb";
#else
  const char md5[] = "02e1c6adff8fee4aeabd91c2c2e4be43";
#endif
  TestSaved(595, 842, md5);
  FPDF_ANNOTATION new_annot = FPDFPage_GetAnnot(m_SavedPage, 0);

  // Check that the string value of the modified date is the newly-set value.
  EXPECT_EQ(FPDF_OBJECT_STRING,
            FPDFAnnot_GetValueType(new_annot, date_key.get()));
  len = FPDFAnnot_GetStringValue(new_annot, date_key.get(), nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(44u, FPDFAnnot_GetStringValue(new_annot, date_key.get(), buf.data(),
                                          len));
  EXPECT_STREQ(new_date,
               GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                   .c_str());

  FPDFPage_CloseAnnot(new_annot);
  CloseSaved();
}

TEST_F(FPDFAnnotEmbeddertest, GetFormFieldFlagsTextField) {
  // Open file with form text fields.
  ASSERT_TRUE(OpenDocument("text_form_multiple.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);

  // Retrieve the first annotation: user-editable text field.
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, 0);
  ASSERT_TRUE(annot);

  // Check that the flag values are as expected.
  int flags = FPDFAnnot_GetFormFieldFlags(page, annot);
  EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
  FPDFPage_CloseAnnot(annot);

  // Retrieve the second annotation: read-only text field.
  annot = FPDFPage_GetAnnot(page, 1);
  ASSERT_TRUE(annot);

  // Check that the flag values are as expected.
  flags = FPDFAnnot_GetFormFieldFlags(page, annot);
  EXPECT_TRUE(flags & FPDF_FORMFLAG_READONLY);
  FPDFPage_CloseAnnot(annot);

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbeddertest, GetFormFieldFlagsComboBox) {
  // Open file with form text fields.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);

  // Retrieve the first annotation: user-editable combobox.
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, 0);
  ASSERT_TRUE(annot);

  // Check that the flag values are as expected.
  int flags = FPDFAnnot_GetFormFieldFlags(page, annot);
  EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
  EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
  EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
  FPDFPage_CloseAnnot(annot);

  // Retrieve the second annotation: regular combobox.
  annot = FPDFPage_GetAnnot(page, 1);
  ASSERT_TRUE(annot);

  // Check that the flag values are as expected.
  flags = FPDFAnnot_GetFormFieldFlags(page, annot);
  EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
  EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
  EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
  FPDFPage_CloseAnnot(annot);

  // Retrieve the third annotation: read-only combobox.
  annot = FPDFPage_GetAnnot(page, 2);
  ASSERT_TRUE(annot);

  // Check that the flag values are as expected.
  flags = FPDFAnnot_GetFormFieldFlags(page, annot);
  EXPECT_TRUE(flags & FPDF_FORMFLAG_READONLY);
  EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
  EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
  FPDFPage_CloseAnnot(annot);

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbeddertest, GetFormAnnotNull) {
  // Open file with form text fields.
  EXPECT_TRUE(OpenDocument("text_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Attempt to get an annotation where no annotation exists on page.
  FPDF_ANNOTATION annot =
      FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, 0, 0);
  EXPECT_FALSE(annot);

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbeddertest, GetFormAnnotAndCheckFlagsTextField) {
  // Open file with form text fields.
  EXPECT_TRUE(OpenDocument("text_form_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Retrieve user-editable text field annotation.
  FPDF_ANNOTATION annot =
      FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, 105, 118);
  ASSERT_TRUE(annot);

  // Check that interactive form annotation flag values are as expected.
  int flags = FPDFAnnot_GetFormFieldFlags(page, annot);
  EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
  FPDFPage_CloseAnnot(annot);

  // Retrieve read-only text field annotation.
  annot = FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, 105, 202);
  ASSERT_TRUE(annot);

  // Check that interactive form annotation flag values are as expected.
  flags = FPDFAnnot_GetFormFieldFlags(page, annot);
  EXPECT_TRUE(flags & FPDF_FORMFLAG_READONLY);
  FPDFPage_CloseAnnot(annot);

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbeddertest, GetFormAnnotAndCheckFlagsComboBox) {
  // Open file with form comboboxes.
  EXPECT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Retrieve user-editable combobox annotation.
  FPDF_ANNOTATION annot =
      FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, 102, 63);
  ASSERT_TRUE(annot);

  // Check that interactive form annotation flag values are as expected.
  int flags = FPDFAnnot_GetFormFieldFlags(page, annot);
  EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
  EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
  EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
  FPDFPage_CloseAnnot(annot);

  // Retrieve regular combobox annotation.
  annot = FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, 102, 113);
  ASSERT_TRUE(annot);

  // Check that interactive form annotation flag values are as expected.
  flags = FPDFAnnot_GetFormFieldFlags(page, annot);
  EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
  EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
  EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
  FPDFPage_CloseAnnot(annot);

  // Retrieve read-only combobox annotation.
  annot = FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, 102, 213);
  ASSERT_TRUE(annot);

  // Check that interactive form annotation flag values are as expected.
  flags = FPDFAnnot_GetFormFieldFlags(page, annot);
  EXPECT_TRUE(flags & FPDF_FORMFLAG_READONLY);
  EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
  EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
  FPDFPage_CloseAnnot(annot);

  UnloadPage(page);
}
