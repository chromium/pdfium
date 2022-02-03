// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_structtree.h"
#include "testing/embedder_test.h"
#include "testing/fx_string_testhelpers.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/base/cxx17_backports.h"

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
    for (size_t i = 0; i < pdfium::size(buffer); ++i)
      EXPECT_EQ(0U, buffer[i]);

    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(gchild_element));
    ASSERT_EQ(24U, FPDF_StructElement_GetAltText(gchild_element, buffer,
                                                 sizeof(buffer)));
    EXPECT_EQ(L"Black Image", GetPlatformWString(buffer));

    ASSERT_EQ(1, FPDF_StructElement_CountChildren(gchild_element));
    FPDF_STRUCTELEMENT ggchild_element =
        FPDF_StructElement_GetChildAtIndex(gchild_element, 0);
    EXPECT_FALSE(ggchild_element);
  }

  UnloadPage(page);
}

TEST_F(FPDFStructTreeEmbedderTest, GetStringAttribute) {
  ASSERT_TRUE(OpenDocument("tagged_table.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
    ASSERT_EQ(1, FPDF_StructTree_CountChildren(struct_tree.get()));

    FPDF_STRUCTELEMENT document =
        FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 0);
    ASSERT_TRUE(document);

    constexpr int kBufLen = 100;
    uint16_t buffer[kBufLen] = {0};
    EXPECT_EQ(18U, FPDF_StructElement_GetType(document, buffer, kBufLen));
    EXPECT_EQ("Document", GetPlatformString(buffer));

    ASSERT_EQ(1, FPDF_StructElement_CountChildren(document));
    FPDF_STRUCTELEMENT table = FPDF_StructElement_GetChildAtIndex(document, 0);
    ASSERT_TRUE(table);

    EXPECT_EQ(12U, FPDF_StructElement_GetType(table, buffer, kBufLen));
    EXPECT_EQ("Table", GetPlatformString(buffer));

    // The table should have an attribute "Summary" set to the empty string.
    EXPECT_EQ(2U, FPDF_StructElement_GetStringAttribute(table, "Summary",
                                                        buffer, kBufLen));

    ASSERT_EQ(2, FPDF_StructElement_CountChildren(table));
    FPDF_STRUCTELEMENT row = FPDF_StructElement_GetChildAtIndex(table, 0);
    ASSERT_TRUE(row);

    ASSERT_EQ(2, FPDF_StructElement_CountChildren(row));
    FPDF_STRUCTELEMENT header_cell = FPDF_StructElement_GetChildAtIndex(row, 0);
    ASSERT_TRUE(header_cell);

    EXPECT_EQ(6U, FPDF_StructElement_GetType(header_cell, buffer, kBufLen));
    EXPECT_EQ("TH", GetPlatformString(buffer));

    // The header should have an attribute "Scope" with a scope of "Row".
    EXPECT_EQ(8U, FPDF_StructElement_GetStringAttribute(header_cell, "Scope",
                                                        buffer, kBufLen));
    EXPECT_EQ("Row", GetPlatformString(buffer));

    // The header has an attribute "ColSpan", but it's not a string so it
    // returns null.
    EXPECT_EQ(0U, FPDF_StructElement_GetStringAttribute(header_cell, "ColSpan",
                                                        buffer, kBufLen));

    // An unsupported attribute should return 0.
    EXPECT_EQ(0U, FPDF_StructElement_GetStringAttribute(header_cell, "Other",
                                                        buffer, kBufLen));

    // A null struct element should not crash.
    EXPECT_EQ(0U, FPDF_StructElement_GetStringAttribute(nullptr, "Other",
                                                        buffer, kBufLen));
  }

  UnloadPage(page);
}

TEST_F(FPDFStructTreeEmbedderTest, GetStringAttributeBadStructElement) {
  ASSERT_TRUE(OpenDocument("tagged_table_bad_elem.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
    ASSERT_EQ(1, FPDF_StructTree_CountChildren(struct_tree.get()));

    FPDF_STRUCTELEMENT document =
        FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 0);
    ASSERT_TRUE(document);

    constexpr int kBufLen = 100;
    uint16_t buffer[kBufLen] = {0};
    EXPECT_EQ(18U, FPDF_StructElement_GetType(document, buffer, kBufLen));
    EXPECT_EQ("Document", GetPlatformString(buffer));

    ASSERT_EQ(1, FPDF_StructElement_CountChildren(document));
    FPDF_STRUCTELEMENT table = FPDF_StructElement_GetChildAtIndex(document, 0);
    ASSERT_TRUE(table);

    EXPECT_EQ(12U, FPDF_StructElement_GetType(table, buffer, kBufLen));
    EXPECT_EQ("Table", GetPlatformString(buffer));

    // The table entry cannot be retrieved, as the element is malformed.
    EXPECT_EQ(0U, FPDF_StructElement_GetStringAttribute(table, "Summary",
                                                        buffer, kBufLen));
  }

  UnloadPage(page);
}

