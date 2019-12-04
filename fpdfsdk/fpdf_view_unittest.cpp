// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdfview.h"
#include "testing/gtest/include/gtest/gtest.h"

#ifdef PDF_ENABLE_XFA
TEST(FPDFView, BstrBadArgs) {
  EXPECT_EQ(-1, FPDF_BStr_Init(nullptr));
  EXPECT_EQ(-1, FPDF_BStr_Set(nullptr, "clams", -1));
  EXPECT_EQ(-1, FPDF_BStr_Clear(nullptr));
}

TEST(FPDFView, BstrEmpty) {
  FPDF_BSTR bst;
  EXPECT_EQ(0, FPDF_BStr_Init(&bst));
  EXPECT_FALSE(bst.str);
  EXPECT_FALSE(bst.len);
  EXPECT_EQ(0, FPDF_BStr_Clear(&bst));
}

TEST(FPDFView, BstrNormal) {
  FPDF_BSTR bst;
  EXPECT_EQ(0, FPDF_BStr_Init(&bst));
  EXPECT_EQ(0, FPDF_BStr_Set(&bst, "clams", -1));
  EXPECT_STREQ("clams", bst.str);
  EXPECT_EQ(5, bst.len);

  EXPECT_EQ(0, FPDF_BStr_Clear(&bst));
  EXPECT_FALSE(bst.str);
  EXPECT_FALSE(bst.len);
}

TEST(FPDFView, BstrReassign) {
  FPDF_BSTR bst;
  EXPECT_EQ(0, FPDF_BStr_Init(&bst));
  EXPECT_EQ(0, FPDF_BStr_Set(&bst, "clams", 3));
  EXPECT_STREQ("cla", bst.str);
  EXPECT_EQ(3, bst.len);

  EXPECT_EQ(0, FPDF_BStr_Set(&bst, "clams", 5));
  EXPECT_STREQ("clams", bst.str);
  EXPECT_EQ(5, bst.len);

  EXPECT_EQ(0, FPDF_BStr_Set(&bst, "clams", 1));
  EXPECT_STREQ("c", bst.str);
  EXPECT_EQ(1, bst.len);

  EXPECT_EQ(0, FPDF_BStr_Set(&bst, "clams", 0));
  EXPECT_FALSE(bst.str);
  EXPECT_EQ(0, bst.len);

  EXPECT_EQ(0, FPDF_BStr_Clear(&bst));
}
#endif  // PDF_ENABLE_XFA
