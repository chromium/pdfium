// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_document.h"

#include <memory>
#include <utility>

#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_pagemodule.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_boolean.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_linearized_header.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/render/cpdf_docrenderdata.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

namespace {

const int kNumTestPages = 7;

CPDF_Dictionary* CreatePageTreeNode(RetainPtr<CPDF_Array> kids,
                                    CPDF_Document* pDoc,
                                    int count) {
  CPDF_Array* pUnowned = pDoc->AddIndirectObject(std::move(kids))->AsArray();
  CPDF_Dictionary* pageNode = pDoc->NewIndirect<CPDF_Dictionary>();
  pageNode->SetNewFor<CPDF_Name>("Type", "Pages");
  pageNode->SetNewFor<CPDF_Reference>("Kids", pDoc, pUnowned->GetObjNum());
  pageNode->SetNewFor<CPDF_Number>("Count", count);
  for (size_t i = 0; i < pUnowned->size(); i++) {
    pUnowned->GetDictAt(i)->SetNewFor<CPDF_Reference>("Parent", pDoc,
                                                      pageNode->GetObjNum());
  }
  return pageNode;
}

RetainPtr<CPDF_Dictionary> CreateNumberedPage(size_t number) {
  auto page = pdfium::MakeRetain<CPDF_Dictionary>();
  page->SetNewFor<CPDF_Name>("Type", "Page");
  page->SetNewFor<CPDF_Number>("PageNumbering", static_cast<int>(number));
  return page;
}

class CPDF_TestDocumentForPages final : public CPDF_Document {
 public:
  CPDF_TestDocumentForPages()
      : CPDF_Document(pdfium::MakeUnique<CPDF_DocRenderData>(),
                      pdfium::MakeUnique<CPDF_DocPageData>()) {
    // Set up test
    auto zeroToTwo = pdfium::MakeRetain<CPDF_Array>();
    zeroToTwo->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(0))->GetObjNum());
    zeroToTwo->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(1))->GetObjNum());
    zeroToTwo->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(2))->GetObjNum());
    CPDF_Dictionary* branch1 =
        CreatePageTreeNode(std::move(zeroToTwo), this, 3);

    auto zeroToThree = pdfium::MakeRetain<CPDF_Array>();
    zeroToThree->AppendNew<CPDF_Reference>(this, branch1->GetObjNum());
    zeroToThree->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(3))->GetObjNum());
    CPDF_Dictionary* branch2 =
        CreatePageTreeNode(std::move(zeroToThree), this, 4);

    auto fourFive = pdfium::MakeRetain<CPDF_Array>();
    fourFive->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(4))->GetObjNum());
    fourFive->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(5))->GetObjNum());
    CPDF_Dictionary* branch3 = CreatePageTreeNode(std::move(fourFive), this, 2);

    auto justSix = pdfium::MakeRetain<CPDF_Array>();
    justSix->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(6))->GetObjNum());
    CPDF_Dictionary* branch4 = CreatePageTreeNode(std::move(justSix), this, 1);

    auto allPages = pdfium::MakeRetain<CPDF_Array>();
    allPages->AppendNew<CPDF_Reference>(this, branch2->GetObjNum());
    allPages->AppendNew<CPDF_Reference>(this, branch3->GetObjNum());
    allPages->AppendNew<CPDF_Reference>(this, branch4->GetObjNum());
    CPDF_Dictionary* pagesDict =
        CreatePageTreeNode(std::move(allPages), this, kNumTestPages);

    SetRootForTesting(NewIndirect<CPDF_Dictionary>());
    GetRoot()->SetNewFor<CPDF_Reference>("Pages", this, pagesDict->GetObjNum());
    ResizePageListForTesting(kNumTestPages);
  }

  void SetTreeSize(int size) {
    GetRoot()->SetNewFor<CPDF_Number>("Count", size);
    ResizePageListForTesting(size);
  }
};

class CPDF_TestDocumentWithPageWithoutPageNum final : public CPDF_Document {
 public:
  CPDF_TestDocumentWithPageWithoutPageNum()
      : CPDF_Document(pdfium::MakeUnique<CPDF_DocRenderData>(),
                      pdfium::MakeUnique<CPDF_DocPageData>()) {
    // Set up test
    auto allPages = pdfium::MakeRetain<CPDF_Array>();
    allPages->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(0))->GetObjNum());
    allPages->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(1))->GetObjNum());
    // Page without pageNum.
    inlined_page_ = allPages->Append(CreateNumberedPage(2));
    CPDF_Dictionary* pagesDict =
        CreatePageTreeNode(std::move(allPages), this, 3);
    SetRootForTesting(NewIndirect<CPDF_Dictionary>());
    GetRoot()->SetNewFor<CPDF_Reference>("Pages", this, pagesDict->GetObjNum());
    ResizePageListForTesting(3);
  }

  const CPDF_Object* inlined_page() const { return inlined_page_; }

 private:
  const CPDF_Object* inlined_page_;
};

