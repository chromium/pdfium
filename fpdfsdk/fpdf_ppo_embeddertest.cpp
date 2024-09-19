// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_formobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_edit.h"
#include "public/fpdf_ppo.h"
#include "public/fpdf_save.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_constants.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/utils/file_util.h"
#include "testing/utils/path_service.h"

namespace {

class FPDFPPOEmbedderTest : public EmbedderTest {};

int FakeBlockWriter(FPDF_FILEWRITE* pThis,
                    const void* pData,
                    unsigned long size) {
  return 1;  // Always succeeds.
}

constexpr int kRectanglesMultiPagesPageCount = 2;

const char* RectanglesMultiPagesExpectedChecksum(int page_index) {
  if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
    static constexpr std::array<const char*, kRectanglesMultiPagesPageCount>
        kChecksums = {{"07606a12487bd0c28a88f23fa00fc313",
                       "94ea6e1eef220833a3ec14d6a1c612b0"}};
    return kChecksums[page_index];
  }
  static constexpr std::array<const char*, kRectanglesMultiPagesPageCount>
      kChecksums = {{"72d0d7a19a2f40e010ca6a1133b33e1e",
                     "fb18142190d770cfbc329d2b071aee4d"}};
  return kChecksums[page_index];
}

const char* Bug750568PageHash(int page_index) {
  constexpr int kBug750568PageCount = 4;
  if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
    static constexpr std::array<const char*, kBug750568PageCount> kChecksums = {
        {"eaa139e944eafb43d31e8742a0e158de", "226485e9d4fa6a67dfe0a88723f12060",
         "c5601a3492ae5dcc5dd25140fc463bfe",
         "1f60055b54de4fac8a59c65e90da156e"}};
    return kChecksums[page_index];
  }
  static constexpr std::array<const char*, kBug750568PageCount> kChecksums = {
      {"64ad08132a1c5a166768298c8a578f57", "83b83e2f6bc80707d0a917c7634140b9",
       "913cd3723a451e4e46fbc2c05702d1ee", "81fb7cfd4860f855eb468f73dfeb6d60"}};
  return kChecksums[page_index];
}

}  // namespace

TEST_F(FPDFPPOEmbedderTest, NoViewerPreferences) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));

  ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
  EXPECT_TRUE(output_doc);
  EXPECT_FALSE(FPDF_CopyViewerPreferences(output_doc.get(), document()));
}

TEST_F(FPDFPPOEmbedderTest, ViewerPreferences) {
  ASSERT_TRUE(OpenDocument("viewer_ref.pdf"));

  ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
  EXPECT_TRUE(output_doc);
  EXPECT_TRUE(FPDF_CopyViewerPreferences(output_doc.get(), document()));
}

TEST_F(FPDFPPOEmbedderTest, ImportPagesByIndex) {
  ASSERT_TRUE(OpenDocument("viewer_ref.pdf"));

  ScopedEmbedderTestPage page = LoadScopedPage(0);
  EXPECT_TRUE(page);

  ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
  ASSERT_TRUE(output_doc);
  EXPECT_TRUE(FPDF_CopyViewerPreferences(output_doc.get(), document()));

  static constexpr int kPageIndices[] = {1};
  EXPECT_TRUE(FPDF_ImportPagesByIndex(
      output_doc.get(), document(), kPageIndices, std::size(kPageIndices), 0));
  EXPECT_EQ(1, FPDF_GetPageCount(output_doc.get()));
}

TEST_F(FPDFPPOEmbedderTest, ImportPages) {
  ASSERT_TRUE(OpenDocument("viewer_ref.pdf"));

  ScopedEmbedderTestPage page = LoadScopedPage(0);
  EXPECT_TRUE(page);

  ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
  ASSERT_TRUE(output_doc);
  EXPECT_TRUE(FPDF_CopyViewerPreferences(output_doc.get(), document()));
  EXPECT_TRUE(FPDF_ImportPages(output_doc.get(), document(), "1", 0));
  EXPECT_EQ(1, FPDF_GetPageCount(output_doc.get()));
}

