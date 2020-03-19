// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <cwchar>
#include <memory>
#include <string>
#include <vector>

#include "build/build_config.h"
#include "constants/annotation_common.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_system.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_annot.h"
#include "public/fpdf_edit.h"
#include "public/fpdf_formfill.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/gmock/include/gmock/gmock-matchers.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/utils/hash.h"

class FPDFAnnotEmbedderTest : public EmbedderTest {};

TEST_F(FPDFAnnotEmbedderTest, BadParams) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  EXPECT_EQ(0, FPDFPage_GetAnnotCount(nullptr));

  EXPECT_FALSE(FPDFPage_GetAnnot(nullptr, 0));
  EXPECT_FALSE(FPDFPage_GetAnnot(nullptr, -1));
  EXPECT_FALSE(FPDFPage_GetAnnot(nullptr, 1));
  EXPECT_FALSE(FPDFPage_GetAnnot(page, -1));
  EXPECT_FALSE(FPDFPage_GetAnnot(page, 1));

  EXPECT_EQ(FPDF_ANNOT_UNKNOWN, FPDFAnnot_GetSubtype(nullptr));

  EXPECT_EQ(0, FPDFAnnot_GetObjectCount(nullptr));

  EXPECT_FALSE(FPDFAnnot_GetObject(nullptr, 0));
  EXPECT_FALSE(FPDFAnnot_GetObject(nullptr, -1));
  EXPECT_FALSE(FPDFAnnot_GetObject(nullptr, 1));

  EXPECT_FALSE(FPDFAnnot_HasKey(nullptr, "foo"));

  static const wchar_t kContents[] = L"Bar";
  ScopedFPDFWideString text = GetFPDFWideString(kContents);
  EXPECT_FALSE(FPDFAnnot_SetStringValue(nullptr, "foo", text.get()));

  FPDF_WCHAR buffer[64];
  EXPECT_EQ(0u, FPDFAnnot_GetStringValue(nullptr, "foo", nullptr, 0));
  EXPECT_EQ(0u, FPDFAnnot_GetStringValue(nullptr, "foo", buffer, 0));
  EXPECT_EQ(0u,
            FPDFAnnot_GetStringValue(nullptr, "foo", buffer, sizeof(buffer)));

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, BadAnnotsEntry) {
  ASSERT_TRUE(OpenDocument("bad_annots_entry.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));
  EXPECT_FALSE(FPDFPage_GetAnnot(page, 0));

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, RenderAnnotWithOnlyRolloverAP) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_rollover_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // This annotation has a malformed appearance stream, which does not have its
  // normal appearance defined, only its rollover appearance. In this case, its
  // normal appearance should be generated, allowing the highlight annotation to
  // still display.
  ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
  CompareBitmap(bitmap.get(), 612, 792, "dc98f06da047bd8aabfa99562d2cbd1e");

  UnloadPage(page);
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_RenderMultilineMarkupAnnotWithoutAP \
  DISABLED_RenderMultilineMarkupAnnotWithoutAP
#else
#define MAYBE_RenderMultilineMarkupAnnotWithoutAP \
  RenderMultilineMarkupAnnotWithoutAP
#endif
TEST_F(FPDFAnnotEmbedderTest, MAYBE_RenderMultilineMarkupAnnotWithoutAP) {
  static const char kMd5[] = "76512832d88017668d9acc7aacd13dae";
  // Open a file with multiline markup annotations.
  ASSERT_TRUE(OpenDocument("annotation_markup_multiline_no_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
  CompareBitmap(bitmap.get(), 595, 842, kMd5);

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, ExtractHighlightLongContent) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_long_content.pdf"));
  FPDF_PAGE page = LoadPageNoEvents(0);
  ASSERT_TRUE(page);

  // Check that there is a total of 1 annotation on its first page.
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));

  // Check that the annotation is of type "highlight".
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnot_GetSubtype(annot.get()));

    // Check that the annotation color is yellow.
    unsigned int R;
    unsigned int G;
    unsigned int B;
    unsigned int A;
    ASSERT_TRUE(FPDFAnnot_GetColor(annot.get(), FPDFANNOT_COLORTYPE_Color, &R,
                                   &G, &B, &A));
    EXPECT_EQ(255u, R);
    EXPECT_EQ(255u, G);
    EXPECT_EQ(0u, B);
    EXPECT_EQ(255u, A);

    // Check that the author is correct.
    static const char kAuthorKey[] = "T";
    EXPECT_EQ(FPDF_OBJECT_STRING,
              FPDFAnnot_GetValueType(annot.get(), kAuthorKey));
    unsigned long length_bytes =
        FPDFAnnot_GetStringValue(annot.get(), kAuthorKey, nullptr, 0);
    ASSERT_EQ(28u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(28u, FPDFAnnot_GetStringValue(annot.get(), kAuthorKey, buf.data(),
                                            length_bytes));
    EXPECT_EQ(L"Jae Hyun Park", GetPlatformWString(buf.data()));

    // Check that the content is correct.
    EXPECT_EQ(
        FPDF_OBJECT_STRING,
        FPDFAnnot_GetValueType(annot.get(), pdfium::annotation::kContents));
    length_bytes = FPDFAnnot_GetStringValue(
        annot.get(), pdfium::annotation::kContents, nullptr, 0);
    ASSERT_EQ(2690u, length_bytes);
    buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(2690u, FPDFAnnot_GetStringValue(annot.get(),
                                              pdfium::annotation::kContents,
                                              buf.data(), length_bytes));
    static const wchar_t kContents[] =
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
    EXPECT_EQ(kContents, GetPlatformWString(buf.data()));

    // Check that the quadpoints are correct.
    FS_QUADPOINTSF quadpoints;
    ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot.get(), 0, &quadpoints));
    EXPECT_EQ(115.802643f, quadpoints.x1);
    EXPECT_EQ(718.913940f, quadpoints.y1);
    EXPECT_EQ(157.211182f, quadpoints.x4);
    EXPECT_EQ(706.264465f, quadpoints.y4);
  }
  UnloadPageNoEvents(page);
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_ExtractInkMultiple DISABLED_ExtractInkMultiple
#else
#define MAYBE_ExtractInkMultiple ExtractInkMultiple
#endif
TEST_F(FPDFAnnotEmbedderTest, MAYBE_ExtractInkMultiple) {
  // Open a file with three annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  FPDF_PAGE page = LoadPageNoEvents(0);
  ASSERT_TRUE(page);

  // Check that there is a total of 3 annotation on its first page.
  EXPECT_EQ(3, FPDFPage_GetAnnotCount(page));

  {
    // Check that the third annotation is of type "ink".
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_INK, FPDFAnnot_GetSubtype(annot.get()));

    // Check that the annotation color is blue with opacity.
    unsigned int R;
    unsigned int G;
    unsigned int B;
    unsigned int A;
    ASSERT_TRUE(FPDFAnnot_GetColor(annot.get(), FPDFANNOT_COLORTYPE_Color, &R,
                                   &G, &B, &A));
    EXPECT_EQ(0u, R);
    EXPECT_EQ(0u, G);
    EXPECT_EQ(255u, B);
    EXPECT_EQ(76u, A);

    // Check that there is no content.
    EXPECT_EQ(2u, FPDFAnnot_GetStringValue(
                      annot.get(), pdfium::annotation::kContents, nullptr, 0));

    // Check that the rectangle coordinates are correct.
    // Note that upon rendering, the rectangle coordinates will be adjusted.
    FS_RECTF rect;
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_EQ(351.820404f, rect.left);
    EXPECT_EQ(583.830688f, rect.bottom);
    EXPECT_EQ(475.336090f, rect.right);
    EXPECT_EQ(681.535034f, rect.top);
  }
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, "354002e1c4386d38fdde29ef8d61074a");
  }
  UnloadPageNoEvents(page);
}