class TestLinearized final : public CPDF_LinearizedHeader {
 public:
  explicit TestLinearized(CPDF_Dictionary* dict)
      : CPDF_LinearizedHeader(dict, 0) {}
};

class CPDF_TestDocPagesWithoutKids final : public CPDF_Document {
 public:
  CPDF_TestDocPagesWithoutKids()
      : CPDF_Document(pdfium::MakeUnique<CPDF_DocRenderData>(),
                      pdfium::MakeUnique<CPDF_DocPageData>()) {
    CPDF_Dictionary* pagesDict = NewIndirect<CPDF_Dictionary>();
    pagesDict->SetNewFor<CPDF_Name>("Type", "Pages");
    pagesDict->SetNewFor<CPDF_Number>("Count", 3);
    ResizePageListForTesting(10);
    SetRootForTesting(NewIndirect<CPDF_Dictionary>());
    GetRoot()->SetNewFor<CPDF_Reference>("Pages", this, pagesDict->GetObjNum());
  }
};

class CPDF_TestDocumentAllowSetParser final : public CPDF_Document {
 public:
  CPDF_TestDocumentAllowSetParser()
      : CPDF_Document(pdfium::MakeUnique<CPDF_DocRenderData>(),
                      pdfium::MakeUnique<CPDF_DocPageData>()) {}

  using CPDF_Document::SetParser;
};

}  // namespace

class cpdf_document_test : public testing::Test {
 public:
  void SetUp() override { CPDF_PageModule::Create(); }
  void TearDown() override { CPDF_PageModule::Destroy(); }
};

TEST_F(cpdf_document_test, GetPages) {
  std::unique_ptr<CPDF_TestDocumentForPages> document =
      pdfium::MakeUnique<CPDF_TestDocumentForPages>();
  for (int i = 0; i < kNumTestPages; i++) {
    CPDF_Dictionary* page = document->GetPageDictionary(i);
    ASSERT_TRUE(page);
    ASSERT_TRUE(page->KeyExist("PageNumbering"));
    EXPECT_EQ(i, page->GetIntegerFor("PageNumbering"));
  }
  CPDF_Dictionary* page = document->GetPageDictionary(kNumTestPages);
  EXPECT_FALSE(page);
}

TEST_F(cpdf_document_test, GetPageWithoutObjNumTwice) {
  auto document = pdfium::MakeUnique<CPDF_TestDocumentWithPageWithoutPageNum>();
  CPDF_Dictionary* page = document->GetPageDictionary(2);
  ASSERT_TRUE(page);
  ASSERT_EQ(document->inlined_page(), page);

  CPDF_Dictionary* second_call_page = document->GetPageDictionary(2);
  EXPECT_TRUE(second_call_page);
  EXPECT_EQ(page, second_call_page);
}

TEST_F(cpdf_document_test, GetPagesReverseOrder) {
  std::unique_ptr<CPDF_TestDocumentForPages> document =
      pdfium::MakeUnique<CPDF_TestDocumentForPages>();
  for (int i = 6; i >= 0; i--) {
    CPDF_Dictionary* page = document->GetPageDictionary(i);
    ASSERT_TRUE(page);
    ASSERT_TRUE(page->KeyExist("PageNumbering"));
    EXPECT_EQ(i, page->GetIntegerFor("PageNumbering"));
  }
  CPDF_Dictionary* page = document->GetPageDictionary(kNumTestPages);
  EXPECT_FALSE(page);
}

TEST_F(cpdf_document_test, GetPagesInDisorder) {
  std::unique_ptr<CPDF_TestDocumentForPages> document =
      pdfium::MakeUnique<CPDF_TestDocumentForPages>();

  CPDF_Dictionary* page = document->GetPageDictionary(1);
  ASSERT_TRUE(page);
  ASSERT_TRUE(page->KeyExist("PageNumbering"));
  EXPECT_EQ(1, page->GetIntegerFor("PageNumbering"));

  page = document->GetPageDictionary(3);
  ASSERT_TRUE(page);
  ASSERT_TRUE(page->KeyExist("PageNumbering"));
  EXPECT_EQ(3, page->GetIntegerFor("PageNumbering"));

  page = document->GetPageDictionary(kNumTestPages);
  EXPECT_FALSE(page);

  page = document->GetPageDictionary(6);
  ASSERT_TRUE(page);
  ASSERT_TRUE(page->KeyExist("PageNumbering"));
  EXPECT_EQ(6, page->GetIntegerFor("PageNumbering"));
}