TEST_F(FPDFPPOEmbedderTest, ImportNPages) {
  ASSERT_TRUE(OpenDocument("rectangles_multi_pages.pdf"));

  ScopedFPDFDocument output_doc_2up(
      FPDF_ImportNPagesToOne(document(), 612, 792, 2, 1));
  ASSERT_TRUE(output_doc_2up);
  EXPECT_EQ(3, FPDF_GetPageCount(output_doc_2up.get()));
  ScopedFPDFDocument output_doc_5up(
      FPDF_ImportNPagesToOne(document(), 612, 792, 5, 1));
  ASSERT_TRUE(output_doc_5up);
  EXPECT_EQ(1, FPDF_GetPageCount(output_doc_5up.get()));
  ScopedFPDFDocument output_doc_8up(
      FPDF_ImportNPagesToOne(document(), 792, 612, 8, 1));
  ASSERT_TRUE(output_doc_8up);
  EXPECT_EQ(1, FPDF_GetPageCount(output_doc_8up.get()));
  ScopedFPDFDocument output_doc_128up(
      FPDF_ImportNPagesToOne(document(), 792, 612, 128, 1));
  ASSERT_TRUE(output_doc_128up);
  EXPECT_EQ(1, FPDF_GetPageCount(output_doc_128up.get()));
}

TEST_F(FPDFPPOEmbedderTest, BadNupParams) {
  ASSERT_TRUE(OpenDocument("rectangles_multi_pages.pdf"));

  FPDF_DOCUMENT output_doc_zero_row =
      FPDF_ImportNPagesToOne(document(), 612, 792, 0, 3);
  ASSERT_FALSE(output_doc_zero_row);
  FPDF_DOCUMENT output_doc_zero_col =
      FPDF_ImportNPagesToOne(document(), 612, 792, 2, 0);
  ASSERT_FALSE(output_doc_zero_col);
  FPDF_DOCUMENT output_doc_zero_width =
      FPDF_ImportNPagesToOne(document(), 0, 792, 2, 1);
  ASSERT_FALSE(output_doc_zero_width);
  FPDF_DOCUMENT output_doc_zero_height =
      FPDF_ImportNPagesToOne(document(), 612, 0, 7, 1);
  ASSERT_FALSE(output_doc_zero_height);
}

// TODO(Xlou): Add more tests to check output doc content of
// FPDF_ImportNPagesToOne()
TEST_F(FPDFPPOEmbedderTest, NupRenderImage) {
  ASSERT_TRUE(OpenDocument("rectangles_multi_pages.pdf"));
  ScopedFPDFDocument output_doc_3up(
      FPDF_ImportNPagesToOne(document(), 792, 612, 3, 1));
  ASSERT_TRUE(output_doc_3up);
  ASSERT_EQ(kRectanglesMultiPagesPageCount,
            FPDF_GetPageCount(output_doc_3up.get()));
  for (int i = 0; i < kRectanglesMultiPagesPageCount; ++i) {
    ScopedFPDFPage page(FPDF_LoadPage(output_doc_3up.get(), i));
    ASSERT_TRUE(page);
    ScopedFPDFBitmap bitmap = RenderPage(page.get());
    EXPECT_EQ(792, FPDFBitmap_GetWidth(bitmap.get()));
    EXPECT_EQ(612, FPDFBitmap_GetHeight(bitmap.get()));
    EXPECT_EQ(RectanglesMultiPagesExpectedChecksum(i),
              HashBitmap(bitmap.get()));
  }
}

