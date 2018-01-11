// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cwchar>
#include <memory>
#include <string>
#include <vector>

#include "core/fxcrt/fx_system.h"
#include "public/fpdf_annot.h"
#include "public/fpdf_edit.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gmock/include/gmock/gmock-matchers.h"
#include "testing/gtest/include/gtest/gtest.h"

static constexpr char kContentsKey[] = "Contents";

class FPDFAnnotEmbeddertest : public EmbedderTest {};

std::wstring BufferToWString(const std::vector<char>& buf) {
  return GetPlatformWString(reinterpret_cast<FPDF_WIDESTRING>(buf.data()));
}

std::string BufferToString(const std::vector<char>& buf) {
  return GetPlatformString(reinterpret_cast<FPDF_WIDESTRING>(buf.data()));
}

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
  static constexpr char kAuthorKey[] = "T";
  EXPECT_EQ(FPDF_OBJECT_STRING, FPDFAnnot_GetValueType(annot, kAuthorKey));
  unsigned long len = FPDFAnnot_GetStringValue(annot, kAuthorKey, nullptr, 0);
  std::vector<char> buf(len);
  EXPECT_EQ(28u, FPDFAnnot_GetStringValue(annot, kAuthorKey, buf.data(), len));
  EXPECT_STREQ(L"Jae Hyun Park", BufferToWString(buf).c_str());

  // Check that the content is correct.
  EXPECT_EQ(FPDF_OBJECT_STRING, FPDFAnnot_GetValueType(annot, kContentsKey));
  len = FPDFAnnot_GetStringValue(annot, kContentsKey, nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(2690u,
            FPDFAnnot_GetStringValue(annot, kContentsKey, buf.data(), len));
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
  EXPECT_STREQ(contents, BufferToWString(buf).c_str());

  // Check that the quadpoints are correct.
  FS_QUADPOINTSF quadpoints;
  ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot, &quadpoints));
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
  EXPECT_EQ(2u, FPDFAnnot_GetStringValue(annot, kContentsKey, nullptr, 0));

  // Check that the rectange coordinates are correct.
  // Note that upon rendering, the rectangle coordinates will be adjusted.
  FS_RECTF rect;
  ASSERT_TRUE(FPDFAnnot_GetRect(annot, &rect));
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
  FS_RECTF rect;
  ASSERT_TRUE(FPDFAnnot_GetRect(annot, &rect));
  EXPECT_EQ(0.f, rect.left);
  EXPECT_EQ(0.f, rect.right);
  rect.left = 35;
  rect.bottom = 150;
  rect.right = 53;
  rect.top = 165;
  ASSERT_TRUE(FPDFAnnot_SetRect(annot, &rect));
  // Check that the annotation rectangle has been set correctly.
  ASSERT_TRUE(FPDFAnnot_GetRect(annot, &rect));
  EXPECT_EQ(35.f, rect.left);
  EXPECT_EQ(150.f, rect.bottom);
  EXPECT_EQ(53.f, rect.right);
  EXPECT_EQ(165.f, rect.top);

  // Set the content of the annotation.
  static constexpr wchar_t contents[] = L"Hello! This is a customized content.";
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text =
      GetFPDFWideString(contents);
  ASSERT_TRUE(FPDFAnnot_SetStringValue(annot, kContentsKey, text.get()));
  // Check that the content has been set correctly.
  unsigned long len = FPDFAnnot_GetStringValue(annot, kContentsKey, nullptr, 0);
  std::vector<char> buf(len);
  EXPECT_EQ(74u,
            FPDFAnnot_GetStringValue(annot, kContentsKey, buf.data(), len));
  EXPECT_STREQ(contents, BufferToWString(buf).c_str());

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
  FS_QUADPOINTSF quadpoints;
  ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot, &quadpoints));
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
  const char md5[] = "dba153419f67b7c0c0e3d22d3e8910d5";

  OpenSavedDocument();
  page = LoadSavedPage(0);
  VerifySavedRendering(page, 612, 792, md5);

  // Check that the saved document has 2 annotations on the first page
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));

  // Check that the second annotation is an underline annotation and verify
  // its quadpoints.
  FPDF_ANNOTATION new_annot = FPDFPage_GetAnnot(page, 1);
  ASSERT_TRUE(new_annot);
  EXPECT_EQ(FPDF_ANNOT_UNDERLINE, FPDFAnnot_GetSubtype(new_annot));
  FS_QUADPOINTSF new_quadpoints;
  ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(new_annot, &new_quadpoints));
  EXPECT_NEAR(quadpoints.x1, new_quadpoints.x1, 0.001f);
  EXPECT_NEAR(quadpoints.y1, new_quadpoints.y1, 0.001f);
  EXPECT_NEAR(quadpoints.x4, new_quadpoints.x4, 0.001f);
  EXPECT_NEAR(quadpoints.y4, new_quadpoints.y4, 0.001f);

  FPDFPage_CloseAnnot(new_annot);

  CloseSavedPage(page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbeddertest, ModifyRectQuadpointsWithAP) {
#if _FX_PLATFORM_ == _FX_PLATFORM_APPLE_
  const char md5_original[] = "63af8432fab95a67cdebb7cd0e514941";
  const char md5_modified_highlight[] = "aec26075011349dec9bace891856b5f2";
  const char md5_modified_square[] = "057f57a32be95975775e5ec513fdcb56";
#elif _FX_PLATFORM_ == _FX_PLATFORM_WINDOWS_
  const char md5_original[] = "0e27376094f11490f74c65f3dc3a42c5";
  const char md5_modified_highlight[] = "66f3caef3a7d488a4fa1ad37fc06310e";
  const char md5_modified_square[] = "a456dad0bc6801ee2d6408a4394af563";
#else
  const char md5_original[] = "0e27376094f11490f74c65f3dc3a42c5";
  const char md5_modified_highlight[] = "66f3caef3a7d488a4fa1ad37fc06310e";
  const char md5_modified_square[] = "a456dad0bc6801ee2d6408a4394af563";
#endif

  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_square_with_ap.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);
  EXPECT_EQ(4, FPDFPage_GetAnnotCount(page));

  // Check that the original file renders correctly.
  FPDF_BITMAP bitmap = RenderPageWithFlags(page, form_handle_, FPDF_ANNOT);
  CompareBitmap(bitmap, 612, 792, md5_original);
  FPDFBitmap_Destroy(bitmap);

  // Retrieve the highlight annotation which has its AP stream already defined.
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, 0);
  ASSERT_TRUE(annot);
  EXPECT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnot_GetSubtype(annot));

  // Check that color cannot be set when an AP stream is defined already.
  EXPECT_FALSE(
      FPDFAnnot_SetColor(annot, FPDFANNOT_COLORTYPE_Color, 51, 102, 153, 204));

  // Verify its attachment points.
  FS_QUADPOINTSF quadpoints;
  ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot, &quadpoints));
  EXPECT_NEAR(72.0000f, quadpoints.x1, 0.001f);
  EXPECT_NEAR(720.792f, quadpoints.y1, 0.001f);
  EXPECT_NEAR(132.055f, quadpoints.x4, 0.001f);
  EXPECT_NEAR(704.796f, quadpoints.y4, 0.001f);

  // Check that updating the attachment points would succeed.
  quadpoints.x1 -= 50.f;
  quadpoints.x2 -= 50.f;
  quadpoints.x3 -= 50.f;
  quadpoints.x4 -= 50.f;
  ASSERT_TRUE(FPDFAnnot_SetAttachmentPoints(annot, &quadpoints));
  FS_QUADPOINTSF new_quadpoints;
  ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot, &new_quadpoints));
  EXPECT_EQ(quadpoints.x1, new_quadpoints.x1);
  EXPECT_EQ(quadpoints.y1, new_quadpoints.y1);
  EXPECT_EQ(quadpoints.x4, new_quadpoints.x4);
  EXPECT_EQ(quadpoints.y4, new_quadpoints.y4);

  // Check that updating quadpoints does not change the annotation's position.
  bitmap = RenderPageWithFlags(page, form_handle_, FPDF_ANNOT);
  CompareBitmap(bitmap, 612, 792, md5_original);
  FPDFBitmap_Destroy(bitmap);

  // Verify its annotation rectangle.
  FS_RECTF rect;
  ASSERT_TRUE(FPDFAnnot_GetRect(annot, &rect));
  EXPECT_NEAR(67.7299f, rect.left, 0.001f);
  EXPECT_NEAR(704.296f, rect.bottom, 0.001f);
  EXPECT_NEAR(136.325f, rect.right, 0.001f);
  EXPECT_NEAR(721.292f, rect.top, 0.001f);

  // Check that updating the rectangle would succeed.
  rect.left -= 60.f;
  rect.right -= 60.f;
  ASSERT_TRUE(FPDFAnnot_SetRect(annot, &rect));
  FS_RECTF new_rect;
  ASSERT_TRUE(FPDFAnnot_GetRect(annot, &new_rect));
  EXPECT_EQ(rect.right, new_rect.right);
  FPDFPage_CloseAnnot(annot);

  // Check that updating the rectangle changes the annotation's position.
  bitmap = RenderPageWithFlags(page, form_handle_, FPDF_ANNOT);
  CompareBitmap(bitmap, 612, 792, md5_modified_highlight);
  FPDFBitmap_Destroy(bitmap);

  // Retrieve the square annotation which has its AP stream already defined.
  annot = FPDFPage_GetAnnot(page, 2);
  ASSERT_TRUE(annot);
  EXPECT_EQ(FPDF_ANNOT_SQUARE, FPDFAnnot_GetSubtype(annot));

  // Check that updating the rectangle would succeed.
  ASSERT_TRUE(FPDFAnnot_GetRect(annot, &rect));
  rect.left += 70.f;
  rect.right += 70.f;
  ASSERT_TRUE(FPDFAnnot_SetRect(annot, &rect));
  ASSERT_TRUE(FPDFAnnot_GetRect(annot, &new_rect));
  EXPECT_EQ(rect.right, new_rect.right);

  // Check that updating the rectangle changes the square annotation's position.
  bitmap = RenderPageWithFlags(page, form_handle_, FPDF_ANNOT);
  CompareBitmap(bitmap, 612, 792, md5_modified_square);
  FPDFBitmap_Destroy(bitmap);

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
  FS_RECTF rect;
  ASSERT_TRUE(FPDFAnnot_GetRect(annot, &rect));
  EXPECT_NEAR(86.1971f, rect.left, 0.001f);
  FPDFPage_CloseAnnot(annot);

  annot = FPDFPage_GetAnnot(page, 1);
  ASSERT_TRUE(FPDFAnnot_GetRect(annot, &rect));
  EXPECT_NEAR(149.8127f, rect.left, 0.001f);
  FPDFPage_CloseAnnot(annot);

  annot = FPDFPage_GetAnnot(page, 2);
  ASSERT_TRUE(FPDFAnnot_GetRect(annot, &rect));
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

  // TODO(npm): VerifySavedRendering changes annot rect dimensions by 1??
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
  ASSERT_TRUE(FPDFAnnot_GetRect(annot, &rect));
  EXPECT_NEAR(86.1971f, rect.left, 0.001f);
  FPDFPage_CloseAnnot(annot);

  annot = FPDFPage_GetAnnot(new_page, 1);
  ASSERT_TRUE(FPDFAnnot_GetRect(annot, &rect));
  EXPECT_NEAR(351.8204f, rect.left, 0.001f);
  FPDFPage_CloseAnnot(annot);
  FPDF_ClosePage(new_page);
  FPDF_CloseDocument(new_doc);
}