TEST_F(FPDFAnnotEmbedderTest, AddIllegalSubtypeAnnotation) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_long_content.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Add an annotation with an illegal subtype.
  ASSERT_FALSE(FPDFPage_CreateAnnot(page, -1));

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, AddFirstTextAnnotation) {
  // Open a file with no annotation and load its first page.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(0, FPDFPage_GetAnnotCount(page));

  {
    // Add a text annotation to the page.
    ScopedFPDFAnnotation annot(FPDFPage_CreateAnnot(page, FPDF_ANNOT_TEXT));
    ASSERT_TRUE(annot);

    // Check that there is now 1 annotations on this page.
    EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));

    // Check that the subtype of the annotation is correct.
    EXPECT_EQ(FPDF_ANNOT_TEXT, FPDFAnnot_GetSubtype(annot.get()));
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_TEXT, FPDFAnnot_GetSubtype(annot.get()));

    // Set the color of the annotation.
    ASSERT_TRUE(FPDFAnnot_SetColor(annot.get(), FPDFANNOT_COLORTYPE_Color, 51,
                                   102, 153, 204));
    // Check that the color has been set correctly.
    unsigned int R;
    unsigned int G;
    unsigned int B;
    unsigned int A;
    ASSERT_TRUE(FPDFAnnot_GetColor(annot.get(), FPDFANNOT_COLORTYPE_Color, &R,
                                   &G, &B, &A));
    EXPECT_EQ(51u, R);
    EXPECT_EQ(102u, G);
    EXPECT_EQ(153u, B);
    EXPECT_EQ(204u, A);

    // Change the color of the annotation.
    ASSERT_TRUE(FPDFAnnot_SetColor(annot.get(), FPDFANNOT_COLORTYPE_Color, 204,
                                   153, 102, 51));
    // Check that the color has been set correctly.
    ASSERT_TRUE(FPDFAnnot_GetColor(annot.get(), FPDFANNOT_COLORTYPE_Color, &R,
                                   &G, &B, &A));
    EXPECT_EQ(204u, R);
    EXPECT_EQ(153u, G);
    EXPECT_EQ(102u, B);
    EXPECT_EQ(51u, A);

    // Set the annotation rectangle.
    FS_RECTF rect;
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_EQ(0.f, rect.left);
    EXPECT_EQ(0.f, rect.right);
    rect.left = 35;
    rect.bottom = 150;
    rect.right = 53;
    rect.top = 165;
    ASSERT_TRUE(FPDFAnnot_SetRect(annot.get(), &rect));
    // Check that the annotation rectangle has been set correctly.
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_EQ(35.f, rect.left);
    EXPECT_EQ(150.f, rect.bottom);
    EXPECT_EQ(53.f, rect.right);
    EXPECT_EQ(165.f, rect.top);

    // Set the content of the annotation.
    static const wchar_t kContents[] = L"Hello! This is a customized content.";
    ScopedFPDFWideString text = GetFPDFWideString(kContents);
    ASSERT_TRUE(FPDFAnnot_SetStringValue(
        annot.get(), pdfium::annotation::kContents, text.get()));
    // Check that the content has been set correctly.
    unsigned long length_bytes = FPDFAnnot_GetStringValue(
        annot.get(), pdfium::annotation::kContents, nullptr, 0);
    ASSERT_EQ(74u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(74u, FPDFAnnot_GetStringValue(annot.get(),
                                            pdfium::annotation::kContents,
                                            buf.data(), length_bytes));
    EXPECT_EQ(kContents, GetPlatformWString(buf.data()));
  }
  UnloadPage(page);
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_AddAndSaveUnderlineAnnotation \
  DISABLED_AddAndSaveUnderlineAnnotation