TEST_F(FPDFPPOEmbedderTest, ImportPageToXObject) {
  const char* checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "d6ebc0a8afc22fe0137f54ce54e1a19c";
    }
    return "2d88d180af7109eb346439f7c855bb29";
  }();

  ASSERT_TRUE(OpenDocument("rectangles.pdf"));

  {
    ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
    ASSERT_TRUE(output_doc);

    FPDF_XOBJECT xobject =
        FPDF_NewXObjectFromPage(output_doc.get(), document(), 0);
    ASSERT_TRUE(xobject);

    for (int i = 0; i < 2; ++i) {
      ScopedFPDFPage page(FPDFPage_New(output_doc.get(), 0, 612, 792));
      ASSERT_TRUE(page);

      FPDF_PAGEOBJECT page_object = FPDF_NewFormObjectFromXObject(xobject);
      ASSERT_TRUE(page_object);
      EXPECT_EQ(FPDF_PAGEOBJ_FORM, FPDFPageObj_GetType(page_object));
      FPDFPage_InsertObject(page.get(), page_object);
      EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));

      ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
      CompareBitmap(page_bitmap.get(), 612, 792, checksum);

      float left;
      float bottom;
      float right;
      float top;
      ASSERT_TRUE(
          FPDFPageObj_GetBounds(page_object, &left, &bottom, &right, &top));
      EXPECT_FLOAT_EQ(-1.0f, left);
      EXPECT_FLOAT_EQ(-1.0f, bottom);
      EXPECT_FLOAT_EQ(201.0f, right);
      EXPECT_FLOAT_EQ(301.0f, top);
    }

    EXPECT_TRUE(FPDF_SaveAsCopy(output_doc.get(), this, 0));

    FPDF_CloseXObject(xobject);
  }

  constexpr int kExpectedPageCount = 2;
  ASSERT_TRUE(OpenSavedDocument());

  std::array<FPDF_PAGE, kExpectedPageCount> saved_pages;
  std::array<FPDF_PAGEOBJECT, kExpectedPageCount> xobjects;
  for (int i = 0; i < kExpectedPageCount; ++i) {
    saved_pages[i] = LoadSavedPage(i);
    ASSERT_TRUE(saved_pages[i]);

    EXPECT_EQ(1, FPDFPage_CountObjects(saved_pages[i]));
    xobjects[i] = FPDFPage_GetObject(saved_pages[i], 0);
    ASSERT_TRUE(xobjects[i]);
    ASSERT_EQ(FPDF_PAGEOBJ_FORM, FPDFPageObj_GetType(xobjects[i]));
    EXPECT_EQ(8, FPDFFormObj_CountObjects(xobjects[i]));

    {
      ScopedFPDFBitmap page_bitmap = RenderPage(saved_pages[i]);
      CompareBitmap(page_bitmap.get(), 612, 792, checksum);
    }
  }

    for (int i = 0; i < kExpectedPageCount; ++i) {
      float left;
      float bottom;
      float right;
      float top;
      ASSERT_TRUE(
          FPDFPageObj_GetBounds(xobjects[i], &left, &bottom, &right, &top));
      EXPECT_FLOAT_EQ(-1.0f, left);
      EXPECT_FLOAT_EQ(-1.0f, bottom);
      EXPECT_FLOAT_EQ(201.0f, right);
      EXPECT_FLOAT_EQ(301.0f, top);
    }

    // Peek at object internals to make sure the two XObjects use the same
    // stream.
    EXPECT_NE(xobjects[0], xobjects[1]);
    CPDF_PageObject* obj1 = CPDFPageObjectFromFPDFPageObject(xobjects[0]);
    ASSERT_TRUE(obj1->AsForm());
    ASSERT_TRUE(obj1->AsForm()->form());
    ASSERT_TRUE(obj1->AsForm()->form()->GetStream());
    CPDF_PageObject* obj2 = CPDFPageObjectFromFPDFPageObject(xobjects[1]);
    ASSERT_TRUE(obj2->AsForm());
    ASSERT_TRUE(obj2->AsForm()->form());
    ASSERT_TRUE(obj2->AsForm()->form()->GetStream());
    EXPECT_EQ(obj1->AsForm()->form()->GetStream(),
              obj2->AsForm()->form()->GetStream());

  for (FPDF_PAGE saved_page : saved_pages)
    CloseSavedPage(saved_page);

  CloseSavedDocument();
}