TEST_F(FPDFAnnotEmbeddertest, AddAndModifyPath) {
#if _FX_PLATFORM_ == _FX_PLATFORM_APPLE_
  const char md5_original[] = "c35408717759562d1f8bf33d317483d2";
  const char md5_modified_path[] = "cf3cea74bd46497520ff6c4d1ea228c8";
  const char md5_two_paths[] = "e8994452fc4385337bae5522354e10ff";
  const char md5_new_annot[] = "ee5372b31fede117fc83b9384598aa25";
#else
  const char md5_original[] = "964f89bbe8911e540a465cf1a64b7f7e";
  const char md5_modified_path[] = "3f77b88ce6048e08e636c9a03921b2e5";
  const char md5_two_paths[] = "bffbf5ecd15862b9fe553c795400ff8e";
  const char md5_new_annot[] = "e020534c7eeea76be537c70d6e359a40";
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
  FS_RECTF new_rect;
  ASSERT_TRUE(FPDFAnnot_GetRect(annot, &new_rect));
  EXPECT_EQ(rect.left, new_rect.left);
  EXPECT_EQ(rect.bottom, new_rect.bottom);
  EXPECT_EQ(rect.right, new_rect.right);
  EXPECT_EQ(rect.top, new_rect.top);

  // Save the document, closing the page and document.
  FPDFPage_CloseAnnot(annot);
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDF_ClosePage(page);

  // Open the saved document.
  OpenSavedDocument();
  page = LoadSavedPage(0);
  VerifySavedRendering(page, 595, 842, md5_new_annot);

  // Check that the document has a correct count of annotations and objects.
  EXPECT_EQ(3, FPDFPage_GetAnnotCount(page));
  annot = FPDFPage_GetAnnot(page, 2);
  ASSERT_TRUE(annot);
  EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot));

  // Check that the new annotation's rectangle is as defined.
  ASSERT_TRUE(FPDFAnnot_GetRect(annot, &new_rect));
  EXPECT_EQ(rect.left, new_rect.left);
  EXPECT_EQ(rect.bottom, new_rect.bottom);
  EXPECT_EQ(rect.right, new_rect.right);
  EXPECT_EQ(rect.top, new_rect.top);

  FPDFPage_CloseAnnot(annot);
  CloseSavedPage(page);
  CloseSavedDocument();
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
#if _FX_PLATFORM_ == _FX_PLATFORM_APPLE_
  const char md5_original[] = "c35408717759562d1f8bf33d317483d2";
  const char md5_new_image[] = "ff012f5697436dfcaec25b32d1333596";
  const char md5_modified_image[] = "86cf8cb2755a7a2046a543e66d9c1e61";
