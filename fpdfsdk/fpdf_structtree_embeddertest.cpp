// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "public/fpdf_structtree.h"
#include "testing/embedder_test.h"
#include "testing/fx_string_testhelpers.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

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
    for (size_t i = 0; i < std::size(buffer); ++i)
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

TEST_F(FPDFStructTreeEmbedderTest, GetActualText) {
  ASSERT_TRUE(OpenDocument("tagged_actual_text.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
    ASSERT_EQ(1, FPDF_StructTree_CountChildren(struct_tree.get()));

    EXPECT_EQ(0U, FPDF_StructElement_GetActualText(nullptr, nullptr, 0));

    FPDF_STRUCTELEMENT element =
        FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 0);
    ASSERT_TRUE(element);
    EXPECT_EQ(0U, FPDF_StructElement_GetActualText(element, nullptr, 0));

    ASSERT_EQ(1, FPDF_StructElement_CountChildren(element));
    FPDF_STRUCTELEMENT child_element =
        FPDF_StructElement_GetChildAtIndex(element, 0);
    ASSERT_TRUE(child_element);
    EXPECT_EQ(0U, FPDF_StructElement_GetActualText(child_element, nullptr, 0));

    ASSERT_EQ(1, FPDF_StructElement_CountChildren(child_element));
    FPDF_STRUCTELEMENT gchild_element =
        FPDF_StructElement_GetChildAtIndex(child_element, 0);
    ASSERT_TRUE(gchild_element);
    ASSERT_EQ(24U,
              FPDF_StructElement_GetActualText(gchild_element, nullptr, 0));

    unsigned short buffer[12] = {};
    // Deliberately pass in a small buffer size to make sure |buffer| remains
    // untouched.
    ASSERT_EQ(24U, FPDF_StructElement_GetActualText(gchild_element, buffer, 1));
    for (size_t i = 0; i < std::size(buffer); ++i)
      EXPECT_EQ(0U, buffer[i]);
    ASSERT_EQ(24U, FPDF_StructElement_GetActualText(gchild_element, buffer,
                                                    sizeof(buffer)));
    EXPECT_EQ(L"Actual Text", GetPlatformWString(buffer));
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

    // The table can be retrieved, even though it does not have /Type.
    ASSERT_EQ(1, FPDF_StructElement_CountChildren(document));
    FPDF_STRUCTELEMENT table = FPDF_StructElement_GetChildAtIndex(document, 0);
    ASSERT_TRUE(table);

    EXPECT_EQ(12U, FPDF_StructElement_GetType(table, buffer, kBufLen));
    EXPECT_EQ("Table", GetPlatformString(buffer));

    // The table entry cannot be retrieved, as the element is malformed.
    EXPECT_EQ(0U, FPDF_StructElement_GetStringAttribute(table, "Summary",
                                                        buffer, kBufLen));

    // The row can be retrieved, even though it had an invalid /Type.
    ASSERT_EQ(1, FPDF_StructElement_CountChildren(table));
    FPDF_STRUCTELEMENT row = FPDF_StructElement_GetChildAtIndex(table, 0);
    EXPECT_TRUE(row);
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

    // Nullptr test
    EXPECT_EQ(0U, FPDF_StructElement_GetLang(nullptr, buffer, kBufLen));

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

// See also FPDFEditEmbedderTest.TraverseMarkedContentID, which traverses the
// marked contents using FPDFPageObj_GetMark() and related API.
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

TEST_F(FPDFStructTreeEmbedderTest, GetMarkedContentIdAtIndex) {
  ASSERT_TRUE(OpenDocument("tagged_marked_content.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
    ASSERT_EQ(4, FPDF_StructTree_CountChildren(struct_tree.get()));

    // K is an integer MCID
    FPDF_STRUCTELEMENT child1 =
        FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 0);
    ASSERT_TRUE(child1);
    // Legacy API
    EXPECT_EQ(0, FPDF_StructElement_GetMarkedContentID(child1));

    // K is a dict containing MCR object reference
    FPDF_STRUCTELEMENT child2 =
        FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 1);
    ASSERT_TRUE(child2);

    // K is an array containing dict MCR object reference and integer MCID
    FPDF_STRUCTELEMENT child3 =
        FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 2);
    ASSERT_TRUE(child3);

    // K does not exist
    FPDF_STRUCTELEMENT child4 =
        FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 3);
    ASSERT_TRUE(child4);

    // New APIs
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentIdCount(nullptr));
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentIdAtIndex(nullptr, 0));
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentIdAtIndex(child1, -1));
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentIdAtIndex(child1, 1));
    EXPECT_EQ(1, FPDF_StructElement_GetMarkedContentIdCount(child1));
    EXPECT_EQ(0, FPDF_StructElement_GetMarkedContentIdAtIndex(child1, 0));

    EXPECT_EQ(1, FPDF_StructElement_GetMarkedContentIdCount(child2));
    EXPECT_EQ(1, FPDF_StructElement_GetMarkedContentIdAtIndex(child2, 0));

    EXPECT_EQ(2, FPDF_StructElement_GetMarkedContentIdCount(child3));
    EXPECT_EQ(2, FPDF_StructElement_GetMarkedContentIdAtIndex(child3, 0));
    EXPECT_EQ(3, FPDF_StructElement_GetMarkedContentIdAtIndex(child3, 1));

    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentIdCount(child4));
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentIdAtIndex(child4, 0));
  }

  UnloadPage(page);
}