TEST_F(FPDFPPOEmbedderTest, ImportPageToXObjectWithSameDoc) {
  const char* checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "8e7d672f49f9ca98fb9157824cefc204";
    }
    return "4d5ca14827b7707f8283e639b33c121a";
  }();

  ASSERT_TRUE(OpenDocument("rectangles.pdf"));

  FPDF_XOBJECT xobject = FPDF_NewXObjectFromPage(document(), document(), 0);
  ASSERT_TRUE(xobject);

  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), 200, 300, pdfium::RectanglesChecksum());
  }

  FPDF_PAGEOBJECT page_object = FPDF_NewFormObjectFromXObject(xobject);
  ASSERT_TRUE(page_object);
  ASSERT_EQ(FPDF_PAGEOBJ_FORM, FPDFPageObj_GetType(page_object));

  static constexpr FS_MATRIX kMatrix = {0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f};
  EXPECT_TRUE(FPDFPageObj_SetMatrix(page_object, &kMatrix));

  FPDFPage_InsertObject(page.get(), page_object);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), 200, 300, checksum);
  }

  FPDF_CloseXObject(xobject);

  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedDocument(200, 300, checksum);
}

TEST_F(FPDFPPOEmbedderTest, XObjectNullParams) {
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));
  ASSERT_EQ(1, FPDF_GetPageCount(document()));

  EXPECT_FALSE(FPDF_NewXObjectFromPage(nullptr, nullptr, -1));
  EXPECT_FALSE(FPDF_NewXObjectFromPage(nullptr, nullptr, 0));
  EXPECT_FALSE(FPDF_NewXObjectFromPage(nullptr, nullptr, 1));
  EXPECT_FALSE(FPDF_NewXObjectFromPage(document(), nullptr, -1));
  EXPECT_FALSE(FPDF_NewXObjectFromPage(document(), nullptr, 0));
  EXPECT_FALSE(FPDF_NewXObjectFromPage(document(), nullptr, 1));
  EXPECT_FALSE(FPDF_NewXObjectFromPage(nullptr, document(), -1));
  EXPECT_FALSE(FPDF_NewXObjectFromPage(nullptr, document(), 0));
  EXPECT_FALSE(FPDF_NewXObjectFromPage(nullptr, document(), 1));

  {
    ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
    ASSERT_TRUE(output_doc);
    EXPECT_FALSE(FPDF_NewXObjectFromPage(output_doc.get(), document(), -1));
    EXPECT_FALSE(FPDF_NewXObjectFromPage(output_doc.get(), document(), 1));
  }

  // Should be a no-op.
  FPDF_CloseXObject(nullptr);

  EXPECT_FALSE(FPDF_NewFormObjectFromXObject(nullptr));
}

TEST_F(FPDFPPOEmbedderTest, Bug925981) {
  ASSERT_TRUE(OpenDocument("bug_925981.pdf"));
  ScopedFPDFDocument output_doc_2up(
      FPDF_ImportNPagesToOne(document(), 612, 792, 2, 1));
  EXPECT_EQ(1, FPDF_GetPageCount(output_doc_2up.get()));
}