#else
  const char md5_original[] = "964f89bbe8911e540a465cf1a64b7f7e";
  const char md5_new_image[] = "9ea8732dc9d579f68853f16892856208";
  const char md5_modified_image[] = "74239d2a8c55c9de1dbb9cd8781895aa";
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
  FPDFBitmap_Destroy(image_bitmap);

  // Test that the saved document renders the modified image object correctly.
  VerifySavedDocument(595, 842, md5_modified_image);
}

TEST_F(FPDFAnnotEmbeddertest, AddAndModifyText) {
#if _FX_PLATFORM_ == _FX_PLATFORM_APPLE_
  const char md5_original[] = "c35408717759562d1f8bf33d317483d2";
  const char md5_new_text[] = "e5680ed048c2cfd9a1d27212cdf41286";
  const char md5_modified_text[] = "79f5cfb0b07caaf936f65f6a7a57ce77";
#else
  const char md5_original[] = "964f89bbe8911e540a465cf1a64b7f7e";
  const char md5_new_text[] = "00b14fa2dc1c90d1b0d034e1608efef5";
  const char md5_modified_text[] = "076c8f24a09ddc0e49f7e758edead6f0";
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
  EXPECT_FALSE(FPDFAnnot_HasKey(annot, "none"));

  // Check that the string value of a non-string dictionary entry is empty.
  static constexpr char kApKey[] = "AP";
  EXPECT_TRUE(FPDFAnnot_HasKey(annot, kApKey));
  EXPECT_EQ(FPDF_OBJECT_REFERENCE, FPDFAnnot_GetValueType(annot, kApKey));
  EXPECT_EQ(2u, FPDFAnnot_GetStringValue(annot, kApKey, nullptr, 0));

  // Check that the string value of the hash is correct.
  static constexpr char kHashKey[] = "AAPL:Hash";
  EXPECT_EQ(FPDF_OBJECT_NAME, FPDFAnnot_GetValueType(annot, kHashKey));
  unsigned long len = FPDFAnnot_GetStringValue(annot, kHashKey, nullptr, 0);
  std::vector<char> buf(len);
  EXPECT_EQ(66u, FPDFAnnot_GetStringValue(annot, kHashKey, buf.data(), len));
  EXPECT_STREQ(L"395fbcb98d558681742f30683a62a2ad",
               BufferToWString(buf).c_str());

  // Check that the string value of the modified date is correct.
  static constexpr char kDateKey[] = "M";
  EXPECT_EQ(FPDF_OBJECT_NAME, FPDFAnnot_GetValueType(annot, kHashKey));
  len = FPDFAnnot_GetStringValue(annot, kDateKey, nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(44u, FPDFAnnot_GetStringValue(annot, kDateKey, buf.data(), len));
  EXPECT_STREQ(L"D:201706071721Z00'00'", BufferToWString(buf).c_str());

  // Update the date entry for the annotation.
  const wchar_t new_date[] = L"D:201706282359Z00'00'";
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text =
      GetFPDFWideString(new_date);
  EXPECT_TRUE(FPDFAnnot_SetStringValue(annot, kDateKey, text.get()));

  // Save the document, closing the page and document.
  FPDFPage_CloseAnnot(annot);
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDF_ClosePage(page);

  // Open the saved annotation.
#if _FX_PLATFORM_ == _FX_PLATFORM_APPLE_
  const char md5[] = "4d64e61c9c0f8c60ab3cc3234bb73b1c";
#else
  const char md5[] = "c96ee1f316d7f5a1b154de9f9d467f01";
#endif
  OpenSavedDocument();
  page = LoadSavedPage(0);
  VerifySavedRendering(page, 595, 842, md5);
  FPDF_ANNOTATION new_annot = FPDFPage_GetAnnot(page, 0);

  // Check that the string value of the modified date is the newly-set value.
  EXPECT_EQ(FPDF_OBJECT_STRING, FPDFAnnot_GetValueType(new_annot, kDateKey));
  len = FPDFAnnot_GetStringValue(new_annot, kDateKey, nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(44u,
            FPDFAnnot_GetStringValue(new_annot, kDateKey, buf.data(), len));
  EXPECT_STREQ(new_date, BufferToWString(buf).c_str());

  FPDFPage_CloseAnnot(new_annot);
  CloseSavedPage(page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbeddertest, GetSetAP) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);

  // Retrieve the first annotation.
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, 0);
  ASSERT_TRUE(annot);

  // Check that the string value of an AP returns the expected length.
  unsigned long normal_len =
      FPDFAnnot_GetAP(annot, FPDF_ANNOT_APPEARANCEMODE_NORMAL, nullptr, 0);
  EXPECT_EQ(73970u, normal_len);

  // Check that the string value of an AP is not returned if the buffer is too
  // small. The result buffer should be overwritten with an empty string.
  std::vector<char> buf(normal_len - 1);
  // Write L"z" in the buffer to verify it's not overwritten.
  wcscpy(reinterpret_cast<wchar_t*>(buf.data()), L"z");
  EXPECT_EQ(73970u, FPDFAnnot_GetAP(annot, FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                                    buf.data(), buf.size()));
  std::string ap = BufferToString(buf);
  EXPECT_STREQ("z", ap.c_str());

  // Check that the string value of an AP is returned through a buffer that is
  // the right size.
  buf.clear();
  buf.resize(normal_len);
  EXPECT_EQ(73970u, FPDFAnnot_GetAP(annot, FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                                    buf.data(), buf.size()));
  ap = BufferToString(buf);
  EXPECT_THAT(ap, testing::StartsWith("q Q q 7.442786 w 2 J"));
  EXPECT_THAT(ap, testing::EndsWith("c 716.5381 327.7156 l S Q Q"));

  // Check that the string value of an AP is returned through a buffer that is
  // larger than necessary.
  buf.clear();
  buf.resize(normal_len + 1);
  EXPECT_EQ(73970u, FPDFAnnot_GetAP(annot, FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                                    buf.data(), buf.size()));
  ap = BufferToString(buf);
  EXPECT_THAT(ap, testing::StartsWith("q Q q 7.442786 w 2 J"));
  EXPECT_THAT(ap, testing::EndsWith("c 716.5381 327.7156 l S Q Q"));

  // Check that getting an AP for a mode that does not have an AP returns an
  // empty string.
  unsigned long rollover_len =
      FPDFAnnot_GetAP(annot, FPDF_ANNOT_APPEARANCEMODE_ROLLOVER, nullptr, 0);
  EXPECT_EQ(2u, rollover_len);

  buf.clear();
  buf.resize(1000);
  EXPECT_EQ(2u, FPDFAnnot_GetAP(annot, FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                                buf.data(), buf.size()));
  EXPECT_STREQ("", BufferToString(buf).c_str());

  // Check that setting the AP for an invalid appearance mode fails.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> apText =
      GetFPDFWideString(L"new test ap");
  EXPECT_FALSE(FPDFAnnot_SetAP(annot, -1, apText.get()));
  EXPECT_FALSE(
      FPDFAnnot_SetAP(annot, FPDF_ANNOT_APPEARANCEMODE_COUNT, apText.get()));
  EXPECT_FALSE(FPDFAnnot_SetAP(annot, FPDF_ANNOT_APPEARANCEMODE_COUNT + 1,
                               apText.get()));

  // Set the AP correctly now.
  EXPECT_TRUE(
      FPDFAnnot_SetAP(annot, FPDF_ANNOT_APPEARANCEMODE_ROLLOVER, apText.get()));

  // Check that the new annotation value is equal to the value we just set.
  rollover_len =
      FPDFAnnot_GetAP(annot, FPDF_ANNOT_APPEARANCEMODE_ROLLOVER, nullptr, 0);
  EXPECT_EQ(24u, rollover_len);
  buf.clear();
  buf.resize(rollover_len);
  EXPECT_EQ(24u, FPDFAnnot_GetAP(annot, FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                                 buf.data(), buf.size()));
  EXPECT_STREQ(L"new test ap", BufferToWString(buf).c_str());

  // Check that the Normal AP was not touched when the Rollover AP was set.
  buf.clear();
  buf.resize(normal_len);
  EXPECT_EQ(73970u, FPDFAnnot_GetAP(annot, FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                                    buf.data(), buf.size()));
  ap = BufferToString(buf);
  EXPECT_THAT(ap, testing::StartsWith("q Q q 7.442786 w 2 J"));
  EXPECT_THAT(ap, testing::EndsWith("c 716.5381 327.7156 l S Q Q"));

  // Save the modified document, then reopen it.
  FPDFPage_CloseAnnot(annot);
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDF_ClosePage(page);

  OpenSavedDocument();
  page = LoadSavedPage(0);
  FPDF_ANNOTATION new_annot = FPDFPage_GetAnnot(page, 0);

  // Check that the new annotation value is equal to the value we set before
  // saving.
  rollover_len = FPDFAnnot_GetAP(new_annot, FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                                 nullptr, 0);
  EXPECT_EQ(24u, rollover_len);
  buf.clear();
  buf.resize(rollover_len);
  EXPECT_EQ(24u, FPDFAnnot_GetAP(new_annot, FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                                 buf.data(), buf.size()));
  EXPECT_STREQ(L"new test ap", BufferToWString(buf).c_str());

  // Close saved document.
  FPDFPage_CloseAnnot(new_annot);
  CloseSavedPage(page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbeddertest, RemoveOptionalAP) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);

  // Retrieve the first annotation.
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, 0);
  ASSERT_TRUE(annot);

  // Set Down AP. Normal AP is already set.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> apText =
      GetFPDFWideString(L"new test ap");
  EXPECT_TRUE(
      FPDFAnnot_SetAP(annot, FPDF_ANNOT_APPEARANCEMODE_DOWN, apText.get()));
  EXPECT_EQ(73970u, FPDFAnnot_GetAP(annot, FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                                    nullptr, 0));
  EXPECT_EQ(24u,
            FPDFAnnot_GetAP(annot, FPDF_ANNOT_APPEARANCEMODE_DOWN, nullptr, 0));

  // Check that setting the Down AP to null removes the Down entry but keeps
  // Normal intact.
  EXPECT_TRUE(FPDFAnnot_SetAP(annot, FPDF_ANNOT_APPEARANCEMODE_DOWN, nullptr));
  EXPECT_EQ(73970u, FPDFAnnot_GetAP(annot, FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                                    nullptr, 0));
  EXPECT_EQ(2u,
            FPDFAnnot_GetAP(annot, FPDF_ANNOT_APPEARANCEMODE_DOWN, nullptr, 0));

  FPDFPage_CloseAnnot(annot);
  FPDF_ClosePage(page);
}

