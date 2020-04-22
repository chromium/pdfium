// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_string.h"
#include "public/fpdf_structtree.h"
#include "testing/embedder_test.h"
#include "third_party/base/optional.h"

class FPDFStructTreeEmbedderTest : public EmbedderTest {};

TEST_F(FPDFStructTreeEmbedderTest, GetAltText) {
  ASSERT_TRUE(OpenDocument("tagged_alt_text.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
    ASSERT_EQ(1, FPDF_StructTree_CountChildren(struct_tree.get()));

    FPDF_STRUCTELEMENT element =
        FPDF_StructTree_GetChildAtIndex(struct_tree.get(), -1);
    EXPECT_FALSE(element);
    element = FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 1);
    EXPECT_FALSE(element);
    element = FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 0);
    ASSERT_TRUE(element);
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(element));
    EXPECT_EQ(0U, FPDF_StructElement_GetAltText(element, nullptr, 0));

    ASSERT_EQ(1, FPDF_StructElement_CountChildren(element));
    FPDF_STRUCTELEMENT child_element =
        FPDF_StructElement_GetChildAtIndex(element, -1);
    EXPECT_FALSE(child_element);
    child_element = FPDF_StructElement_GetChildAtIndex(element, 1);
    EXPECT_FALSE(child_element);
    child_element = FPDF_StructElement_GetChildAtIndex(element, 0);
    ASSERT_TRUE(child_element);
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(child_element));
    EXPECT_EQ(0U, FPDF_StructElement_GetAltText(child_element, nullptr, 0));

    ASSERT_EQ(1, FPDF_StructElement_CountChildren(child_element));
    FPDF_STRUCTELEMENT gchild_element =
        FPDF_StructElement_GetChildAtIndex(child_element, -1);
    EXPECT_FALSE(gchild_element);
    gchild_element = FPDF_StructElement_GetChildAtIndex(child_element, 1);
    EXPECT_FALSE(gchild_element);
    gchild_element = FPDF_StructElement_GetChildAtIndex(child_element, 0);
    ASSERT_TRUE(gchild_element);
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(gchild_element));
    ASSERT_EQ(24U, FPDF_StructElement_GetAltText(gchild_element, nullptr, 0));

    unsigned short buffer[12];
    memset(buffer, 0, sizeof(buffer));
    // Deliberately pass in a small buffer size to make sure |buffer| remains
    // untouched.
    ASSERT_EQ(24U, FPDF_StructElement_GetAltText(gchild_element, buffer, 1));
    for (size_t i = 0; i < FX_ArraySize(buffer); ++i)
      EXPECT_EQ(0U, buffer[i]);

    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(gchild_element));
    ASSERT_EQ(24U, FPDF_StructElement_GetAltText(gchild_element, buffer,
                                                 sizeof(buffer)));
    const wchar_t kExpected[] = L"Black Image";
    EXPECT_EQ(WideString(kExpected),
              WideString::FromUTF16LE(buffer, FXSYS_len(kExpected)));

    ASSERT_EQ(1, FPDF_StructElement_CountChildren(gchild_element));
    FPDF_STRUCTELEMENT ggchild_element =
        FPDF_StructElement_GetChildAtIndex(gchild_element, 0);
    EXPECT_FALSE(ggchild_element);
  }

  UnloadPage(page);
}

TEST_F(FPDFStructTreeEmbedderTest, GetMarkedContentID) {
  ASSERT_TRUE(OpenDocument("marked_content_id.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
    ASSERT_EQ(1, FPDF_StructTree_CountChildren(struct_tree.get()));

    FPDF_STRUCTELEMENT element =
        FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 0);
    EXPECT_EQ(0, FPDF_StructElement_GetMarkedContentID(element));
  }

  UnloadPage(page);
}