TEST_F(FPDFPPOEmbedderTest, Bug1229106) {
  static constexpr int kPageCount = 4;
  static constexpr int kTwoUpPageCount = 2;
  static const char kRectsChecksum[] = "140d629b3c96a07ced2e3e408ea85a1d";
  static const char kTwoUpChecksum[] = "fa4501562301b2e75da354bd067495ec";

  ASSERT_TRUE(OpenDocument("bug_1229106.pdf"));

  // Show all pages render the same.
  ASSERT_EQ(kPageCount, FPDF_GetPageCount(document()));
  for (int i = 0; i < kPageCount; ++i) {
    ScopedEmbedderTestPage page = LoadScopedPage(0);
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), 792, 612, kRectsChecksum);
  }

  // Create a 2-up PDF.
  ScopedFPDFDocument output_doc_2up(
      FPDF_ImportNPagesToOne(document(), 612, 792, 1, 2));
  ASSERT_EQ(kTwoUpPageCount, FPDF_GetPageCount(output_doc_2up.get()));
  for (int i = 0; i < kTwoUpPageCount; ++i) {
    ScopedFPDFPage page(FPDF_LoadPage(output_doc_2up.get(), i));
    ScopedFPDFBitmap bitmap = RenderPage(page.get());
    CompareBitmap(bitmap.get(), 612, 792, kTwoUpChecksum);
  }
}

TEST_F(FPDFPPOEmbedderTest, BadRepeatViewerPref) {
  ASSERT_TRUE(OpenDocument("repeat_viewer_ref.pdf"));

  ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
  EXPECT_TRUE(output_doc);
  EXPECT_TRUE(FPDF_CopyViewerPreferences(output_doc.get(), document()));

  FPDF_FILEWRITE writer;
  writer.version = 1;
  writer.WriteBlock = FakeBlockWriter;

  EXPECT_TRUE(FPDF_SaveAsCopy(output_doc.get(), &writer, 0));
}

TEST_F(FPDFPPOEmbedderTest, BadCircularViewerPref) {
  ASSERT_TRUE(OpenDocument("circular_viewer_ref.pdf"));

  ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
  EXPECT_TRUE(output_doc);
  EXPECT_TRUE(FPDF_CopyViewerPreferences(output_doc.get(), document()));

  FPDF_FILEWRITE writer;
  writer.version = 1;
  writer.WriteBlock = FakeBlockWriter;

  EXPECT_TRUE(FPDF_SaveAsCopy(output_doc.get(), &writer, 0));
}

TEST_F(FPDFPPOEmbedderTest, CopyViewerPrefTypes) {
  ASSERT_TRUE(OpenDocument("viewer_pref_types.pdf"));

  ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
  ASSERT_TRUE(output_doc);
  EXPECT_TRUE(FPDF_CopyViewerPreferences(output_doc.get(), document()));

  // Peek under the hook to check the result.
  const CPDF_Document* output_doc_impl =
      CPDFDocumentFromFPDFDocument(output_doc.get());
  RetainPtr<const CPDF_Dictionary> prefs =
      output_doc_impl->GetRoot()->GetDictFor("ViewerPreferences");
  ASSERT_TRUE(prefs);
  EXPECT_EQ(6u, prefs->size());

  RetainPtr<const CPDF_Object> bool_obj = prefs->GetObjectFor("Bool");
  ASSERT_TRUE(bool_obj);
  EXPECT_TRUE(bool_obj->IsBoolean());

  RetainPtr<const CPDF_Number> num_obj = prefs->GetNumberFor("Num");
  ASSERT_TRUE(num_obj);
  EXPECT_TRUE(num_obj->IsInteger());
  EXPECT_EQ(1, num_obj->GetInteger());

  RetainPtr<const CPDF_String> str_obj = prefs->GetStringFor("Str");
  ASSERT_TRUE(str_obj);
  EXPECT_EQ("str", str_obj->GetString());

  EXPECT_EQ("name", prefs->GetNameFor("Name"));

  RetainPtr<const CPDF_Array> empty_array_obj =
      prefs->GetArrayFor("EmptyArray");
  ASSERT_TRUE(empty_array_obj);
  EXPECT_TRUE(empty_array_obj->IsEmpty());

  RetainPtr<const CPDF_Array> good_array_obj = prefs->GetArrayFor("GoodArray");
  ASSERT_TRUE(good_array_obj);
  EXPECT_EQ(4u, good_array_obj->size());
}