TEST_F(FPDFAnnotEmbeddertest, RemoveRequiredAP) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);

  // Retrieve the first annotation.
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, 0);
  ASSERT_TRUE(annot);

  // Set Down AP. Normal AP is already set.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> apText =
      GetFPDFWideString(L"new test ap");
  EXPECT_TRUE(
      FPDFAnnot_SetAP(annot, FPDF_ANNOT_APPEARANCEMODE_DOWN, apText.get()));
  EXPECT_EQ(73970u, FPDFAnnot_GetAP(annot, FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                                    nullptr, 0));
  EXPECT_EQ(24u,
            FPDFAnnot_GetAP(annot, FPDF_ANNOT_APPEARANCEMODE_DOWN, nullptr, 0));

  // Check that setting the Normal AP to null removes the whole AP dictionary.
  EXPECT_TRUE(
      FPDFAnnot_SetAP(annot, FPDF_ANNOT_APPEARANCEMODE_NORMAL, nullptr));
  EXPECT_EQ(
      2u, FPDFAnnot_GetAP(annot, FPDF_ANNOT_APPEARANCEMODE_NORMAL, nullptr, 0));
  EXPECT_EQ(2u,
            FPDFAnnot_GetAP(annot, FPDF_ANNOT_APPEARANCEMODE_DOWN, nullptr, 0));

  FPDFPage_CloseAnnot(annot);
  FPDF_ClosePage(page);
}