TEST_F(FPDFStructTreeEmbedderTest, GetChildMarkedContentID) {
  ASSERT_TRUE(OpenDocument("tagged_mcr_multipage.pdf"));

  // Using the loop to make difference clear
  for (int page_i : {0, 1}) {
    FPDF_PAGE page = LoadPage(page_i);
    ASSERT_TRUE(page);
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
    ASSERT_EQ(1, FPDF_StructTree_CountChildren(struct_tree.get()));

    FPDF_STRUCTELEMENT struct_doc =
        FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 0);
    ASSERT_TRUE(struct_doc);
    EXPECT_EQ(-1, FPDF_StructElement_GetMarkedContentID(struct_doc));

    ASSERT_EQ(2, FPDF_StructElement_CountChildren(struct_doc));
    FPDF_STRUCTELEMENT child1 =
        FPDF_StructElement_GetChildAtIndex(struct_doc, 0);
    EXPECT_FALSE(child1);
    FPDF_STRUCTELEMENT child2 =
        FPDF_StructElement_GetChildAtIndex(struct_doc, 1);
    EXPECT_FALSE(child2);

    EXPECT_EQ(2, FPDF_StructElement_GetMarkedContentIdCount(struct_doc));

    // Both MCID are returned as if part of this page, while they are not.
    // So `FPDF_StructElement_GetMarkedContentIdAtIndex(...)` does not work
    // for StructElement spanning multiple pages.
    EXPECT_EQ(0, FPDF_StructElement_GetMarkedContentIdAtIndex(struct_doc, 0));
    EXPECT_EQ(0, FPDF_StructElement_GetMarkedContentIdAtIndex(struct_doc, 1));

    // One MCR is pointing to page 1, another to page2, so those are different
    // for different pages.
    EXPECT_EQ(page_i == 0 ? 0 : -1,
              FPDF_StructElement_GetChildMarkedContentID(struct_doc, 0));
    EXPECT_EQ(page_i == 1 ? 0 : -1,
              FPDF_StructElement_GetChildMarkedContentID(struct_doc, 1));
    // Invalid index
    EXPECT_EQ(-1, FPDF_StructElement_GetChildMarkedContentID(struct_doc, -1));
    EXPECT_EQ(-1, FPDF_StructElement_GetChildMarkedContentID(struct_doc, 2));
    // Invalid element
    EXPECT_EQ(-1, FPDF_StructElement_GetChildMarkedContentID(nullptr, 0));
    UnloadPage(page);
  }
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
    for (size_t i = 0; i < std::size(buffer); ++i)
      EXPECT_EQ(0U, buffer[i]);

    ASSERT_EQ(18U, FPDF_StructElement_GetType(element, buffer, sizeof(buffer)));
    EXPECT_EQ(L"Document", GetPlatformWString(buffer));
  }

  UnloadPage(page);
}