TEST_F(FPDFStructTreeEmbedderTest, GetID) {
  ASSERT_TRUE(OpenDocument("tagged_table.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
    ASSERT_EQ(1, FPDF_StructTree_CountChildren(struct_tree.get()));

    FPDF_STRUCTELEMENT document =
        FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 0);
    ASSERT_TRUE(document);

    constexpr int kBufLen = 100;
    uint16_t buffer[kBufLen] = {0};
    EXPECT_EQ(18U, FPDF_StructElement_GetType(document, buffer, kBufLen));
    EXPECT_EQ("Document", GetPlatformString(buffer));

    // The document has no ID.
    EXPECT_EQ(0U, FPDF_StructElement_GetID(document, buffer, kBufLen));

    ASSERT_EQ(1, FPDF_StructElement_CountChildren(document));
    FPDF_STRUCTELEMENT table = FPDF_StructElement_GetChildAtIndex(document, 0);
    ASSERT_TRUE(table);

    EXPECT_EQ(12U, FPDF_StructElement_GetType(table, buffer, kBufLen));
    EXPECT_EQ("Table", GetPlatformString(buffer));

    // The table has an ID.
    EXPECT_EQ(14U, FPDF_StructElement_GetID(table, buffer, kBufLen));
    EXPECT_EQ("node12", GetPlatformString(buffer));

    // The first child of the table is a row, which has an empty ID.
    // It returns 2U, the length of an empty string, instead of 0U,
    // representing null.
    ASSERT_EQ(2, FPDF_StructElement_CountChildren(table));
    FPDF_STRUCTELEMENT row = FPDF_StructElement_GetChildAtIndex(table, 0);
    ASSERT_TRUE(row);
    EXPECT_EQ(2U, FPDF_StructElement_GetID(row, buffer, kBufLen));
  }

  UnloadPage(page);
}