TEST_F(FPDFAnnotEmbeddertest, ExtractLinkedAnnotations) {
  // Open a file with annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_square_with_ap.pdf"));
  FPDF_PAGE page = FPDF_LoadPage(document(), 0);
  ASSERT_TRUE(page);
  EXPECT_EQ(-1, FPDFPage_GetAnnotIndex(page, nullptr));

  // Retrieve the highlight annotation which has its popup defined.
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, 0);
  ASSERT_TRUE(annot);
  EXPECT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnot_GetSubtype(annot));
  EXPECT_EQ(0, FPDFPage_GetAnnotIndex(page, annot));
  static constexpr char kPopupKey[] = "Popup";
  ASSERT_TRUE(FPDFAnnot_HasKey(annot, kPopupKey));
  ASSERT_EQ(FPDF_OBJECT_REFERENCE, FPDFAnnot_GetValueType(annot, kPopupKey));

  // Retrieve and verify the popup of the highlight annotation.
  FPDF_ANNOTATION popup = FPDFAnnot_GetLinkedAnnot(annot, kPopupKey);
  ASSERT_TRUE(popup);
  EXPECT_EQ(FPDF_ANNOT_POPUP, FPDFAnnot_GetSubtype(popup));
  EXPECT_EQ(1, FPDFPage_GetAnnotIndex(page, popup));
  FS_RECTF rect;
  ASSERT_TRUE(FPDFAnnot_GetRect(popup, &rect));
  EXPECT_NEAR(612.0f, rect.left, 0.001f);
  EXPECT_NEAR(578.792, rect.bottom, 0.001f);

  // Attempting to retrieve |annot|'s "IRT"-linked annotation would fail, since
  // "IRT" is not a key in |annot|'s dictionary.
  static constexpr char kIRTKey[] = "IRT";
  ASSERT_FALSE(FPDFAnnot_HasKey(annot, kIRTKey));
  EXPECT_FALSE(FPDFAnnot_GetLinkedAnnot(annot, kIRTKey));

  // Attempting to retrieve |annot|'s parent dictionary as an annotation would
  // fail, since its parent is not an annotation.
  static constexpr char kPKey[] = "P";
  ASSERT_TRUE(FPDFAnnot_HasKey(annot, kPKey));
  EXPECT_EQ(FPDF_OBJECT_REFERENCE, FPDFAnnot_GetValueType(annot, kPKey));
  EXPECT_FALSE(FPDFAnnot_GetLinkedAnnot(annot, kPKey));

  FPDFPage_CloseAnnot(popup);
  FPDFPage_CloseAnnot(annot);
  UnloadPage(page);
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
