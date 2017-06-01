// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "public/fpdf_annot.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFAnnotEmbeddertest : public EmbedderTest {};

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

  // Check that the third annotation of type "ink".
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