TEST_F(FPDFPPOEmbedderTest, BadIndices) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));

  ScopedEmbedderTestPage page = LoadScopedPage(0);
  EXPECT_TRUE(page);

  ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
  EXPECT_TRUE(output_doc);

  static constexpr int kBadIndices1[] = {-1};
  EXPECT_FALSE(FPDF_ImportPagesByIndex(
      output_doc.get(), document(), kBadIndices1, std::size(kBadIndices1), 0));

  static constexpr int kBadIndices2[] = {1};
  EXPECT_FALSE(FPDF_ImportPagesByIndex(
      output_doc.get(), document(), kBadIndices2, std::size(kBadIndices2), 0));

  static constexpr int kBadIndices3[] = {-1, 0, 1};
  EXPECT_FALSE(FPDF_ImportPagesByIndex(
      output_doc.get(), document(), kBadIndices3, std::size(kBadIndices3), 0));

  static constexpr int kBadIndices4[] = {42};
  EXPECT_FALSE(FPDF_ImportPagesByIndex(
      output_doc.get(), document(), kBadIndices4, std::size(kBadIndices4), 0));
}

TEST_F(FPDFPPOEmbedderTest, GoodIndices) {
  ASSERT_TRUE(OpenDocument("viewer_ref.pdf"));

  ScopedEmbedderTestPage page = LoadScopedPage(0);
  EXPECT_TRUE(page);

  ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
  EXPECT_TRUE(output_doc);

  static constexpr int kGoodIndices1[] = {0, 0, 0, 0};
  EXPECT_TRUE(FPDF_ImportPagesByIndex(output_doc.get(), document(),
                                      kGoodIndices1, std::size(kGoodIndices1),
                                      0));
  EXPECT_EQ(4, FPDF_GetPageCount(output_doc.get()));

  static constexpr int kGoodIndices2[] = {0};
  EXPECT_TRUE(FPDF_ImportPagesByIndex(output_doc.get(), document(),
                                      kGoodIndices2, std::size(kGoodIndices2),
                                      0));
  EXPECT_EQ(5, FPDF_GetPageCount(output_doc.get()));

  static constexpr int kGoodIndices3[] = {4};
  EXPECT_TRUE(FPDF_ImportPagesByIndex(output_doc.get(), document(),
                                      kGoodIndices3, std::size(kGoodIndices3),
                                      0));
  EXPECT_EQ(6, FPDF_GetPageCount(output_doc.get()));

  static constexpr int kGoodIndices4[] = {1, 2, 3};
  EXPECT_TRUE(FPDF_ImportPagesByIndex(output_doc.get(), document(),
                                      kGoodIndices4, std::size(kGoodIndices4),
                                      0));
  EXPECT_EQ(9, FPDF_GetPageCount(output_doc.get()));

  // Passing in a nullptr should import all the pages.
  EXPECT_TRUE(
      FPDF_ImportPagesByIndex(output_doc.get(), document(), nullptr, 0, 0));
  EXPECT_EQ(14, FPDF_GetPageCount(output_doc.get()));
}

TEST_F(FPDFPPOEmbedderTest, BadRanges) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));

  ScopedEmbedderTestPage page = LoadScopedPage(0);
  EXPECT_TRUE(page);

  ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
  EXPECT_TRUE(output_doc);
  EXPECT_FALSE(FPDF_ImportPages(output_doc.get(), document(), "clams", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc.get(), document(), "0", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc.get(), document(), "42", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc.get(), document(), "1,2", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc.get(), document(), "1-2", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc.get(), document(), ",1", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc.get(), document(), "1,", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc.get(), document(), "1-", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc.get(), document(), "-1", 0));
  EXPECT_FALSE(FPDF_ImportPages(output_doc.get(), document(), "-,0,,,1-", 0));
}