TEST_F(FPDFStructTreeEmbedderTest, GetType) {
  ASSERT_TRUE(OpenDocument("tagged_alt_text.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
    ASSERT_EQ(1, FPDF_StructTree_CountChildren(struct_tree.get()));

    FPDF_STRUCTELEMENT element =
        FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 0);
    ASSERT_TRUE(element);

    // test nullptr inputs
    unsigned short buffer[12];
    ASSERT_EQ(0U, FPDF_StructElement_GetType(nullptr, buffer, sizeof(buffer)));
    ASSERT_EQ(0U, FPDF_StructElement_GetType(nullptr, nullptr, 0));
    ASSERT_EQ(18U, FPDF_StructElement_GetType(element, nullptr, 0));

    memset(buffer, 0, sizeof(buffer));
    // Deliberately pass in a small buffer size to make sure |buffer| remains
    // untouched.
    ASSERT_EQ(18U, FPDF_StructElement_GetType(element, buffer, 1));
    for (size_t i = 0; i < FX_ArraySize(buffer); ++i)
      EXPECT_EQ(0U, buffer[i]);

    ASSERT_EQ(18U, FPDF_StructElement_GetType(element, buffer, sizeof(buffer)));
    const wchar_t kExpected[] = L"Document";
    EXPECT_EQ(WideString(kExpected),
              WideString::FromUTF16LE(buffer, FXSYS_len(kExpected)));
  }

  UnloadPage(page);
}

TEST_F(FPDFStructTreeEmbedderTest, GetTitle) {
  ASSERT_TRUE(OpenDocument("tagged_alt_text.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
    ASSERT_EQ(1, FPDF_StructTree_CountChildren(struct_tree.get()));

    FPDF_STRUCTELEMENT element =
        FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 0);
    ASSERT_TRUE(element);

    // test nullptr inputs
    unsigned short buffer[13];
    ASSERT_EQ(0U, FPDF_StructElement_GetTitle(nullptr, buffer, sizeof(buffer)));
    ASSERT_EQ(0U, FPDF_StructElement_GetTitle(nullptr, nullptr, 0));
    ASSERT_EQ(20U, FPDF_StructElement_GetTitle(element, nullptr, 0));

    memset(buffer, 0, sizeof(buffer));
    // Deliberately pass in a small buffer size to make sure |buffer| remains
    // untouched.
    ASSERT_EQ(20U, FPDF_StructElement_GetTitle(element, buffer, 1));
    for (size_t i = 0; i < FX_ArraySize(buffer); ++i)
      EXPECT_EQ(0U, buffer[i]);

    ASSERT_EQ(20U,
              FPDF_StructElement_GetTitle(element, buffer, sizeof(buffer)));

    const wchar_t kExpected[] = L"TitleText";
    EXPECT_EQ(WideString(kExpected),
              WideString::FromUTF16LE(buffer, FXSYS_len(kExpected)));

    ASSERT_EQ(1, FPDF_StructElement_CountChildren(element));
    FPDF_STRUCTELEMENT child_element =
        FPDF_StructElement_GetChildAtIndex(element, 0);
    ASSERT_TRUE(element);

    ASSERT_EQ(26U, FPDF_StructElement_GetTitle(child_element, buffer,
                                               sizeof(buffer)));
    const wchar_t kChildExpected[] = L"symbol: 100k";
    EXPECT_EQ(WideString(kChildExpected),
              WideString::FromUTF16LE(buffer, FXSYS_len(kChildExpected)));
  }

  UnloadPage(page);
}