TEST_F(FPDFStructTreeEmbedderTest, GetObjType) {
  ASSERT_TRUE(OpenDocument("tagged_table_bad_elem.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
    ASSERT_EQ(1, FPDF_StructTree_CountChildren(struct_tree.get()));

    FPDF_STRUCTELEMENT child =
        FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 0);
    ASSERT_TRUE(child);

    // test nullptr inputs
    unsigned short buffer[28] = {};
    ASSERT_EQ(0U,
              FPDF_StructElement_GetObjType(nullptr, buffer, sizeof(buffer)));
    ASSERT_EQ(0U, FPDF_StructElement_GetObjType(nullptr, nullptr, 0));
    ASSERT_EQ(22U, FPDF_StructElement_GetObjType(child, nullptr, 0));

    // Deliberately pass in a small buffer size to make sure `buffer` remains
    // untouched.
    ASSERT_EQ(22U, FPDF_StructElement_GetObjType(child, buffer, 1));
    for (size_t i = 0; i < std::size(buffer); ++i)
      EXPECT_EQ(0U, buffer[i]);

    ASSERT_EQ(22U,
              FPDF_StructElement_GetObjType(child, buffer, sizeof(buffer)));
    EXPECT_EQ(L"StructElem", GetPlatformWString(buffer));

    ASSERT_EQ(1, FPDF_StructElement_CountChildren(child));
    FPDF_STRUCTELEMENT gchild = FPDF_StructElement_GetChildAtIndex(child, 0);
    memset(buffer, 0, sizeof(buffer));
    // Missing /Type in `gchild`
    ASSERT_EQ(0U,
              FPDF_StructElement_GetObjType(gchild, buffer, sizeof(buffer)));
    // Buffer is untouched.
    for (size_t i = 0; i < std::size(buffer); ++i)
      EXPECT_EQ(0U, buffer[i]);

    ASSERT_EQ(1, FPDF_StructElement_CountChildren(gchild));
    FPDF_STRUCTELEMENT ggchild = FPDF_StructElement_GetChildAtIndex(gchild, 0);
    ASSERT_EQ(28U,
              FPDF_StructElement_GetObjType(ggchild, buffer, sizeof(buffer)));
    // Reading bad elem also works.
    EXPECT_EQ(L"NotStructElem", GetPlatformWString(buffer));
  }

  UnloadPage(page);
}