TEST_F(FPDFPPOEmbedderTest, GoodRanges) {
  ASSERT_TRUE(OpenDocument("viewer_ref.pdf"));

  ScopedEmbedderTestPage page = LoadScopedPage(0);
  EXPECT_TRUE(page);

  ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
  EXPECT_TRUE(output_doc);
  EXPECT_TRUE(FPDF_CopyViewerPreferences(output_doc.get(), document()));
  EXPECT_TRUE(FPDF_ImportPages(output_doc.get(), document(), "1,1,1,1", 0));
  EXPECT_EQ(4, FPDF_GetPageCount(output_doc.get()));
  EXPECT_TRUE(FPDF_ImportPages(output_doc.get(), document(), "1-1", 0));
  EXPECT_EQ(5, FPDF_GetPageCount(output_doc.get()));
  EXPECT_TRUE(FPDF_ImportPages(output_doc.get(), document(), "5-5", 0));
  EXPECT_EQ(6, FPDF_GetPageCount(output_doc.get()));
  EXPECT_TRUE(FPDF_ImportPages(output_doc.get(), document(), "2-4", 0));
  EXPECT_EQ(9, FPDF_GetPageCount(output_doc.get()));
}

TEST_F(FPDFPPOEmbedderTest, Bug664284) {
  ASSERT_TRUE(OpenDocument("bug_664284.pdf"));

  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_NE(nullptr, page.get());

  ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
  EXPECT_TRUE(output_doc);

  static constexpr int kIndices[] = {0};
  EXPECT_TRUE(FPDF_ImportPagesByIndex(output_doc.get(), document(), kIndices,
                                      std::size(kIndices), 0));
}

TEST_F(FPDFPPOEmbedderTest, Bug750568) {
  ASSERT_TRUE(OpenDocument("bug_750568.pdf"));
  ASSERT_EQ(4, FPDF_GetPageCount(document()));

  for (size_t i = 0; i < 4; ++i) {
    ScopedEmbedderTestPage page = LoadScopedPage(i);
    ASSERT_TRUE(page);

    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), 200, 200, Bug750568PageHash(i));
  }

  ScopedFPDFDocument output_doc(FPDF_CreateNewDocument());
  ASSERT_TRUE(output_doc);

  static constexpr int kIndices[] = {0, 1, 2, 3};
  EXPECT_TRUE(FPDF_ImportPagesByIndex(output_doc.get(), document(), kIndices,
                                      std::size(kIndices), 0));
  ASSERT_EQ(4, FPDF_GetPageCount(output_doc.get()));
  for (size_t i = 0; i < 4; ++i) {
    ScopedFPDFPage page(FPDF_LoadPage(output_doc.get(), i));
    ASSERT_TRUE(page);

    ScopedFPDFBitmap bitmap = RenderPage(page.get());
    CompareBitmap(bitmap.get(), 200, 200, Bug750568PageHash(i));
  }
}

