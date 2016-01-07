// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_edit.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gmock/include/gmock/gmock-matchers.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

class FPDFEditEmbeddertest : public EmbedderTest, public TestSaver {};

TEST_F(FPDFEditEmbeddertest, EmptyCreation) {
  EXPECT_TRUE(CreateEmptyDocument());
  FPDF_PAGE page = FPDFPage_New(document(), 1, 640.0, 480.0);
  EXPECT_NE(nullptr, page);
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  EXPECT_THAT(GetString(),
              testing::MatchesRegex(
                  "%PDF-1.7\r\n"
                  "%\xA1\xB3\xC5\xD7\r\n"
                  "1 0 obj\r\n"
                  "<</Type/Catalog/Pages 2 0 R >>\r\n"
                  "endobj\r\n"
                  "2 0 obj\r\n"
                  "<</Type/Pages/Count 1/Kids\\[ 4 0 R \\]>>\r\n"
                  "endobj\r\n"
                  "3 0 obj\r\n"
                  "<</CreationDate\\(D:.*\\)/Creator\\(PDFium\\)>>\r\n"
                  "endobj\r\n"
                  "4 0 obj\r\n"
                  "<</Type/Page/Parent 2 0 R /MediaBox\\[ 0 0 640 "
                  "480\\]/Rotate 0/Resources<<>>/Contents 5 0 R >>\r\n"
                  "endobj\r\n"
                  "5 0 obj\r\n"
                  "<</Length 8/Filter/FlateDecode>>stream\r\n"
                  "x\x9C\x3\0\0\0\0\x1\r\n"
                  "endstream\r\n"
                  "endobj\r\n"
                  "xref\r\n"
                  "0 6\r\n"
                  "0000000000 65535 f\r\n"
                  "0000000017 00000 n\r\n"
                  "0000000066 00000 n\r\n"
                  "0000000122 00000 n\r\n"
                  "0000000192 00000 n\r\n"
                  "0000000301 00000 n\r\n"
                  "trailer\r\n"
                  "<<\r\n"
                  "/Root 1 0 R\r\n"
                  "/Info 3 0 R\r\n"
                  "/Size 6/ID\\[<.*><.*>\\]>>\r\n"
                  "startxref\r\n"
                  "379\r\n"
                  "%%EOF\r\n"));
  FPDFPage_Delete(document(), 1);
}