TEST_F(FPDFStructTreeEmbedderTest, GetParent) {
  ASSERT_TRUE(OpenDocument("tagged_alt_text.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
    ASSERT_EQ(1, FPDF_StructTree_CountChildren(struct_tree.get()));

    FPDF_STRUCTELEMENT parent =
        FPDF_StructTree_GetChildAtIndex(struct_tree.get(), 0);
    ASSERT_TRUE(parent);

    ASSERT_EQ(1, FPDF_StructElement_CountChildren(parent));

    FPDF_STRUCTELEMENT child = FPDF_StructElement_GetChildAtIndex(parent, 0);
    ASSERT_TRUE(child);

    // test nullptr inputs
    ASSERT_EQ(nullptr, FPDF_StructElement_GetParent(nullptr));

    ASSERT_EQ(parent, FPDF_StructElement_GetParent(child));

    // The parent of `parent` is StructTreeRoot and no longer a StructElement.
    // We currently handle this case by returning a nullptr.
    ASSERT_EQ(nullptr, FPDF_StructElement_GetParent(parent));
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
    for (size_t i = 0; i < std::size(buffer); ++i)
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

TEST_F(FPDFStructTreeEmbedderTest, GetAttributes) {
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

    ASSERT_EQ(1, FPDF_StructElement_CountChildren(document));
    ASSERT_EQ(-1, FPDF_StructElement_GetAttributeCount(document));
    FPDF_STRUCTELEMENT table = FPDF_StructElement_GetChildAtIndex(document, 0);
    ASSERT_TRUE(table);

    ASSERT_EQ(2, FPDF_StructElement_CountChildren(table));

    {
      FPDF_STRUCTELEMENT tr = FPDF_StructElement_GetChildAtIndex(table, 0);
      ASSERT_TRUE(tr);

      ASSERT_EQ(2, FPDF_StructElement_CountChildren(tr));
      FPDF_STRUCTELEMENT th = FPDF_StructElement_GetChildAtIndex(tr, 0);
      ASSERT_TRUE(th);

      ASSERT_EQ(2, FPDF_StructElement_GetAttributeCount(th));

      // nullptr test
      ASSERT_EQ(nullptr, FPDF_StructElement_GetAttributeAtIndex(document, 0));
      ASSERT_EQ(nullptr, FPDF_StructElement_GetAttributeAtIndex(document, -1));
      ASSERT_EQ(nullptr, FPDF_StructElement_GetAttributeAtIndex(th, 2));

      FPDF_STRUCTELEMENT_ATTR attr =
          FPDF_StructElement_GetAttributeAtIndex(th, 1);
      ASSERT_TRUE(attr);

      ASSERT_EQ(2, FPDF_StructElement_Attr_GetCount(attr));
      ASSERT_FALSE(
          FPDF_StructElement_Attr_GetName(attr, 1, nullptr, 0U, nullptr));
      unsigned long buffer_len_needed = ULONG_MAX;
      // Pass buffer = nullptr to obtain the size of the buffer needed,
      ASSERT_TRUE(FPDF_StructElement_Attr_GetName(attr, 1, nullptr, 0,
                                                  &buffer_len_needed));
      EXPECT_EQ(2U, buffer_len_needed);
      char buffer[8] = {};
      unsigned long out_len = ULONG_MAX;
      // Deliberately pass in a small buffer size to make sure `buffer` remains
      // untouched.
      ASSERT_TRUE(
          FPDF_StructElement_Attr_GetName(attr, 1, buffer, 1, &out_len));
      EXPECT_EQ(2U, out_len);
      for (size_t i = 0; i < std::size(buffer); ++i)
        EXPECT_EQ(0, buffer[i]);

      ASSERT_TRUE(FPDF_StructElement_Attr_GetName(attr, 1, buffer,
                                                  sizeof(buffer), &out_len));
      EXPECT_EQ(2U, out_len);
      EXPECT_STREQ("O", buffer);
      EXPECT_EQ(FPDF_OBJECT_NAME,
                FPDF_StructElement_Attr_GetType(attr, buffer));

      unsigned short str_val[12] = {};
      ASSERT_TRUE(FPDF_StructElement_Attr_GetStringValue(
          attr, buffer, str_val, sizeof(str_val), &out_len));
      EXPECT_EQ(12U, out_len);
      EXPECT_EQ(L"Table", GetPlatformWString(str_val));

      memset(buffer, 0, sizeof(buffer));
      ASSERT_TRUE(FPDF_StructElement_Attr_GetName(attr, 0, buffer,
                                                  sizeof(buffer), &out_len));
      EXPECT_EQ(8U, out_len);
      EXPECT_STREQ("ColSpan", buffer);
      EXPECT_EQ(FPDF_OBJECT_NUMBER,
                FPDF_StructElement_Attr_GetType(attr, buffer));
      float num_val;
      ASSERT_TRUE(
          FPDF_StructElement_Attr_GetNumberValue(attr, buffer, &num_val));
      EXPECT_FLOAT_EQ(2.0f, num_val);
    }

    {
      FPDF_STRUCTELEMENT tr = FPDF_StructElement_GetChildAtIndex(table, 1);
      ASSERT_TRUE(tr);

      ASSERT_EQ(1, FPDF_StructElement_GetAttributeCount(tr));
      // nullptr when index out of range
      ASSERT_EQ(nullptr, FPDF_StructElement_GetAttributeAtIndex(tr, 1));

      ASSERT_EQ(2, FPDF_StructElement_CountChildren(tr));
      FPDF_STRUCTELEMENT td = FPDF_StructElement_GetChildAtIndex(tr, 1);
      ASSERT_TRUE(td);
      {
        // Test counting and obtaining attributes via reference
        ASSERT_EQ(1, FPDF_StructElement_GetAttributeCount(td));
        FPDF_STRUCTELEMENT_ATTR attr =
            FPDF_StructElement_GetAttributeAtIndex(td, 0);
        ASSERT_TRUE(attr);
        ASSERT_EQ(4, FPDF_StructElement_Attr_GetCount(attr));
        // Test string and blob type
        {
          char buffer[16] = {};
          unsigned long out_len = ULONG_MAX;
          ASSERT_TRUE(FPDF_StructElement_Attr_GetName(
              attr, 0, buffer, sizeof(buffer), &out_len));
          EXPECT_EQ(8U, out_len);
          EXPECT_STREQ("ColProp", buffer);

          EXPECT_EQ(FPDF_OBJECT_STRING,
                    FPDF_StructElement_Attr_GetType(attr, buffer));

          unsigned short str_val[12] = {};
          ASSERT_TRUE(FPDF_StructElement_Attr_GetStringValue(
              attr, buffer, str_val, sizeof(str_val), &out_len));
          EXPECT_EQ(8U, out_len);
          EXPECT_EQ(L"Sum", GetPlatformWString(str_val));

          char blob_val[3] = {};
          ASSERT_TRUE(FPDF_StructElement_Attr_GetBlobValue(
              attr, buffer, blob_val, sizeof(blob_val), &out_len));
          EXPECT_EQ(3U, out_len);
          EXPECT_EQ('S', blob_val[0]);
          EXPECT_EQ('u', blob_val[1]);
          EXPECT_EQ('m', blob_val[2]);
        }

        // Test boolean type
        {
          char buffer[16] = {};
          unsigned long out_len = ULONG_MAX;
          ASSERT_TRUE(FPDF_StructElement_Attr_GetName(
              attr, 1, buffer, sizeof(buffer), &out_len));
          EXPECT_EQ(7U, out_len);
          EXPECT_STREQ("CurUSD", buffer);

          EXPECT_EQ(FPDF_OBJECT_BOOLEAN,
                    FPDF_StructElement_Attr_GetType(attr, buffer));
          FPDF_BOOL val;
          ASSERT_TRUE(
              FPDF_StructElement_Attr_GetBooleanValue(attr, buffer, &val));
          EXPECT_TRUE(val);
        }

        // Test reference to number
        {
          char buffer[16] = {};
          unsigned long out_len = ULONG_MAX;
          ASSERT_TRUE(FPDF_StructElement_Attr_GetName(
              attr, 3, buffer, sizeof(buffer), &out_len));
          EXPECT_EQ(8U, out_len);
          EXPECT_STREQ("RowSpan", buffer);

          EXPECT_EQ(FPDF_OBJECT_REFERENCE,
                    FPDF_StructElement_Attr_GetType(attr, buffer));
          float val;
          ASSERT_TRUE(
              FPDF_StructElement_Attr_GetNumberValue(attr, buffer, &val));
          EXPECT_FLOAT_EQ(3, val);
        }
      }
    }
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

TEST_F(FPDFStructTreeEmbedderTest, Bug1296920) {
  ASSERT_TRUE(OpenDocument("bug_1296920.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
    ASSERT_EQ(1, FPDF_StructTree_CountChildren(struct_tree.get()));

    // Destroying this tree should not crash.
  }

  UnloadPage(page);
}

TEST_F(FPDFStructTreeEmbedderTest, Bug1443100) {
  ASSERT_TRUE(OpenDocument("tagged_table_bad_parent.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Calling these APIs should not trigger a dangling pointer.
    ScopedFPDFStructTree struct_tree(FPDF_StructTree_GetForPage(page));
    ASSERT_TRUE(struct_tree);
    ASSERT_EQ(1, FPDF_StructTree_CountChildren(struct_tree.get()));
  }

  UnloadPage(page);
}