TEST_F(FPDFPPOEmbedderTest, ImportWithZeroLengthStream) {
  ASSERT_TRUE(OpenDocument("zero_length_stream.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
  CompareBitmap(bitmap.get(), 200, 200, pdfium::HelloWorldChecksum());

  ScopedFPDFDocument new_doc(FPDF_CreateNewDocument());
  ASSERT_TRUE(new_doc);

  static constexpr int kIndices[] = {0};
  EXPECT_TRUE(FPDF_ImportPagesByIndex(new_doc.get(), document(), kIndices,
                                      std::size(kIndices), 0));

  EXPECT_EQ(1, FPDF_GetPageCount(new_doc.get()));
  ScopedFPDFPage new_page(FPDF_LoadPage(new_doc.get(), 0));
  ASSERT_TRUE(new_page);
  ScopedFPDFBitmap new_bitmap = RenderPage(new_page.get());
  CompareBitmap(new_bitmap.get(), 200, 200, pdfium::HelloWorldChecksum());
}

TEST_F(FPDFPPOEmbedderTest, ImportIntoDestDocWithoutInfo) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_EQ(1, FPDF_GetPageCount(document()));

  std::string file_path = PathService::GetTestFilePath("rectangles.pdf");
  ASSERT_FALSE(file_path.empty());
  std::vector<uint8_t> file_contents = GetFileContents(file_path.c_str());
  ASSERT_FALSE(file_contents.empty());

  ScopedFPDFDocument src_doc(FPDF_LoadMemDocument(
      file_contents.data(), file_contents.size(), nullptr));
  ASSERT_TRUE(src_doc);

  static constexpr int kIndices[] = {0};
  EXPECT_TRUE(FPDF_ImportPagesByIndex(document(), src_doc.get(), kIndices,
                                      std::size(kIndices), 0));
  EXPECT_EQ(2, FPDF_GetPageCount(document()));

  EXPECT_TRUE(FPDF_ImportPages(document(), src_doc.get(), "1", 0));
  EXPECT_EQ(3, FPDF_GetPageCount(document()));
}

TEST_F(FPDFPPOEmbedderTest, ImportIntoDocWithWrongPageType) {
  ASSERT_TRUE(OpenDocument("bad_page_type.pdf"));
  EXPECT_EQ(2, FPDF_GetPageCount(document()));

  std::string file_path = PathService::GetTestFilePath("rectangles.pdf");
  ASSERT_FALSE(file_path.empty());
  std::vector<uint8_t> file_contents = GetFileContents(file_path.c_str());
  ASSERT_FALSE(file_contents.empty());

  ScopedFPDFDocument src_doc(FPDF_LoadMemDocument(
      file_contents.data(), file_contents.size(), nullptr));
  ASSERT_TRUE(src_doc);
  EXPECT_EQ(1, FPDF_GetPageCount(src_doc.get()));

  FPDFPage_Delete(document(), 0);
  EXPECT_EQ(1, FPDF_GetPageCount(document()));

  static constexpr int kPageIndices[] = {0};
  ASSERT_TRUE(FPDF_ImportPagesByIndex(document(), src_doc.get(), kPageIndices,
                                      std::size(kPageIndices), 0));
  EXPECT_EQ(2, FPDF_GetPageCount(document()));
  const char* const new_page_1_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "b4e411a6b5ffa59a50efede2efece597";
    }
    return "0a90de37f52127619c3dfb642b5fa2fe";
  }();
  const char new_page_2_checksum[] = "39336760026e7f3d26135e3b765125c3";
  {
    ScopedEmbedderTestPage page = LoadScopedPage(0);
    ASSERT_TRUE(page);
    ScopedFPDFBitmap bitmap = RenderPage(page.get());
    CompareBitmap(bitmap.get(), 200, 300, new_page_1_checksum);
  }
  {
    ScopedEmbedderTestPage page = LoadScopedPage(1);
    ASSERT_TRUE(page);
    ScopedFPDFBitmap bitmap = RenderPage(page.get());
    CompareBitmap(bitmap.get(), 200, 100, new_page_2_checksum);
  }

  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  ASSERT_TRUE(OpenSavedDocument());
  EXPECT_EQ(2, FPDF_GetPageCount(saved_document()));
  {
    FPDF_PAGE page = LoadSavedPage(0);
    ASSERT_TRUE(page);
    ScopedFPDFBitmap bitmap = RenderPage(page);
    CompareBitmap(bitmap.get(), 200, 300, new_page_1_checksum);
    CloseSavedPage(page);
  }
  {
    FPDF_PAGE page = LoadSavedPage(1);
    ASSERT_TRUE(page);
    ScopedFPDFBitmap bitmap = RenderPage(page);
    CompareBitmap(bitmap.get(), 200, 100, new_page_2_checksum);
    CloseSavedPage(page);
  }
}