TEST_F(FPDFStructTreeEmbedderTest, GetStructTreeForNestedTaggedPDF) {
  ASSERT_TRUE(OpenDocument("tagged_nested.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // This call should not crash. https://crbug.com/pdfium/1480
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
  }
  UnloadPage(page);
}

TEST_F(FPDFStructTreeEmbedderTest, MarkedContentReferenceAndObjectReference) {
  ASSERT_TRUE(OpenDocument("tagged_mcr_objr.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
    ASSERT_EQ(1, FPDF_StructTree_CountChildren(struct_tree.get()));

    FPDF_STRUCTELEMENT object8 =
        FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 0);
    ASSERT_TRUE(object8);
    unsigned short buffer[12];
    ASSERT_EQ(18U, FPDF_StructElement_GetType(object8, buffer, sizeof(buffer)));
    const wchar_t kExpectedObject8Type[] = L"Document";
    EXPECT_EQ(WideString(kExpectedObject8Type),
              WideString::FromUTF16LE(buffer, FXSYS_len(kExpectedObject8Type)));
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(object8));
    ASSERT_EQ(2, FPDF_StructElement_CountChildren(object8));

    // First branch. 10 -> 12 -> 13 -> Inline dict.
    FPDF_STRUCTELEMENT object10 =
        FPDF_StructElement_GetChildAtIndex(object8, 0);
    ASSERT_TRUE(object10);
    ASSERT_EQ(20U,
              FPDF_StructElement_GetType(object10, buffer, sizeof(buffer)));
    const wchar_t kExpectedObject10Type[] = L"NonStruct";
    EXPECT_EQ(
        WideString(kExpectedObject10Type),
        WideString::FromUTF16LE(buffer, FXSYS_len(kExpectedObject10Type)));
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(object10));
    ASSERT_EQ(1, FPDF_StructElement_CountChildren(object10));

    FPDF_STRUCTELEMENT object12 =
        FPDF_StructElement_GetChildAtIndex(object10, 0);
    ASSERT_TRUE(object12);
    ASSERT_EQ(4U, FPDF_StructElement_GetType(object12, buffer, sizeof(buffer)));
    const wchar_t kExpectedObject12Type[] = L"P";
    EXPECT_EQ(
        WideString(kExpectedObject12Type),
        WideString::FromUTF16LE(buffer, FXSYS_len(kExpectedObject12Type)));
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(object12));
    ASSERT_EQ(1, FPDF_StructElement_CountChildren(object12));

    FPDF_STRUCTELEMENT object13 =
        FPDF_StructElement_GetChildAtIndex(object12, 0);
    ASSERT_TRUE(object13);
    ASSERT_EQ(20U,
              FPDF_StructElement_GetType(object13, buffer, sizeof(buffer)));
    const wchar_t kExpectedObject13Type[] = L"NonStruct";
    EXPECT_EQ(
        WideString(kExpectedObject13Type),
        WideString::FromUTF16LE(buffer, FXSYS_len(kExpectedObject13Type)));
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(object13));
    ASSERT_EQ(1, FPDF_StructElement_CountChildren(object13));

    // TODO(crbug.com/pdfium/672): Fetch this child element.
    EXPECT_FALSE(FPDF_StructElement_GetChildAtIndex(object13, 0));

    // Second branch. 11 -> 14 -> Inline dict.
    //                         -> 15 -> Inline dict.
    FPDF_STRUCTELEMENT object11 =
        FPDF_StructElement_GetChildAtIndex(object8, 1);
    ASSERT_TRUE(object11);
    ASSERT_EQ(4U, FPDF_StructElement_GetType(object11, buffer, sizeof(buffer)));
    const wchar_t kExpectedObject11Type[] = L"P";
    EXPECT_EQ(
        WideString(kExpectedObject11Type),
        WideString::FromUTF16LE(buffer, FXSYS_len(kExpectedObject11Type)));
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(object11));
    ASSERT_EQ(1, FPDF_StructElement_CountChildren(object11));

    FPDF_STRUCTELEMENT object14 =
        FPDF_StructElement_GetChildAtIndex(object11, 0);
    ASSERT_TRUE(object14);
    ASSERT_EQ(20U,
              FPDF_StructElement_GetType(object14, buffer, sizeof(buffer)));
    const wchar_t kExpectedObject14Type[] = L"NonStruct";
    EXPECT_EQ(
        WideString(kExpectedObject14Type),
        WideString::FromUTF16LE(buffer, FXSYS_len(kExpectedObject14Type)));
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(object14));
    ASSERT_EQ(2, FPDF_StructElement_CountChildren(object14));

    // TODO(crbug.com/pdfium/672): Object 15 should be at index 1.
    EXPECT_FALSE(FPDF_StructElement_GetChildAtIndex(object14, 1));
    FPDF_STRUCTELEMENT object15 =
        FPDF_StructElement_GetChildAtIndex(object14, 0);
    ASSERT_TRUE(object15);
    ASSERT_EQ(20U,
              FPDF_StructElement_GetType(object15, buffer, sizeof(buffer)));
    const wchar_t kExpectedObject15Type[] = L"NonStruct";
    EXPECT_EQ(
        WideString(kExpectedObject15Type),
        WideString::FromUTF16LE(buffer, FXSYS_len(kExpectedObject15Type)));
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(object15));
    ASSERT_EQ(1, FPDF_StructElement_CountChildren(object15));

    // TODO(crbug.com/pdfium/672): Fetch this child element.
    EXPECT_FALSE(FPDF_StructElement_GetChildAtIndex(object15, 0));
  }

  UnloadPage(page);
}