TEST_F(FPDFStructTreeEmbedderTest, GetLang) {
  ASSERT_TRUE(OpenDocument("tagged_table.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
    ASSERT_EQ(1, FPDF_StructTree_CountChildren(struct_tree.get()));

    FPDF_STRUCTELEMENT document =
        FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 0);
    ASSERT_TRUE(document);

    constexpr int kBufLen = 100;
    uint16_t buffer[kBufLen] = {0};
    EXPECT_EQ(18U, FPDF_StructElement_GetType(document, buffer, kBufLen));
    EXPECT_EQ("Document", GetPlatformString(buffer));

    // The document has a language.
    EXPECT_EQ(12U, FPDF_StructElement_GetLang(document, buffer, kBufLen));
    EXPECT_EQ("en-US", GetPlatformString(buffer));

    ASSERT_EQ(1, FPDF_StructElement_CountChildren(document));
    FPDF_STRUCTELEMENT table = FPDF_StructElement_GetChildAtIndex(document, 0);
    ASSERT_TRUE(table);

    // The first child is a table, with a language.
    EXPECT_EQ(12U, FPDF_StructElement_GetType(table, buffer, kBufLen));
    EXPECT_EQ("Table", GetPlatformString(buffer));

    EXPECT_EQ(6U, FPDF_StructElement_GetLang(table, buffer, kBufLen));
    EXPECT_EQ("hu", GetPlatformString(buffer));

    // The first child of the table is a row, which doesn't have a
    // language explicitly set on it.
    ASSERT_EQ(2, FPDF_StructElement_CountChildren(table));
    FPDF_STRUCTELEMENT row = FPDF_StructElement_GetChildAtIndex(table, 0);
    ASSERT_TRUE(row);
    EXPECT_EQ(0U, FPDF_StructElement_GetLang(row, buffer, kBufLen));
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
    for (size_t i = 0; i < pdfium::size(buffer); ++i)
      EXPECT_EQ(0U, buffer[i]);

    ASSERT_EQ(18U, FPDF_StructElement_GetType(element, buffer, sizeof(buffer)));
    EXPECT_EQ(L"Document", GetPlatformWString(buffer));
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
    for (size_t i = 0; i < pdfium::size(buffer); ++i)
      EXPECT_EQ(0U, buffer[i]);

    ASSERT_EQ(20U,
              FPDF_StructElement_GetTitle(element, buffer, sizeof(buffer)));

    EXPECT_EQ(L"TitleText", GetPlatformWString(buffer));

    ASSERT_EQ(1, FPDF_StructElement_CountChildren(element));
    FPDF_STRUCTELEMENT child_element =
        FPDF_StructElement_GetChildAtIndex(element, 0);
    ASSERT_TRUE(element);

    ASSERT_EQ(26U, FPDF_StructElement_GetTitle(child_element, buffer,
                                               sizeof(buffer)));
    EXPECT_EQ(L"symbol: 100k", GetPlatformWString(buffer));
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
    EXPECT_EQ(L"Document", GetPlatformWString(buffer));
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(object8));
    ASSERT_EQ(2, FPDF_StructElement_CountChildren(object8));

    // First branch. 10 -> 12 -> 13 -> Inline dict.
    FPDF_STRUCTELEMENT object10 =
        FPDF_StructElement_GetChildAtIndex(object8, 0);
    ASSERT_TRUE(object10);
    ASSERT_EQ(20U,
              FPDF_StructElement_GetType(object10, buffer, sizeof(buffer)));
    EXPECT_EQ(L"NonStruct", GetPlatformWString(buffer));
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(object10));
    ASSERT_EQ(1, FPDF_StructElement_CountChildren(object10));

    FPDF_STRUCTELEMENT object12 =
        FPDF_StructElement_GetChildAtIndex(object10, 0);
    ASSERT_TRUE(object12);
    ASSERT_EQ(4U, FPDF_StructElement_GetType(object12, buffer, sizeof(buffer)));
    EXPECT_EQ(L"P", GetPlatformWString(buffer));
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(object12));
    ASSERT_EQ(1, FPDF_StructElement_CountChildren(object12));

    FPDF_STRUCTELEMENT object13 =
        FPDF_StructElement_GetChildAtIndex(object12, 0);
    ASSERT_TRUE(object13);
    ASSERT_EQ(20U,
              FPDF_StructElement_GetType(object13, buffer, sizeof(buffer)));
    EXPECT_EQ(L"NonStruct", GetPlatformWString(buffer));
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
    EXPECT_EQ(L"P", GetPlatformWString(buffer));
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(object11));
    ASSERT_EQ(1, FPDF_StructElement_CountChildren(object11));

    FPDF_STRUCTELEMENT object14 =
        FPDF_StructElement_GetChildAtIndex(object11, 0);
    ASSERT_TRUE(object14);
    ASSERT_EQ(20U,
              FPDF_StructElement_GetType(object14, buffer, sizeof(buffer)));
    EXPECT_EQ(L"NonStruct", GetPlatformWString(buffer));
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(object14));
    ASSERT_EQ(2, FPDF_StructElement_CountChildren(object14));

    // TODO(crbug.com/pdfium/672): Object 15 should be at index 1.
    EXPECT_FALSE(FPDF_StructElement_GetChildAtIndex(object14, 1));
    FPDF_STRUCTELEMENT object15 =
        FPDF_StructElement_GetChildAtIndex(object14, 0);
    ASSERT_TRUE(object15);
    ASSERT_EQ(20U,
              FPDF_StructElement_GetType(object15, buffer, sizeof(buffer)));
    EXPECT_EQ(L"NonStruct", GetPlatformWString(buffer));
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(object15));
    ASSERT_EQ(1, FPDF_StructElement_CountChildren(object15));

    // TODO(crbug.com/pdfium/672): Fetch this child element.
    EXPECT_FALSE(FPDF_StructElement_GetChildAtIndex(object15, 0));
  }

  UnloadPage(page);
}

TEST_F(FPDFStructTreeEmbedderTest, Bug1768) {
  ASSERT_TRUE(OpenDocument("bug_1768.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
    ASSERT_EQ(1, FPDF_StructTree_CountChildren(struct_tree.get()));

    // TODO(crbug.com/pdfium/1768): Fetch this child element. Then consider
    // writing more of the test to make sure other elements in the tree can be
    // fetched correctly as well.
    EXPECT_FALSE(FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 0));
  }

  UnloadPage(page);
}