TEST_F(cpdf_document_test, IsValidPageObject) {
  CPDF_TestDocumentForPages document;

  auto dict_type_name_page = pdfium::MakeRetain<CPDF_Dictionary>();
  dict_type_name_page->SetNewFor<CPDF_Name>("Type", "Page");
  EXPECT_TRUE(CPDF_Document::IsValidPageObject(
      document.AddIndirectObject(dict_type_name_page)));

  auto dict_type_string_page = pdfium::MakeRetain<CPDF_Dictionary>();
  dict_type_string_page->SetNewFor<CPDF_String>("Type", "Page", false);
  EXPECT_FALSE(CPDF_Document::IsValidPageObject(
      document.AddIndirectObject(dict_type_string_page)));

  auto dict_type_name_font = pdfium::MakeRetain<CPDF_Dictionary>();
  dict_type_name_font->SetNewFor<CPDF_Name>("Type", "Font");
  EXPECT_FALSE(CPDF_Document::IsValidPageObject(
      document.AddIndirectObject(dict_type_name_font)));

  CPDF_Object* obj_no_type = document.NewIndirect<CPDF_Dictionary>();
  EXPECT_FALSE(CPDF_Document::IsValidPageObject(obj_no_type));
}

TEST_F(cpdf_document_test, UseCachedPageObjNumIfHaveNotPagesDict) {
  // ObjNum can be added in CPDF_DataAvail::IsPageAvail(), and PagesDict may not
  // exist in this case, e.g. when hint table is used to page check in
  // CPDF_DataAvail.
  constexpr int kPageCount = 100;
  constexpr int kTestPageNum = 33;

  auto linearization_dict = pdfium::MakeRetain<CPDF_Dictionary>();
  CPDF_TestDocumentAllowSetParser document;

  {
    CPDF_Object* first_page = document.AddIndirectObject(CreateNumberedPage(0));
    ASSERT(first_page);
    int first_page_obj_num = first_page->GetObjNum();
    ASSERT_NE(kTestPageNum, first_page_obj_num);

    linearization_dict->SetNewFor<CPDF_Boolean>("Linearized", true);
    linearization_dict->SetNewFor<CPDF_Number>("N", kPageCount);
    linearization_dict->SetNewFor<CPDF_Number>("O", first_page_obj_num);

    auto parser = pdfium::MakeUnique<CPDF_Parser>();
    parser->SetLinearizedHeaderForTesting(
        pdfium::MakeUnique<TestLinearized>(linearization_dict.Get()));
    document.SetParser(std::move(parser));
  }

  document.LoadPages();

  ASSERT_EQ(kPageCount, document.GetPageCount());
  CPDF_Object* page_stub = document.NewIndirect<CPDF_Dictionary>();
  const uint32_t obj_num = page_stub->GetObjNum();

  EXPECT_FALSE(document.IsPageLoaded(kTestPageNum));
  EXPECT_EQ(nullptr, document.GetPageDictionary(kTestPageNum));

  document.SetPageObjNum(kTestPageNum, obj_num);
  EXPECT_TRUE(document.IsPageLoaded(kTestPageNum));
  EXPECT_EQ(page_stub, document.GetPageDictionary(kTestPageNum));
}

TEST_F(cpdf_document_test, CountGreaterThanPageTree) {
  std::unique_ptr<CPDF_TestDocumentForPages> document =
      pdfium::MakeUnique<CPDF_TestDocumentForPages>();
  document->SetTreeSize(kNumTestPages + 3);
  for (int i = 0; i < kNumTestPages; i++)
    EXPECT_TRUE(document->GetPageDictionary(i));
  for (int i = kNumTestPages; i < kNumTestPages + 4; i++)
    EXPECT_FALSE(document->GetPageDictionary(i));
  EXPECT_TRUE(document->GetPageDictionary(kNumTestPages - 1));
}

TEST_F(cpdf_document_test, PagesWithoutKids) {
  // Set up a document with Pages dict without kids, and Count = 3
  auto pDoc = pdfium::MakeUnique<CPDF_TestDocPagesWithoutKids>();
  EXPECT_TRUE(pDoc->GetPageDictionary(0));
  // Test GetPage does not fetch pages out of range
  for (int i = 1; i < 5; i++)
    EXPECT_FALSE(pDoc->GetPageDictionary(i));

  EXPECT_TRUE(pDoc->GetPageDictionary(0));
}