#else
#define MAYBE_AddAndSaveUnderlineAnnotation AddAndSaveUnderlineAnnotation
#endif
TEST_F(FPDFAnnotEmbedderTest, MAYBE_AddAndSaveUnderlineAnnotation) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_long_content.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Check that there is a total of one annotation on its first page, and verify
  // its quadpoints.
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));
  FS_QUADPOINTSF quadpoints;
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot.get(), 0, &quadpoints));
    EXPECT_EQ(115.802643f, quadpoints.x1);
    EXPECT_EQ(718.913940f, quadpoints.y1);
    EXPECT_EQ(157.211182f, quadpoints.x4);
    EXPECT_EQ(706.264465f, quadpoints.y4);
  }

  // Add an underline annotation to the page and set its quadpoints.
  {
    ScopedFPDFAnnotation annot(
        FPDFPage_CreateAnnot(page, FPDF_ANNOT_UNDERLINE));
    ASSERT_TRUE(annot);
    quadpoints.x1 = 140.802643f;
    quadpoints.x3 = 140.802643f;
    ASSERT_TRUE(FPDFAnnot_AppendAttachmentPoints(annot.get(), &quadpoints));
  }

  // Save the document, closing the page and document.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Open the saved document.
  static const char kMd5[] = "dba153419f67b7c0c0e3d22d3e8910d5";

  ASSERT_TRUE(OpenSavedDocument());
  page = LoadSavedPage(0);
  VerifySavedRendering(page, 612, 792, kMd5);

  // Check that the saved document has 2 annotations on the first page
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));

  {
    // Check that the second annotation is an underline annotation and verify
    // its quadpoints.
    ScopedFPDFAnnotation new_annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(new_annot);
    EXPECT_EQ(FPDF_ANNOT_UNDERLINE, FPDFAnnot_GetSubtype(new_annot.get()));
    FS_QUADPOINTSF new_quadpoints;
    ASSERT_TRUE(
        FPDFAnnot_GetAttachmentPoints(new_annot.get(), 0, &new_quadpoints));
    EXPECT_NEAR(quadpoints.x1, new_quadpoints.x1, 0.001f);
    EXPECT_NEAR(quadpoints.y1, new_quadpoints.y1, 0.001f);
    EXPECT_NEAR(quadpoints.x4, new_quadpoints.x4, 0.001f);
    EXPECT_NEAR(quadpoints.y4, new_quadpoints.y4, 0.001f);
  }

  CloseSavedPage(page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbedderTest, GetAndSetQuadPoints) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_square_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(4, FPDFPage_GetAnnotCount(page));

  // Retrieve the highlight annotation.
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, 0);
  ASSERT_TRUE(annot);
  ASSERT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnot_GetSubtype(annot));

  FS_QUADPOINTSF quadpoints;
  ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot, 0, &quadpoints));

  {
    // Verify the current one set of quadpoints.
    ASSERT_EQ(1u, FPDFAnnot_CountAttachmentPoints(annot));

    EXPECT_NEAR(72.0000f, quadpoints.x1, 0.001f);
    EXPECT_NEAR(720.792f, quadpoints.y1, 0.001f);
    EXPECT_NEAR(132.055f, quadpoints.x4, 0.001f);
    EXPECT_NEAR(704.796f, quadpoints.y4, 0.001f);
  }

  {
    // Update the quadpoints.
    FS_QUADPOINTSF new_quadpoints = quadpoints;
    new_quadpoints.y1 -= 20.f;
    new_quadpoints.y2 -= 20.f;
    new_quadpoints.y3 -= 20.f;
    new_quadpoints.y4 -= 20.f;
    ASSERT_TRUE(FPDFAnnot_SetAttachmentPoints(annot, 0, &new_quadpoints));

    // Verify added quadpoint set
    ASSERT_EQ(1u, FPDFAnnot_CountAttachmentPoints(annot));
    ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot, 0, &quadpoints));
    EXPECT_NEAR(new_quadpoints.x1, quadpoints.x1, 0.001f);
    EXPECT_NEAR(new_quadpoints.y1, quadpoints.y1, 0.001f);
    EXPECT_NEAR(new_quadpoints.x4, quadpoints.x4, 0.001f);
    EXPECT_NEAR(new_quadpoints.y4, quadpoints.y4, 0.001f);
  }

  {
    // Append a new set of quadpoints.
    FS_QUADPOINTSF new_quadpoints = quadpoints;
    new_quadpoints.y1 += 20.f;
    new_quadpoints.y2 += 20.f;
    new_quadpoints.y3 += 20.f;
    new_quadpoints.y4 += 20.f;
    ASSERT_TRUE(FPDFAnnot_AppendAttachmentPoints(annot, &new_quadpoints));

    // Verify added quadpoint set
    ASSERT_EQ(2u, FPDFAnnot_CountAttachmentPoints(annot));
    ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot, 1, &quadpoints));
    EXPECT_NEAR(new_quadpoints.x1, quadpoints.x1, 0.001f);
    EXPECT_NEAR(new_quadpoints.y1, quadpoints.y1, 0.001f);
    EXPECT_NEAR(new_quadpoints.x4, quadpoints.x4, 0.001f);
    EXPECT_NEAR(new_quadpoints.y4, quadpoints.y4, 0.001f);
  }

  {
    // Setting and getting quadpoints at out-of-bound index should fail
    EXPECT_FALSE(FPDFAnnot_SetAttachmentPoints(annot, 300000, &quadpoints));
    EXPECT_FALSE(FPDFAnnot_GetAttachmentPoints(annot, 300000, &quadpoints));
  }

  FPDFPage_CloseAnnot(annot);

  // Retrieve the square annotation
  FPDF_ANNOTATION squareAnnot = FPDFPage_GetAnnot(page, 2);

  {
    // Check that attempting to set its quadpoints would fail
    ASSERT_TRUE(squareAnnot);
    EXPECT_EQ(FPDF_ANNOT_SQUARE, FPDFAnnot_GetSubtype(squareAnnot));
    EXPECT_EQ(0u, FPDFAnnot_CountAttachmentPoints(squareAnnot));
    EXPECT_FALSE(FPDFAnnot_SetAttachmentPoints(squareAnnot, 0, &quadpoints));
  }

  FPDFPage_CloseAnnot(squareAnnot);
  UnloadPage(page);
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_ModifyRectQuadpointsWithAP DISABLED_ModifyRectQuadpointsWithAP
#else
#define MAYBE_ModifyRectQuadpointsWithAP ModifyRectQuadpointsWithAP
#endif
TEST_F(FPDFAnnotEmbedderTest, MAYBE_ModifyRectQuadpointsWithAP) {
#if defined(OS_MACOSX)
  static const char kMd5Original[] = "fc59468d154f397fd298c69f47ef565a";
  static const char kMd5ModifiedHighlight[] =
      "e64bf648f6e9354d1f3eedb47a2c9498";
  static const char kMd5ModifiedSquare[] = "a66591662c8e7ad3c6059952e234bebf";
#elif defined(OS_WIN)
  static const char kMd5Original[] = "0e27376094f11490f74c65f3dc3a42c5";
  static const char kMd5ModifiedHighlight[] =
      "66f3caef3a7d488a4fa1ad37fc06310e";
  static const char kMd5ModifiedSquare[] = "a456dad0bc6801ee2d6408a4394af563";
#else
  static const char kMd5Original[] = "0e27376094f11490f74c65f3dc3a42c5";
  static const char kMd5ModifiedHighlight[] =
      "66f3caef3a7d488a4fa1ad37fc06310e";
  static const char kMd5ModifiedSquare[] = "a456dad0bc6801ee2d6408a4394af563";
#endif

  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_square_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(4, FPDFPage_GetAnnotCount(page));

  // Check that the original file renders correctly.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, kMd5Original);
  }

  FS_RECTF rect;
  FS_RECTF new_rect;

  // Retrieve the highlight annotation which has its AP stream already defined.
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnot_GetSubtype(annot.get()));

    // Check that color cannot be set when an AP stream is defined already.
    EXPECT_FALSE(FPDFAnnot_SetColor(annot.get(), FPDFANNOT_COLORTYPE_Color, 51,
                                    102, 153, 204));

    // Verify its attachment points.
    FS_QUADPOINTSF quadpoints;
    ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot.get(), 0, &quadpoints));
    EXPECT_NEAR(72.0000f, quadpoints.x1, 0.001f);
    EXPECT_NEAR(720.792f, quadpoints.y1, 0.001f);
    EXPECT_NEAR(132.055f, quadpoints.x4, 0.001f);
    EXPECT_NEAR(704.796f, quadpoints.y4, 0.001f);

    // Check that updating the attachment points would succeed.
    quadpoints.x1 -= 50.f;
    quadpoints.x2 -= 50.f;
    quadpoints.x3 -= 50.f;
    quadpoints.x4 -= 50.f;
    ASSERT_TRUE(FPDFAnnot_SetAttachmentPoints(annot.get(), 0, &quadpoints));
    FS_QUADPOINTSF new_quadpoints;
    ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot.get(), 0, &new_quadpoints));
    EXPECT_EQ(quadpoints.x1, new_quadpoints.x1);
    EXPECT_EQ(quadpoints.y1, new_quadpoints.y1);
    EXPECT_EQ(quadpoints.x4, new_quadpoints.x4);
    EXPECT_EQ(quadpoints.y4, new_quadpoints.y4);

    // Check that updating quadpoints does not change the annotation's position.
    {
      ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 612, 792, kMd5Original);
    }

    // Verify its annotation rectangle.
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_NEAR(67.7299f, rect.left, 0.001f);
    EXPECT_NEAR(704.296f, rect.bottom, 0.001f);
    EXPECT_NEAR(136.325f, rect.right, 0.001f);
    EXPECT_NEAR(721.292f, rect.top, 0.001f);

    // Check that updating the rectangle would succeed.
    rect.left -= 60.f;
    rect.right -= 60.f;
    ASSERT_TRUE(FPDFAnnot_SetRect(annot.get(), &rect));
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &new_rect));
    EXPECT_EQ(rect.right, new_rect.right);
  }

  // Check that updating the rectangle changes the annotation's position.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, kMd5ModifiedHighlight);
  }

  {
    // Retrieve the square annotation which has its AP stream already defined.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_SQUARE, FPDFAnnot_GetSubtype(annot.get()));

    // Check that updating the rectangle would succeed.
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    rect.left += 70.f;
    rect.right += 70.f;
    ASSERT_TRUE(FPDFAnnot_SetRect(annot.get(), &rect));
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &new_rect));
    EXPECT_EQ(rect.right, new_rect.right);

    // Check that updating the rectangle changes the square annotation's
    // position.
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, kMd5ModifiedSquare);
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, CountAttachmentPoints) {
  // Open a file with multiline markup annotations.
  ASSERT_TRUE(OpenDocument("annotation_markup_multiline_no_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // This is a three line annotation.
    EXPECT_EQ(3u, FPDFAnnot_CountAttachmentPoints(annot.get()));
  }
  UnloadPage(page);

  // null annotation should return 0
  EXPECT_EQ(0u, FPDFAnnot_CountAttachmentPoints(nullptr));
}

TEST_F(FPDFAnnotEmbedderTest, RemoveAnnotation) {
  // Open a file with 3 annotations on its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  FPDF_PAGE page = LoadPageNoEvents(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(3, FPDFPage_GetAnnotCount(page));

  FS_RECTF rect;

  // Check that the annotations have the expected rectangle coordinates.
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_NEAR(86.1971f, rect.left, 0.001f);
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_NEAR(149.8127f, rect.left, 0.001f);
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_NEAR(351.8204f, rect.left, 0.001f);
  }

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
  UnloadPageNoEvents(page);

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

  // Check that the remaining 2 annotations are the original 1st and 3rd ones
  // by verifying their rectangle coordinates.
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(new_page, 0));
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_NEAR(86.1971f, rect.left, 0.001f);
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(new_page, 1));
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_NEAR(351.8204f, rect.left, 0.001f);
  }
  FPDF_ClosePage(new_page);
  FPDF_CloseDocument(new_doc);
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_AddAndModifyPath DISABLED_AddAndModifyPath
#else
#define MAYBE_AddAndModifyPath AddAndModifyPath
#endif
TEST_F(FPDFAnnotEmbedderTest, MAYBE_AddAndModifyPath) {
#if defined(OS_MACOSX)
  static const char kMd5Original[] = "80d7b6cc7b13a78d77a6151bc846e80b";
  static const char kMd5ModifiedPath[] = "8cfae6d547fc5d6702f5f1ac631beb5e";
  static const char kMd5TwoPaths[] = "9677e4892bb02950d3e4dbe74470578f";
  static const char kMd5NewAnnot[] = "e8ebddac4db8c0a4b556ddf79aa1a26d";
#elif defined(OS_WIN)
  static const char kMd5Original[] = "6aa001a77ec05d0f1b0d1d22e28744d4";
  static const char kMd5ModifiedPath[] = "a7a8d675a6ddbcbdfecee65a33ba19e1";
  static const char kMd5TwoPaths[] = "7c0bdd4552329704c47a7cce47edbbd6";
  static const char kMd5NewAnnot[] = "3c48d492b4f62941fed0fb62f729f31e";
#else
  static const char kMd5Original[] = "b42cef463483e668eaf4055a65e4f1f5";
  static const char kMd5ModifiedPath[] = "6ff77d6d1fec4ea571fabe0c7a19b517";
  static const char kMd5TwoPaths[] = "ca37ad549e74ac5b359a055708f3e7b6";
  static const char kMd5NewAnnot[] = "0d7a0e33fbf41ff7fa5d732ab2c5edff";
#endif

  // Open a file with two annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));

  // Check that the page renders correctly.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, kMd5Original);
  }

  {
    // Retrieve the stamp annotation which has its AP stream already defined.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Check that this annotation has one path object and retrieve it.
    EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot.get()));
    ASSERT_EQ(32, FPDFPage_CountObjects(page));
    FPDF_PAGEOBJECT path = FPDFAnnot_GetObject(annot.get(), 1);
    EXPECT_FALSE(path);
    path = FPDFAnnot_GetObject(annot.get(), 0);
    EXPECT_EQ(FPDF_PAGEOBJ_PATH, FPDFPageObj_GetType(path));
    EXPECT_TRUE(path);

    // Modify the color of the path object.
    EXPECT_TRUE(FPDFPageObj_SetStrokeColor(path, 0, 0, 0, 255));
    EXPECT_TRUE(FPDFAnnot_UpdateObject(annot.get(), path));

    // Check that the page with the modified annotation renders correctly.
    {
      ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 595, 842, kMd5ModifiedPath);
    }

    // Add a second path object to the same annotation.
    FPDF_PAGEOBJECT dot = FPDFPageObj_CreateNewPath(7, 84);
    EXPECT_TRUE(FPDFPath_BezierTo(dot, 9, 86, 10, 87, 11, 88));
    EXPECT_TRUE(FPDFPageObj_SetStrokeColor(dot, 255, 0, 0, 100));
    EXPECT_TRUE(FPDFPageObj_SetStrokeWidth(dot, 14));
    EXPECT_TRUE(FPDFPath_SetDrawMode(dot, 0, 1));
    EXPECT_TRUE(FPDFAnnot_AppendObject(annot.get(), dot));
    EXPECT_EQ(2, FPDFAnnot_GetObjectCount(annot.get()));

    // The object is in the annontation, not in the page, so the page object
    // array should not change.
    ASSERT_EQ(32, FPDFPage_CountObjects(page));

    // Check that the page with an annotation with two paths renders correctly.
    {
      ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 595, 842, kMd5TwoPaths);
    }

    // Delete the newly added path object.
    EXPECT_TRUE(FPDFAnnot_RemoveObject(annot.get(), 1));
    EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot.get()));
    ASSERT_EQ(32, FPDFPage_CountObjects(page));
  }

  // Check that the page renders the same as before.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, kMd5ModifiedPath);
  }

  FS_RECTF rect;

  {
    // Create another stamp annotation and set its annotation rectangle.
    ScopedFPDFAnnotation annot(FPDFPage_CreateAnnot(page, FPDF_ANNOT_STAMP));
    ASSERT_TRUE(annot);
    rect.left = 200.f;
    rect.bottom = 400.f;
    rect.right = 500.f;
    rect.top = 600.f;
    EXPECT_TRUE(FPDFAnnot_SetRect(annot.get(), &rect));

    // Add a new path to the annotation.
    FPDF_PAGEOBJECT check = FPDFPageObj_CreateNewPath(200, 500);
    EXPECT_TRUE(FPDFPath_LineTo(check, 300, 400));
    EXPECT_TRUE(FPDFPath_LineTo(check, 500, 600));
    EXPECT_TRUE(FPDFPath_MoveTo(check, 350, 550));
    EXPECT_TRUE(FPDFPath_LineTo(check, 450, 450));
    EXPECT_TRUE(FPDFPageObj_SetStrokeColor(check, 0, 255, 255, 180));
    EXPECT_TRUE(FPDFPageObj_SetStrokeWidth(check, 8.35f));
    EXPECT_TRUE(FPDFPath_SetDrawMode(check, 0, 1));
    EXPECT_TRUE(FPDFAnnot_AppendObject(annot.get(), check));
    EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot.get()));

    // Check that the annotation's bounding box came from its rectangle.
    FS_RECTF new_rect;
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &new_rect));
    EXPECT_EQ(rect.left, new_rect.left);
    EXPECT_EQ(rect.bottom, new_rect.bottom);
    EXPECT_EQ(rect.right, new_rect.right);
    EXPECT_EQ(rect.top, new_rect.top);
  }

  // Save the document, closing the page and document.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Open the saved document.
  ASSERT_TRUE(OpenSavedDocument());
  page = LoadSavedPage(0);
  VerifySavedRendering(page, 595, 842, kMd5NewAnnot);

  // Check that the document has a correct count of annotations and objects.
  EXPECT_EQ(3, FPDFPage_GetAnnotCount(page));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);
    EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot.get()));

    // Check that the new annotation's rectangle is as defined.
    FS_RECTF new_rect;
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &new_rect));
    EXPECT_EQ(rect.left, new_rect.left);
    EXPECT_EQ(rect.bottom, new_rect.bottom);
    EXPECT_EQ(rect.right, new_rect.right);
    EXPECT_EQ(rect.top, new_rect.top);
  }

  CloseSavedPage(page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbedderTest, ModifyAnnotationFlags) {
  // Open a file with an annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_rollover_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Check that the page renders correctly.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, "dc98f06da047bd8aabfa99562d2cbd1e");
  }

  {
    // Retrieve the annotation.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Check that the original flag values are as expected.
    int flags = FPDFAnnot_GetFlags(annot.get());
    EXPECT_FALSE(flags & FPDF_ANNOT_FLAG_HIDDEN);
    EXPECT_TRUE(flags & FPDF_ANNOT_FLAG_PRINT);

    // Set the HIDDEN flag.
    flags |= FPDF_ANNOT_FLAG_HIDDEN;
    EXPECT_TRUE(FPDFAnnot_SetFlags(annot.get(), flags));
    flags = FPDFAnnot_GetFlags(annot.get());
    EXPECT_TRUE(flags & FPDF_ANNOT_FLAG_HIDDEN);
    EXPECT_TRUE(flags & FPDF_ANNOT_FLAG_PRINT);

    // Check that the page renders correctly without rendering the annotation.
    {
      ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 612, 792, "1940568c9ba33bac5d0b1ee9558c76b3");
    }

    // Unset the HIDDEN flag.
    EXPECT_TRUE(FPDFAnnot_SetFlags(annot.get(), FPDF_ANNOT_FLAG_NONE));
    EXPECT_FALSE(FPDFAnnot_GetFlags(annot.get()));
    flags &= ~FPDF_ANNOT_FLAG_HIDDEN;
    EXPECT_TRUE(FPDFAnnot_SetFlags(annot.get(), flags));
    flags = FPDFAnnot_GetFlags(annot.get());
    EXPECT_FALSE(flags & FPDF_ANNOT_FLAG_HIDDEN);
    EXPECT_TRUE(flags & FPDF_ANNOT_FLAG_PRINT);

    // Check that the page renders correctly as before.
    {
      ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 612, 792, "dc98f06da047bd8aabfa99562d2cbd1e");
    }
  }

  UnloadPage(page);
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_AddAndModifyImage DISABLED_AddAndModifyImage
#else
#define MAYBE_AddAndModifyImage AddAndModifyImage
#endif
TEST_F(FPDFAnnotEmbedderTest, MAYBE_AddAndModifyImage) {
#if defined(OS_MACOSX)
  static const char kMd5Original[] = "80d7b6cc7b13a78d77a6151bc846e80b";
  static const char kMd5NewImage[] = "dd18709d90c245a12ce0b8c4d092bea9";
  static const char kMd5ModifiedImage[] = "8d6f478ff8c7e67d49b253f1af587a99";
#elif defined(OS_WIN)
  static const char kMd5Original[] = "6aa001a77ec05d0f1b0d1d22e28744d4";
  static const char kMd5NewImage[] = "3d77d06a971bcb9fb54db082f1082c8b";
  static const char kMd5ModifiedImage[] = "dc4f4afc26c345418330d31c065020e1";
#else
  static const char kMd5Original[] = "b42cef463483e668eaf4055a65e4f1f5";
  static const char kMd5NewImage[] = "528e6243dc29d54f36b61e0d3287d935";
  static const char kMd5ModifiedImage[] = "6d9e59f3e57a1ff82fb258356b7eb731";
#endif

  // Open a file with two annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));

  // Check that the page renders correctly.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, kMd5Original);
  }

  constexpr int kBitmapSize = 200;
  FPDF_BITMAP image_bitmap;

  {
    // Create a stamp annotation and set its annotation rectangle.
    ScopedFPDFAnnotation annot(FPDFPage_CreateAnnot(page, FPDF_ANNOT_STAMP));
    ASSERT_TRUE(annot);
    FS_RECTF rect;
    rect.left = 200.f;
    rect.bottom = 600.f;
    rect.right = 400.f;
    rect.top = 800.f;
    EXPECT_TRUE(FPDFAnnot_SetRect(annot.get(), &rect));

    // Add a solid-color translucent image object to the new annotation.
    image_bitmap = FPDFBitmap_Create(kBitmapSize, kBitmapSize, 1);
    FPDFBitmap_FillRect(image_bitmap, 0, 0, kBitmapSize, kBitmapSize,
                        0xeeeecccc);
    EXPECT_EQ(kBitmapSize, FPDFBitmap_GetWidth(image_bitmap));
    EXPECT_EQ(kBitmapSize, FPDFBitmap_GetHeight(image_bitmap));
    FPDF_PAGEOBJECT image_object = FPDFPageObj_NewImageObj(document());
    ASSERT_TRUE(FPDFImageObj_SetBitmap(&page, 0, image_object, image_bitmap));
    ASSERT_TRUE(FPDFImageObj_SetMatrix(image_object, kBitmapSize, 0, 0,
                                       kBitmapSize, 0, 0));
    FPDFPageObj_Transform(image_object, 1, 0, 0, 1, 200, 600);
    EXPECT_TRUE(FPDFAnnot_AppendObject(annot.get(), image_object));
  }

  // Check that the page renders correctly with the new image object.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, kMd5NewImage);
  }

  {
    // Retrieve the newly added stamp annotation and its image object.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);
    EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot.get()));
    FPDF_PAGEOBJECT image_object = FPDFAnnot_GetObject(annot.get(), 0);
    EXPECT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(image_object));

    // Modify the image in the new annotation.
    FPDFBitmap_FillRect(image_bitmap, 0, 0, kBitmapSize, kBitmapSize,
                        0xff000000);
    ASSERT_TRUE(FPDFImageObj_SetBitmap(&page, 0, image_object, image_bitmap));
    EXPECT_TRUE(FPDFAnnot_UpdateObject(annot.get(), image_object));
  }

  // Save the document, closing the page and document.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);
  FPDFBitmap_Destroy(image_bitmap);

  // Test that the saved document renders the modified image object correctly.
  VerifySavedDocument(595, 842, kMd5ModifiedImage);
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_AddAndModifyText DISABLED_AddAndModifyText
#else
#define MAYBE_AddAndModifyText AddAndModifyText
#endif
TEST_F(FPDFAnnotEmbedderTest, MAYBE_AddAndModifyText) {
#if defined(OS_MACOSX)
  static const char kMd5Original[] = "80d7b6cc7b13a78d77a6151bc846e80b";
  static const char kMd5NewText[] = "e657266260b88c964938efe6c9b292da";
  static const char kMd5ModifiedText[] = "7accdf2bac64463101783221f53d3188";
#elif defined(OS_WIN)
  static const char kMd5Original[] = "6aa001a77ec05d0f1b0d1d22e28744d4";
  static const char kMd5NewText[] = "204cc01749a70b8afc246a4ca33c7eb6";
  static const char kMd5ModifiedText[] = "641261a45e8dfd68c89b80bfd237660d";
#else
  static const char kMd5Original[] = "b42cef463483e668eaf4055a65e4f1f5";
  static const char kMd5NewText[] = "00197ad6206f763febad5719e5935306";
  static const char kMd5ModifiedText[] = "85853bc0aaa5a4e3af04e58b9cbfff23";
#endif

  // Open a file with two annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));

  // Check that the page renders correctly.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, kMd5Original);
  }

  {
    // Create a stamp annotation and set its annotation rectangle.
    ScopedFPDFAnnotation annot(FPDFPage_CreateAnnot(page, FPDF_ANNOT_STAMP));
    ASSERT_TRUE(annot);
    FS_RECTF rect;
    rect.left = 200.f;
    rect.bottom = 550.f;
    rect.right = 450.f;
    rect.top = 650.f;
    EXPECT_TRUE(FPDFAnnot_SetRect(annot.get(), &rect));

    // Add a translucent text object to the new annotation.
    FPDF_PAGEOBJECT text_object =
        FPDFPageObj_NewTextObj(document(), "Arial", 12.0f);
    EXPECT_TRUE(text_object);
    ScopedFPDFWideString text =
        GetFPDFWideString(L"I'm a translucent text laying on other text.");
    EXPECT_TRUE(FPDFText_SetText(text_object, text.get()));
    EXPECT_TRUE(FPDFPageObj_SetFillColor(text_object, 0, 0, 255, 150));
    FPDFPageObj_Transform(text_object, 1, 0, 0, 1, 200, 600);
    EXPECT_TRUE(FPDFAnnot_AppendObject(annot.get(), text_object));
  }

  // Check that the page renders correctly with the new text object.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, kMd5NewText);
  }

  {
    // Retrieve the newly added stamp annotation and its text object.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);
    EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot.get()));
    FPDF_PAGEOBJECT text_object = FPDFAnnot_GetObject(annot.get(), 0);
    EXPECT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(text_object));

    // Modify the text in the new annotation.
    ScopedFPDFWideString new_text = GetFPDFWideString(L"New text!");
    EXPECT_TRUE(FPDFText_SetText(text_object, new_text.get()));
    EXPECT_TRUE(FPDFAnnot_UpdateObject(annot.get(), text_object));
  }

  // Check that the page renders correctly with the modified text object.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, kMd5ModifiedText);
  }

  // Remove the new annotation, and check that the page renders as before.
  EXPECT_TRUE(FPDFPage_RemoveAnnot(page, 2));
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, kMd5Original);
  }

  UnloadPage(page);
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_GetSetStringValue DISABLED_GetSetStringValue
#else
#define MAYBE_GetSetStringValue GetSetStringValue
#endif
TEST_F(FPDFAnnotEmbedderTest, MAYBE_GetSetStringValue) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  static const wchar_t kNewDate[] = L"D:201706282359Z00'00'";

  {
    // Retrieve the first annotation.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Check that a non-existent key does not exist.
    EXPECT_FALSE(FPDFAnnot_HasKey(annot.get(), "none"));

    // Check that the string value of a non-string dictionary entry is empty.
    EXPECT_TRUE(FPDFAnnot_HasKey(annot.get(), pdfium::annotation::kAP));
    EXPECT_EQ(FPDF_OBJECT_REFERENCE,
              FPDFAnnot_GetValueType(annot.get(), pdfium::annotation::kAP));
    EXPECT_EQ(2u, FPDFAnnot_GetStringValue(annot.get(), pdfium::annotation::kAP,
                                           nullptr, 0));

    // Check that the string value of the hash is correct.
    static const char kHashKey[] = "AAPL:Hash";
    EXPECT_EQ(FPDF_OBJECT_NAME, FPDFAnnot_GetValueType(annot.get(), kHashKey));
    unsigned long length_bytes =
        FPDFAnnot_GetStringValue(annot.get(), kHashKey, nullptr, 0);
    ASSERT_EQ(66u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(66u, FPDFAnnot_GetStringValue(annot.get(), kHashKey, buf.data(),
                                            length_bytes));
    EXPECT_EQ(L"395fbcb98d558681742f30683a62a2ad",
              GetPlatformWString(buf.data()));

    // Check that the string value of the modified date is correct.
    EXPECT_EQ(FPDF_OBJECT_NAME, FPDFAnnot_GetValueType(annot.get(), kHashKey));
    length_bytes = FPDFAnnot_GetStringValue(annot.get(), pdfium::annotation::kM,
                                            nullptr, 0);
    ASSERT_EQ(44u, length_bytes);
    buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(44u, FPDFAnnot_GetStringValue(annot.get(), pdfium::annotation::kM,
                                            buf.data(), length_bytes));
    EXPECT_EQ(L"D:201706071721Z00'00'", GetPlatformWString(buf.data()));

    // Update the date entry for the annotation.
    ScopedFPDFWideString text = GetFPDFWideString(kNewDate);
    EXPECT_TRUE(FPDFAnnot_SetStringValue(annot.get(), pdfium::annotation::kM,
                                         text.get()));
  }

  // Save the document, closing the page and document.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

#if defined(OS_MACOSX)
  static const char kMd5[] = "5e7e185b386ad21ca83b0287268c50fb";
#elif defined(OS_WIN)
  static const char kMd5[] = "20b612ebd46babcb44c48c903e2c5a48";
#else
  static const char kMd5[] = "1d7bea2042c6fea0558ff2aef05811b5";
#endif

  // Open the saved annotation.
  ASSERT_TRUE(OpenSavedDocument());
  page = LoadSavedPage(0);
  VerifySavedRendering(page, 595, 842, kMd5);
  {
    ScopedFPDFAnnotation new_annot(FPDFPage_GetAnnot(page, 0));

    // Check that the string value of the modified date is the newly-set value.
    EXPECT_EQ(FPDF_OBJECT_STRING,
              FPDFAnnot_GetValueType(new_annot.get(), pdfium::annotation::kM));
    unsigned long length_bytes = FPDFAnnot_GetStringValue(
        new_annot.get(), pdfium::annotation::kM, nullptr, 0);
    ASSERT_EQ(44u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(44u,
              FPDFAnnot_GetStringValue(new_annot.get(), pdfium::annotation::kM,
                                       buf.data(), length_bytes));
    EXPECT_EQ(kNewDate, GetPlatformWString(buf.data()));
  }

  CloseSavedPage(page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbedderTest, GetNumberValue) {
  // Open a file with four text annotations and load its first page.
  ASSERT_TRUE(OpenDocument("text_form_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  {
    // First two annotations do not have "MaxLen" attribute.
    for (int i = 0; i < 2; i++) {
      ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, i));
      ASSERT_TRUE(annot);

      // Verify that no "MaxLen" key present.
      EXPECT_FALSE(FPDFAnnot_HasKey(annot.get(), "MaxLen"));

      float value;
      EXPECT_FALSE(FPDFAnnot_GetNumberValue(annot.get(), "MaxLen", &value));
    }

    // Annotation in index 2 has "MaxLen" of 10.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);

    // Verify that "MaxLen" key present.
    EXPECT_TRUE(FPDFAnnot_HasKey(annot.get(), "MaxLen"));

    float value;
    EXPECT_TRUE(FPDFAnnot_GetNumberValue(annot.get(), "MaxLen", &value));
    EXPECT_FLOAT_EQ(10.0f, value);

    // Check bad inputs.
    EXPECT_FALSE(FPDFAnnot_GetNumberValue(nullptr, "MaxLen", &value));
    EXPECT_FALSE(FPDFAnnot_GetNumberValue(annot.get(), nullptr, &value));
    EXPECT_FALSE(FPDFAnnot_GetNumberValue(annot.get(), "MaxLen", nullptr));
    // Ask for key that exists but is not a number.
    EXPECT_FALSE(FPDFAnnot_GetNumberValue(annot.get(), "V", &value));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetSetAP) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    static const char kMd5NormalAP[] = "be903df0343fd774fadab9c8900cdf4a";
    static constexpr size_t kExpectNormalAPLength = 73970;

    // Retrieve the first annotation.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Check that the string value of an AP returns the expected length.
    unsigned long normal_length_bytes = FPDFAnnot_GetAP(
        annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL, nullptr, 0);
    ASSERT_EQ(kExpectNormalAPLength, normal_length_bytes);

    // Check that the string value of an AP is not returned if the buffer is too
    // small. The result buffer should be overwritten with an empty string.
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(normal_length_bytes);
    // Write in the buffer to verify it's not overwritten.
    memcpy(buf.data(), "abcdefgh", 8);
    EXPECT_EQ(kExpectNormalAPLength,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              buf.data(), normal_length_bytes - 1));
    EXPECT_EQ(0, memcmp(buf.data(), "abcdefgh", 8));

    // Check that the string value of an AP is returned through a buffer that is
    // the right size.
    EXPECT_EQ(kExpectNormalAPLength,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              buf.data(), normal_length_bytes));
    EXPECT_EQ(kMd5NormalAP,
              GenerateMD5Base16(reinterpret_cast<uint8_t*>(buf.data()),
                                normal_length_bytes));

    // Check that the string value of an AP is returned through a buffer that is
    // larger than necessary.
    buf = GetFPDFWideStringBuffer(normal_length_bytes + 2);
    EXPECT_EQ(kExpectNormalAPLength,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              buf.data(), normal_length_bytes + 2));
    EXPECT_EQ(kMd5NormalAP,
              GenerateMD5Base16(reinterpret_cast<uint8_t*>(buf.data()),
                                normal_length_bytes));

    // Check that getting an AP for a mode that does not have an AP returns an
    // empty string.
    unsigned long rollover_length_bytes = FPDFAnnot_GetAP(
        annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER, nullptr, 0);
    ASSERT_EQ(2u, rollover_length_bytes);

    buf = GetFPDFWideStringBuffer(1000);
    EXPECT_EQ(2u,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                              buf.data(), 1000));
    EXPECT_EQ(L"", GetPlatformWString(buf.data()));

    // Check that setting the AP for an invalid appearance mode fails.
    ScopedFPDFWideString ap_text = GetFPDFWideString(L"new test ap");
    EXPECT_FALSE(FPDFAnnot_SetAP(annot.get(), -1, ap_text.get()));
    EXPECT_FALSE(FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_COUNT,
                                 ap_text.get()));
    EXPECT_FALSE(FPDFAnnot_SetAP(
        annot.get(), FPDF_ANNOT_APPEARANCEMODE_COUNT + 1, ap_text.get()));

    // Set the AP correctly now.
    EXPECT_TRUE(FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                                ap_text.get()));

    // Check that the new annotation value is equal to the value we just set.
    rollover_length_bytes = FPDFAnnot_GetAP(
        annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER, nullptr, 0);
    ASSERT_EQ(24u, rollover_length_bytes);
    buf = GetFPDFWideStringBuffer(rollover_length_bytes);
    EXPECT_EQ(24u,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                              buf.data(), rollover_length_bytes));
    EXPECT_EQ(L"new test ap", GetPlatformWString(buf.data()));

    // Check that the Normal AP was not touched when the Rollover AP was set.
    buf = GetFPDFWideStringBuffer(normal_length_bytes);
    EXPECT_EQ(kExpectNormalAPLength,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              buf.data(), normal_length_bytes));
    EXPECT_EQ(kMd5NormalAP,
              GenerateMD5Base16(reinterpret_cast<uint8_t*>(buf.data()),
                                normal_length_bytes));
  }

  // Save the modified document, then reopen it.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  ASSERT_TRUE(OpenSavedDocument());
  page = LoadSavedPage(0);
  {
    ScopedFPDFAnnotation new_annot(FPDFPage_GetAnnot(page, 0));

    // Check that the new annotation value is equal to the value we set before
    // saving.
    unsigned long rollover_length_bytes = FPDFAnnot_GetAP(
        new_annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER, nullptr, 0);
    ASSERT_EQ(24u, rollover_length_bytes);
    std::vector<FPDF_WCHAR> buf =
        GetFPDFWideStringBuffer(rollover_length_bytes);
    EXPECT_EQ(24u, FPDFAnnot_GetAP(new_annot.get(),
                                   FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                                   buf.data(), rollover_length_bytes));
    EXPECT_EQ(L"new test ap", GetPlatformWString(buf.data()));
  }

  // Close saved document.
  CloseSavedPage(page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbedderTest, RemoveOptionalAP) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve the first annotation.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Set Down AP. Normal AP is already set.
    ScopedFPDFWideString ap_text = GetFPDFWideString(L"new test ap");
    EXPECT_TRUE(FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_DOWN,
                                ap_text.get()));
    EXPECT_EQ(73970u,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              nullptr, 0));
    EXPECT_EQ(24u, FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_DOWN,
                                   nullptr, 0));

    // Check that setting the Down AP to null removes the Down entry but keeps
    // Normal intact.
    EXPECT_TRUE(
        FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_DOWN, nullptr));
    EXPECT_EQ(73970u,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              nullptr, 0));
    EXPECT_EQ(2u, FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_DOWN,
                                  nullptr, 0));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, RemoveRequiredAP) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve the first annotation.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Set Down AP. Normal AP is already set.
    ScopedFPDFWideString ap_text = GetFPDFWideString(L"new test ap");
    EXPECT_TRUE(FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_DOWN,
                                ap_text.get()));
    EXPECT_EQ(73970u,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              nullptr, 0));
    EXPECT_EQ(24u, FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_DOWN,
                                   nullptr, 0));

    // Check that setting the Normal AP to null removes the whole AP dictionary.
    EXPECT_TRUE(FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                                nullptr));
    EXPECT_EQ(2u, FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                                  nullptr, 0));
    EXPECT_EQ(2u, FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_DOWN,
                                  nullptr, 0));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, ExtractLinkedAnnotations) {
  // Open a file with annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_square_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(-1, FPDFPage_GetAnnotIndex(page, nullptr));

  {
    // Retrieve the highlight annotation which has its popup defined.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnot_GetSubtype(annot.get()));
    EXPECT_EQ(0, FPDFPage_GetAnnotIndex(page, annot.get()));
    static const char kPopupKey[] = "Popup";
    ASSERT_TRUE(FPDFAnnot_HasKey(annot.get(), kPopupKey));
    ASSERT_EQ(FPDF_OBJECT_REFERENCE,
              FPDFAnnot_GetValueType(annot.get(), kPopupKey));

    // Retrieve and verify the popup of the highlight annotation.
    ScopedFPDFAnnotation popup(
        FPDFAnnot_GetLinkedAnnot(annot.get(), kPopupKey));
    ASSERT_TRUE(popup);
    EXPECT_EQ(FPDF_ANNOT_POPUP, FPDFAnnot_GetSubtype(popup.get()));
    EXPECT_EQ(1, FPDFPage_GetAnnotIndex(page, popup.get()));
    FS_RECTF rect;
    ASSERT_TRUE(FPDFAnnot_GetRect(popup.get(), &rect));
    EXPECT_NEAR(612.0f, rect.left, 0.001f);
    EXPECT_NEAR(578.792, rect.bottom, 0.001f);

    // Attempting to retrieve |annot|'s "IRT"-linked annotation would fail,
    // since "IRT" is not a key in |annot|'s dictionary.
    static const char kIRTKey[] = "IRT";
    ASSERT_FALSE(FPDFAnnot_HasKey(annot.get(), kIRTKey));
    EXPECT_FALSE(FPDFAnnot_GetLinkedAnnot(annot.get(), kIRTKey));

    // Attempting to retrieve |annot|'s parent dictionary as an annotation
    // would fail, since its parent is not an annotation.
    ASSERT_TRUE(FPDFAnnot_HasKey(annot.get(), pdfium::annotation::kP));
    EXPECT_EQ(FPDF_OBJECT_REFERENCE,
              FPDFAnnot_GetValueType(annot.get(), pdfium::annotation::kP));
    EXPECT_FALSE(FPDFAnnot_GetLinkedAnnot(annot.get(), pdfium::annotation::kP));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldFlagsTextField) {
  // Open file with form text fields.
  ASSERT_TRUE(OpenDocument("text_form_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve the first annotation: user-editable text field.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Check that the flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_TEXT_PASSWORD);
  }

  {
    // Retrieve the second annotation: read-only text field.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    // Check that the flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_TRUE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_TEXT_PASSWORD);
  }

  {
    // Retrieve the fourth annotation: user-editable password text field.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 3));
    ASSERT_TRUE(annot);

    // Check that the flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_TEXT_PASSWORD);
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldFlagsComboBox) {
  // Open file with form text fields.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve the first annotation: user-editable combobox.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Check that the flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
  }

  {
    // Retrieve the second annotation: regular combobox.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    // Check that the flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
  }

  {
    // Retrieve the third annotation: read-only combobox.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);

    // Check that the flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_TRUE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormAnnotNull) {
  // Open file with form text fields.
  EXPECT_TRUE(OpenDocument("text_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Attempt to get an annotation where no annotation exists on page.
  static const FS_POINTF kOriginPoint = {0.0f, 0.0f};
  EXPECT_FALSE(
      FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, &kOriginPoint));

  static const FS_POINTF kValidPoint = {120.0f, 120.0f};
  {
    // Verify there is an annotation.
    ScopedFPDFAnnotation annot(
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, &kValidPoint));
    EXPECT_TRUE(annot);
  }

  // Try other bad inputs at a valid location.
  EXPECT_FALSE(FPDFAnnot_GetFormFieldAtPoint(nullptr, nullptr, &kValidPoint));
  EXPECT_FALSE(FPDFAnnot_GetFormFieldAtPoint(nullptr, page, &kValidPoint));
  EXPECT_FALSE(
      FPDFAnnot_GetFormFieldAtPoint(form_handle(), nullptr, &kValidPoint));

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormAnnotAndCheckFlagsTextField) {
  // Open file with form text fields.
  EXPECT_TRUE(OpenDocument("text_form_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve user-editable text field annotation.
    static const FS_POINTF kPoint = {105.0f, 118.0f};
    ScopedFPDFAnnotation annot(
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, &kPoint));
    ASSERT_TRUE(annot);

    // Check that interactive form annotation flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
  }

  {
    // Retrieve read-only text field annotation.
    static const FS_POINTF kPoint = {105.0f, 202.0f};
    ScopedFPDFAnnotation annot(
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, &kPoint));
    ASSERT_TRUE(annot);

    // Check that interactive form annotation flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_TRUE(flags & FPDF_FORMFLAG_READONLY);
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormAnnotAndCheckFlagsComboBox) {
  // Open file with form comboboxes.
  EXPECT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve user-editable combobox annotation.
    static const FS_POINTF kPoint = {102.0f, 363.0f};
    ScopedFPDFAnnotation annot(
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, &kPoint));
    ASSERT_TRUE(annot);

    // Check that interactive form annotation flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
  }

  {
    // Retrieve regular combobox annotation.
    static const FS_POINTF kPoint = {102.0f, 413.0f};
    ScopedFPDFAnnotation annot(
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, &kPoint));
    ASSERT_TRUE(annot);

    // Check that interactive form annotation flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
  }

  {
    // Retrieve read-only combobox annotation.
    static const FS_POINTF kPoint = {102.0f, 513.0f};
    ScopedFPDFAnnotation annot(
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, &kPoint));
    ASSERT_TRUE(annot);

    // Check that interactive form annotation flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_TRUE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
  }

  UnloadPage(page);
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_BUG_1206 DISABLED_BUG_1206
#else
#define MAYBE_BUG_1206 BUG_1206
#endif
TEST_F(FPDFAnnotEmbedderTest, MAYBE_BUG_1206) {
  static constexpr size_t kExpectedSize = 1609;
  static const char kExpectedBitmap[] = "0d9fc05c6762fd788bd23fd87a4967bc";

  ASSERT_TRUE(OpenDocument("bug_1206.pdf"));

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  EXPECT_EQ(kExpectedSize, GetString().size());
  ClearString();

  for (size_t i = 0; i < 10; ++i) {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, kExpectedBitmap);

    ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
    // TODO(https://crbug.com/pdfium/1206): This is wrong. The size should be
    // equal, not bigger.
    EXPECT_LT(kExpectedSize, GetString().size());
    ClearString();
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, BUG_1212) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(0, FPDFPage_GetAnnotCount(page));

  static const char kTestKey[] = "test";
  static const wchar_t kData[] = L"\xf6\xe4";
  static const size_t kBufSize = 12;
  std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(kBufSize);

  {
    // Add a text annotation to the page.
    ScopedFPDFAnnotation annot(FPDFPage_CreateAnnot(page, FPDF_ANNOT_TEXT));
    ASSERT_TRUE(annot);
    EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));
    EXPECT_EQ(FPDF_ANNOT_TEXT, FPDFAnnot_GetSubtype(annot.get()));

    // Make sure there is no test key, add set a value there, and read it back.
    std::fill(buf.begin(), buf.end(), 'x');
    ASSERT_EQ(2u, FPDFAnnot_GetStringValue(annot.get(), kTestKey, buf.data(),
                                           kBufSize));
    EXPECT_EQ(L"", GetPlatformWString(buf.data()));

    ScopedFPDFWideString text = GetFPDFWideString(kData);
    EXPECT_TRUE(FPDFAnnot_SetStringValue(annot.get(), kTestKey, text.get()));

    std::fill(buf.begin(), buf.end(), 'x');
    ASSERT_EQ(6u, FPDFAnnot_GetStringValue(annot.get(), kTestKey, buf.data(),
                                           kBufSize));
    EXPECT_EQ(kData, GetPlatformWString(buf.data()));
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_CreateAnnot(page, FPDF_ANNOT_STAMP));
    ASSERT_TRUE(annot);
    const FS_RECTF bounding_rect{206.0f, 753.0f, 339.0f, 709.0f};
    EXPECT_TRUE(FPDFAnnot_SetRect(annot.get(), &bounding_rect));
    EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));
    EXPECT_EQ(FPDF_ANNOT_STAMP, FPDFAnnot_GetSubtype(annot.get()));
    // Also do the same test for its appearance string.
    std::fill(buf.begin(), buf.end(), 'x');
    ASSERT_EQ(2u,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                              buf.data(), kBufSize));
    EXPECT_EQ(L"", GetPlatformWString(buf.data()));

    ScopedFPDFWideString text = GetFPDFWideString(kData);
    EXPECT_TRUE(FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                                text.get()));

    std::fill(buf.begin(), buf.end(), 'x');
    ASSERT_EQ(6u,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                              buf.data(), kBufSize));
    EXPECT_EQ(kData, GetPlatformWString(buf.data()));
  }

  UnloadPage(page);

  {
    // Save a copy, open the copy, and check the annotation again.
    // Note that it renders the rotation.
    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
    ASSERT_TRUE(OpenSavedDocument());
    FPDF_PAGE saved_page = LoadSavedPage(0);
    ASSERT_TRUE(saved_page);

    EXPECT_EQ(2, FPDFPage_GetAnnotCount(saved_page));
    {
      ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(saved_page, 0));
      ASSERT_TRUE(annot);
      EXPECT_EQ(FPDF_ANNOT_TEXT, FPDFAnnot_GetSubtype(annot.get()));

      std::fill(buf.begin(), buf.end(), 'x');
      ASSERT_EQ(6u, FPDFAnnot_GetStringValue(annot.get(), kTestKey, buf.data(),
                                             kBufSize));
      EXPECT_EQ(kData, GetPlatformWString(buf.data()));
    }

    {
      ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(saved_page, 0));
      ASSERT_TRUE(annot);
      // TODO(thestig): This return FPDF_ANNOT_UNKNOWN for some reason.
      // EXPECT_EQ(FPDF_ANNOT_TEXT, FPDFAnnot_GetSubtype(annot.get()));

      std::fill(buf.begin(), buf.end(), 'x');
      ASSERT_EQ(6u, FPDFAnnot_GetStringValue(annot.get(), kTestKey, buf.data(),
                                             kBufSize));
      EXPECT_EQ(kData, GetPlatformWString(buf.data()));
    }

    CloseSavedPage(saved_page);
    CloseSavedDocument();
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionCountCombobox) {
  // Open a file with combobox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    EXPECT_EQ(3, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    EXPECT_EQ(26, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));

    // Check bad form handle / annot.
    EXPECT_EQ(-1, FPDFAnnot_GetOptionCount(nullptr, nullptr));
    EXPECT_EQ(-1, FPDFAnnot_GetOptionCount(form_handle(), nullptr));
    EXPECT_EQ(-1, FPDFAnnot_GetOptionCount(nullptr, annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionCountListbox) {
  // Open a file with listbox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("listbox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    EXPECT_EQ(3, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    EXPECT_EQ(26, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionCountInvalidAnnotations) {
  // Open a file with ink annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // annotations do not have "Opt" array and will return -1
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    EXPECT_EQ(-1, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    EXPECT_EQ(-1, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionLabelCombobox) {
  // Open a file with combobox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    int index = 0;
    unsigned long length_bytes =
        FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index, nullptr, 0);
    ASSERT_EQ(8u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(8u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index,
                                           buf.data(), length_bytes));
    EXPECT_EQ(L"Foo", GetPlatformWString(buf.data()));

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    index = 0;
    length_bytes =
        FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index, nullptr, 0);
    ASSERT_EQ(12u, length_bytes);
    buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(12u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index,
                                            buf.data(), length_bytes));
    EXPECT_EQ(L"Apple", GetPlatformWString(buf.data()));

    index = 25;
    length_bytes =
        FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index, nullptr, 0);
    buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(18u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index,
                                            buf.data(), length_bytes));
    EXPECT_EQ(L"Zucchini", GetPlatformWString(buf.data()));

    // Indices out of range
    EXPECT_EQ(0u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), -1,
                                           nullptr, 0));
    EXPECT_EQ(0u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), 26,
                                           nullptr, 0));

    // Check bad form handle / annot.
    EXPECT_EQ(0u, FPDFAnnot_GetOptionLabel(nullptr, nullptr, 0, nullptr, 0));
    EXPECT_EQ(0u,
              FPDFAnnot_GetOptionLabel(nullptr, annot.get(), 0, nullptr, 0));
    EXPECT_EQ(0u,
              FPDFAnnot_GetOptionLabel(form_handle(), nullptr, 0, nullptr, 0));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionLabelListbox) {
  // Open a file with listbox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("listbox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    int index = 0;
    unsigned long length_bytes =
        FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index, nullptr, 0);
    ASSERT_EQ(8u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(8u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index,
                                           buf.data(), length_bytes));
    EXPECT_EQ(L"Foo", GetPlatformWString(buf.data()));

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    index = 0;
    length_bytes =
        FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index, nullptr, 0);
    ASSERT_EQ(12u, length_bytes);
    buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(12u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index,
                                            buf.data(), length_bytes));
    EXPECT_EQ(L"Apple", GetPlatformWString(buf.data()));

    index = 25;
    length_bytes =
        FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index, nullptr, 0);
    ASSERT_EQ(18u, length_bytes);
    buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(18u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index,
                                            buf.data(), length_bytes));
    EXPECT_EQ(L"Zucchini", GetPlatformWString(buf.data()));

    // indices out of range
    EXPECT_EQ(0u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), -1,
                                           nullptr, 0));
    EXPECT_EQ(0u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), 26,
                                           nullptr, 0));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionLabelInvalidAnnotations) {
  // Open a file with ink annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // annotations do not have "Opt" array and will return 0
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    EXPECT_EQ(0u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), 0,
                                           nullptr, 0));

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    EXPECT_EQ(0u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), 0,
                                           nullptr, 0));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFontSizeCombobox) {
  // Open a file with combobox annotations and load its first page.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // All 3 widgets have Tf font size 12.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    float value;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value));
    EXPECT_EQ(12.0, value);

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    float value_two;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value_two));
    EXPECT_EQ(12.0, value_two);

    annot.reset(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);

    float value_three;
    ASSERT_TRUE(
        FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value_three));
    EXPECT_EQ(12.0, value_three);
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFontSizeTextField) {
  // Open a file with textfield annotations and load its first page.
  ASSERT_TRUE(OpenDocument("text_form_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // All 4 widgets have Tf font size 12.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    float value;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value));
    EXPECT_EQ(12.0, value);

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    float value_two;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value_two));
    EXPECT_EQ(12.0, value_two);

    annot.reset(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);

    float value_three;
    ASSERT_TRUE(
        FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value_three));
    EXPECT_EQ(12.0, value_three);

    float value_four;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value_four));
    EXPECT_EQ(12.0, value_four);
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFontSizeInvalidAnnotationTypes) {
  // Open a file with ink annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Annotations that do not have variable text and will return -1.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    float value;
    ASSERT_FALSE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value));

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    ASSERT_FALSE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFontSizeInvalidArguments) {
  // Open a file with combobox annotations and load its first page.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Check bad form handle / annot.
    float value;
    ASSERT_FALSE(FPDFAnnot_GetFontSize(nullptr, annot.get(), &value));
    ASSERT_FALSE(FPDFAnnot_GetFontSize(form_handle(), nullptr, &value));
    ASSERT_FALSE(FPDFAnnot_GetFontSize(nullptr, nullptr, &value));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFontSizeNegative) {
  // Open a file with textfield annotations and load its first page.
  ASSERT_TRUE(OpenDocument("text_form_negative_fontsize.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Obtain the first annotation, a text field with negative font size, -12.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    float value;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value));
    EXPECT_EQ(-12.0, value);
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedCheckbox) {
  // Open a file with checkbox and radiobuttons widget annotations and load its
  // first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedCheckboxReadOnly) {
  // Open a file with checkbox and radiobutton widget annotations and load its
  // first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    ASSERT_TRUE(FPDFAnnot_IsChecked(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedRadioButton) {
  // Open a file with checkbox and radiobutton widget annotations and load its
  // first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 5));
    ASSERT_TRUE(annot);
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page, 6));
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page, 7));
    ASSERT_TRUE(FPDFAnnot_IsChecked(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedRadioButtonReadOnly) {
  // Open a file with checkbox and radiobutton widget annotations and load its
  // first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page, 3));
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page, 4));
    ASSERT_TRUE(FPDFAnnot_IsChecked(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedInvalidArguments) {
  // Open a file with checkbox and radiobuttons widget annotations and load its
  // first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    ASSERT_FALSE(FPDFAnnot_IsChecked(nullptr, annot.get()));
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), nullptr));
    ASSERT_FALSE(FPDFAnnot_IsChecked(nullptr, nullptr));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedInvalidWidgetType) {
  // Open a file with text widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("text_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldType) {
  ASSERT_TRUE(OpenDocument("multiple_form_types.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  EXPECT_EQ(-1, FPDFAnnot_GetFormFieldType(form_handle(), nullptr));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);
    EXPECT_EQ(-1, FPDFAnnot_GetFormFieldType(nullptr, annot.get()));
  }

  constexpr int kExpectedAnnotTypes[] = {-1,
                                         FPDF_FORMFIELD_COMBOBOX,
                                         FPDF_FORMFIELD_LISTBOX,
                                         FPDF_FORMFIELD_TEXTFIELD,
                                         FPDF_FORMFIELD_CHECKBOX,
                                         FPDF_FORMFIELD_RADIOBUTTON};

  for (size_t i = 0; i < FX_ArraySize(kExpectedAnnotTypes); ++i) {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, i));
    ASSERT_TRUE(annot);
    EXPECT_EQ(kExpectedAnnotTypes[i],
              FPDFAnnot_GetFormFieldType(form_handle(), annot.get()));
  }
  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldValueTextField) {
  ASSERT_TRUE(OpenDocument("text_form_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    EXPECT_EQ(0u,
              FPDFAnnot_GetFormFieldValue(form_handle(), nullptr, nullptr, 0));

    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    EXPECT_EQ(0u,
              FPDFAnnot_GetFormFieldValue(nullptr, annot.get(), nullptr, 0));

    unsigned long length_bytes =
        FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(), nullptr, 0);
    ASSERT_EQ(2u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(2u, FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(),
                                              buf.data(), length_bytes));
    EXPECT_EQ(L"", GetPlatformWString(buf.data()));
  }
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);

    unsigned long length_bytes =
        FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(), nullptr, 0);
    ASSERT_EQ(18u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(18u, FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(),
                                               buf.data(), length_bytes));
    EXPECT_EQ(L"Elephant", GetPlatformWString(buf.data()));
  }
  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldValueComboBox) {
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    unsigned long length_bytes =
        FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(), nullptr, 0);
    ASSERT_EQ(2u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(2u, FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(),
                                              buf.data(), length_bytes));
    EXPECT_EQ(L"", GetPlatformWString(buf.data()));
  }
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    unsigned long length_bytes =
        FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(), nullptr, 0);
    ASSERT_EQ(14u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(14u, FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(),
                                               buf.data(), length_bytes));
    EXPECT_EQ(L"Banana", GetPlatformWString(buf.data()));
  }
  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldNameTextField) {
  ASSERT_TRUE(OpenDocument("text_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    EXPECT_EQ(0u,
              FPDFAnnot_GetFormFieldName(form_handle(), nullptr, nullptr, 0));

    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    EXPECT_EQ(0u, FPDFAnnot_GetFormFieldName(nullptr, annot.get(), nullptr, 0));

    unsigned long length_bytes =
        FPDFAnnot_GetFormFieldName(form_handle(), annot.get(), nullptr, 0);
    ASSERT_EQ(18u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(18u, FPDFAnnot_GetFormFieldName(form_handle(), annot.get(),
                                              buf.data(), length_bytes));
    EXPECT_EQ(L"Text Box", GetPlatformWString(buf.data()));
  }
  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldNameComboBox) {
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    unsigned long length_bytes =
        FPDFAnnot_GetFormFieldName(form_handle(), annot.get(), nullptr, 0);
    ASSERT_EQ(30u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(30u, FPDFAnnot_GetFormFieldName(form_handle(), annot.get(),
                                              buf.data(), length_bytes));
    EXPECT_EQ(L"Combo_Editable", GetPlatformWString(buf.data()));
  }
  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, SetFocusableAnnotSubtypes) {
  ASSERT_TRUE(OpenDocument("text_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Annotations of type FPDF_ANNOT_WIDGET are by default focusable.
  EXPECT_EQ(1, FPDFAnnot_GetFocusableSubtypesCount(form_handle()));

  EXPECT_TRUE(FPDFAnnot_SetFocusableSubtypes(form_handle(), nullptr, 0));
  EXPECT_EQ(0, FPDFAnnot_GetFocusableSubtypesCount(form_handle()));

  constexpr FPDF_ANNOTATION_SUBTYPE kFocusableSubtypes[] = {FPDF_ANNOT_WIDGET};
  constexpr size_t kSubtypeCount = FX_ArraySize(kFocusableSubtypes);

  EXPECT_FALSE(FPDFAnnot_SetFocusableSubtypes(nullptr, kFocusableSubtypes,
                                              kSubtypeCount));
  EXPECT_FALSE(
      FPDFAnnot_SetFocusableSubtypes(form_handle(), nullptr, kSubtypeCount));

  EXPECT_TRUE(FPDFAnnot_SetFocusableSubtypes(form_handle(), kFocusableSubtypes,
                                             kSubtypeCount));
  EXPECT_EQ(static_cast<int>(kSubtypeCount),
            FPDFAnnot_GetFocusableSubtypesCount(form_handle()));

  EXPECT_TRUE(
      FPDFAnnot_SetFocusableSubtypes(form_handle(), kFocusableSubtypes, 0));
  EXPECT_EQ(0, FPDFAnnot_GetFocusableSubtypesCount(form_handle()));

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFocusableAnnotSubtypesCount) {
  ASSERT_TRUE(OpenDocument("text_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  EXPECT_EQ(-1, FPDFAnnot_GetFocusableSubtypesCount(nullptr));

  // FPDF_ANNOT_WIDGET is default annot subtype which is focusable,
  // hence 1 is default count.
  EXPECT_EQ(1, FPDFAnnot_GetFocusableSubtypesCount(form_handle()));

  constexpr FPDF_ANNOTATION_SUBTYPE kFocusableSubtypes[] = {FPDF_ANNOT_WIDGET,
                                                            FPDF_ANNOT_LINK};
  constexpr size_t kSubtypeCount = FX_ArraySize(kFocusableSubtypes);

  FPDFAnnot_SetFocusableSubtypes(form_handle(), kFocusableSubtypes,
                                 kSubtypeCount);

  EXPECT_EQ(static_cast<int>(kSubtypeCount),
            FPDFAnnot_GetFocusableSubtypesCount(form_handle()));

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFocusableAnnotSubtypes) {
  ASSERT_TRUE(OpenDocument("text_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  constexpr FPDF_ANNOTATION_SUBTYPE kFocusableSubtypes[] = {FPDF_ANNOT_WIDGET};
  constexpr size_t kSubtypeCount = FX_ArraySize(kFocusableSubtypes);

  FPDFAnnot_SetFocusableSubtypes(form_handle(), kFocusableSubtypes,
                                 kSubtypeCount);

  int count = FPDFAnnot_GetFocusableSubtypesCount(form_handle());
  ASSERT_EQ(1, count);
  EXPECT_FALSE(FPDFAnnot_GetFocusableSubtypes(form_handle(), nullptr, count));

  std::vector<FPDF_ANNOTATION_SUBTYPE> subtypes(count, FPDF_ANNOT_UNKNOWN);
  EXPECT_FALSE(FPDFAnnot_GetFocusableSubtypes(form_handle(), subtypes.data(),
                                              count - 1));

  EXPECT_TRUE(
      FPDFAnnot_GetFocusableSubtypes(form_handle(), subtypes.data(), count));

  for (int i = 0; i < count; ++i) {
    EXPECT_EQ(kFocusableSubtypes[i], subtypes[i]);
  }

  UnloadPage(page);
}
