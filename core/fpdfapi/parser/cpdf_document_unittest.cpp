// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_document.h"

#include <memory>
#include <utility>

#include "core/fpdfapi/page/test_with_page_module.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_boolean.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_linearized_header.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/cpdf_test_document.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/check.h"

namespace {

const int kNumTestPages = 7;

RetainPtr<CPDF_Dictionary> CreatePageTreeNode(RetainPtr<CPDF_Array> kids,
                                              CPDF_Document* pDoc,
                                              int count) {
  uint32_t new_objnum = pDoc->AddIndirectObject(kids);
  auto pageNode = pDoc->NewIndirect<CPDF_Dictionary>();
  pageNode->SetNewFor<CPDF_Name>("Type", "Pages");
  pageNode->SetNewFor<CPDF_Reference>("Kids", pDoc, new_objnum);
  pageNode->SetNewFor<CPDF_Number>("Count", count);
  for (size_t i = 0; i < kids->size(); i++) {
    kids->GetMutableDictAt(i)->SetNewFor<CPDF_Reference>("Parent", pDoc,
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

class CPDF_TestDocumentForPages final : public CPDF_TestDocument {
 public:
  CPDF_TestDocumentForPages() {
    // Set up test
    auto zeroToTwo = pdfium::MakeRetain<CPDF_Array>();
    zeroToTwo->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(0)));
    zeroToTwo->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(1)));
    zeroToTwo->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(2)));
    RetainPtr<CPDF_Dictionary> branch1 =
        CreatePageTreeNode(std::move(zeroToTwo), this, 3);

    auto zeroToThree = pdfium::MakeRetain<CPDF_Array>();
    zeroToThree->AppendNew<CPDF_Reference>(this, branch1->GetObjNum());
    zeroToThree->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(3)));
    RetainPtr<CPDF_Dictionary> branch2 =
        CreatePageTreeNode(std::move(zeroToThree), this, 4);

    auto fourFive = pdfium::MakeRetain<CPDF_Array>();
    fourFive->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(4)));
    fourFive->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(5)));
    RetainPtr<CPDF_Dictionary> branch3 =
        CreatePageTreeNode(std::move(fourFive), this, 2);

    auto justSix = pdfium::MakeRetain<CPDF_Array>();
    justSix->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(6)));
    RetainPtr<CPDF_Dictionary> branch4 =
        CreatePageTreeNode(std::move(justSix), this, 1);

    auto allPages = pdfium::MakeRetain<CPDF_Array>();
    allPages->AppendNew<CPDF_Reference>(this, branch2->GetObjNum());
    allPages->AppendNew<CPDF_Reference>(this, branch3->GetObjNum());
    allPages->AppendNew<CPDF_Reference>(this, branch4->GetObjNum());
    RetainPtr<CPDF_Dictionary> pagesDict =
        CreatePageTreeNode(std::move(allPages), this, kNumTestPages);

    SetRootForTesting(NewIndirect<CPDF_Dictionary>());
    GetMutableRoot()->SetNewFor<CPDF_Reference>("Pages", this,
                                                pagesDict->GetObjNum());
    ResizePageListForTesting(kNumTestPages);
  }

  void SetTreeSize(int size) {
    GetMutableRoot()->SetNewFor<CPDF_Number>("Count", size);
    ResizePageListForTesting(size);
  }
};

class CPDF_TestDocumentWithPageWithoutPageNum final : public CPDF_TestDocument {
 public:
  CPDF_TestDocumentWithPageWithoutPageNum() {
    // Set up test
    auto allPages = pdfium::MakeRetain<CPDF_Array>();
    allPages->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(0)));
    allPages->AppendNew<CPDF_Reference>(
        this, AddIndirectObject(CreateNumberedPage(1)));
    // Page without pageNum.
    inlined_page_ = CreateNumberedPage(2);
    allPages->Append(inlined_page_);
    RetainPtr<CPDF_Dictionary> pagesDict =
        CreatePageTreeNode(std::move(allPages), this, 3);
    SetRootForTesting(NewIndirect<CPDF_Dictionary>());
    GetMutableRoot()->SetNewFor<CPDF_Reference>("Pages", this,
                                                pagesDict->GetObjNum());
    ResizePageListForTesting(3);
  }

  const CPDF_Object* inlined_page() const { return inlined_page_.Get(); }

 private:
  RetainPtr<CPDF_Object> inlined_page_;
};

class TestLinearized final : public CPDF_LinearizedHeader {
 public:
  explicit TestLinearized(CPDF_Dictionary* dict)
      : CPDF_LinearizedHeader(dict, 0) {}
};

class CPDF_TestDocPagesWithoutKids final : public CPDF_TestDocument {
 public:
  CPDF_TestDocPagesWithoutKids() {
    auto pagesDict = NewIndirect<CPDF_Dictionary>();
    pagesDict->SetNewFor<CPDF_Name>("Type", "Pages");
    pagesDict->SetNewFor<CPDF_Number>("Count", 3);
    ResizePageListForTesting(10);
    SetRootForTesting(NewIndirect<CPDF_Dictionary>());
    GetMutableRoot()->SetNewFor<CPDF_Reference>("Pages", this,
                                                pagesDict->GetObjNum());
  }
};

class CPDF_TestDocumentAllowSetParser final : public CPDF_TestDocument {
 public:
  CPDF_TestDocumentAllowSetParser() = default;

  using CPDF_Document::SetParser;
};

}  // namespace

using DocumentTest = TestWithPageModule;

TEST_F(DocumentTest, GetPages) {
  std::unique_ptr<CPDF_TestDocumentForPages> document =
      std::make_unique<CPDF_TestDocumentForPages>();
  for (int i = 0; i < kNumTestPages; i++) {
    RetainPtr<const CPDF_Dictionary> page = document->GetPageDictionary(i);
    ASSERT_TRUE(page);
    ASSERT_TRUE(page->KeyExist("PageNumbering"));
    EXPECT_EQ(i, page->GetIntegerFor("PageNumbering"));
  }
  RetainPtr<const CPDF_Dictionary> page =
      document->GetPageDictionary(kNumTestPages);
  EXPECT_FALSE(page);
}

TEST_F(DocumentTest, GetPageWithoutObjNumTwice) {
  auto document = std::make_unique<CPDF_TestDocumentWithPageWithoutPageNum>();
  RetainPtr<const CPDF_Dictionary> page = document->GetPageDictionary(2);
  ASSERT_TRUE(page);
  ASSERT_EQ(document->inlined_page(), page);

  RetainPtr<const CPDF_Dictionary> second_call_page =
      document->GetPageDictionary(2);
  EXPECT_TRUE(second_call_page);
  EXPECT_EQ(page, second_call_page);
}

TEST_F(DocumentTest, GetPagesReverseOrder) {
  std::unique_ptr<CPDF_TestDocumentForPages> document =
      std::make_unique<CPDF_TestDocumentForPages>();
  for (int i = 6; i >= 0; i--) {
    RetainPtr<const CPDF_Dictionary> page = document->GetPageDictionary(i);
    ASSERT_TRUE(page);
    ASSERT_TRUE(page->KeyExist("PageNumbering"));
    EXPECT_EQ(i, page->GetIntegerFor("PageNumbering"));
  }
  RetainPtr<const CPDF_Dictionary> page =
      document->GetPageDictionary(kNumTestPages);
  EXPECT_FALSE(page);
}

TEST_F(DocumentTest, GetPagesInDisorder) {
  std::unique_ptr<CPDF_TestDocumentForPages> document =
      std::make_unique<CPDF_TestDocumentForPages>();

  RetainPtr<const CPDF_Dictionary> page = document->GetPageDictionary(1);
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

TEST_F(DocumentTest, IsValidPageObject) {
  CPDF_TestDocumentForPages document;

  auto dict_type_name_page = pdfium::MakeRetain<CPDF_Dictionary>();
  dict_type_name_page->SetNewFor<CPDF_Name>("Type", "Page");
  document.AddIndirectObject(dict_type_name_page);
  EXPECT_TRUE(CPDF_Document::IsValidPageObject(dict_type_name_page.Get()));

  auto dict_type_string_page = pdfium::MakeRetain<CPDF_Dictionary>();
  dict_type_string_page->SetNewFor<CPDF_String>("Type", "Page", false);
  document.AddIndirectObject(dict_type_string_page);
  EXPECT_FALSE(CPDF_Document::IsValidPageObject(dict_type_string_page.Get()));

  auto dict_type_name_font = pdfium::MakeRetain<CPDF_Dictionary>();
  dict_type_name_font->SetNewFor<CPDF_Name>("Type", "Font");
  document.AddIndirectObject(dict_type_name_font);
  EXPECT_FALSE(CPDF_Document::IsValidPageObject(dict_type_name_font.Get()));

  auto obj_no_type = document.NewIndirect<CPDF_Dictionary>();
  EXPECT_FALSE(CPDF_Document::IsValidPageObject(obj_no_type.Get()));
}

TEST_F(DocumentTest, UseCachedPageObjNumIfHaveNotPagesDict) {
  // ObjNum can be added in CPDF_DataAvail::IsPageAvail(), and PagesDict may not
  // exist in this case, e.g. when hint table is used to page check in
  // CPDF_DataAvail.
  constexpr int kPageCount = 100;
  constexpr int kTestPageNum = 33;

  auto linearization_dict = pdfium::MakeRetain<CPDF_Dictionary>();
  CPDF_TestDocumentAllowSetParser document;

  {
    auto first_page = CreateNumberedPage(0);
    ASSERT_TRUE(first_page);

    int first_page_obj_num = document.AddIndirectObject(first_page);
    ASSERT_NE(kTestPageNum, first_page_obj_num);

    linearization_dict->SetNewFor<CPDF_Boolean>("Linearized", true);
    linearization_dict->SetNewFor<CPDF_Number>("N", kPageCount);
    linearization_dict->SetNewFor<CPDF_Number>("O", first_page_obj_num);

    auto parser = std::make_unique<CPDF_Parser>();
    parser->SetLinearizedHeaderForTesting(
        std::make_unique<TestLinearized>(linearization_dict.Get()));
    document.SetParser(std::move(parser));
  }

  document.LoadPages();

  ASSERT_EQ(kPageCount, document.GetPageCount());
  auto page_stub = document.NewIndirect<CPDF_Dictionary>();
  const uint32_t obj_num = page_stub->GetObjNum();

  EXPECT_FALSE(document.IsPageLoaded(kTestPageNum));
  EXPECT_FALSE(document.GetPageDictionary(kTestPageNum));

  document.SetPageObjNum(kTestPageNum, obj_num);
  EXPECT_TRUE(document.IsPageLoaded(kTestPageNum));
  EXPECT_EQ(page_stub, document.GetPageDictionary(kTestPageNum));
}

TEST_F(DocumentTest, CountGreaterThanPageTree) {
  std::unique_ptr<CPDF_TestDocumentForPages> document =
      std::make_unique<CPDF_TestDocumentForPages>();
  document->SetTreeSize(kNumTestPages + 3);
  for (int i = 0; i < kNumTestPages; i++)
    EXPECT_TRUE(document->GetPageDictionary(i));
  for (int i = kNumTestPages; i < kNumTestPages + 4; i++)
    EXPECT_FALSE(document->GetPageDictionary(i));
  EXPECT_TRUE(document->GetPageDictionary(kNumTestPages - 1));
}

TEST_F(DocumentTest, PagesWithoutKids) {
  // Set up a document with Pages dict without kids, and Count = 3
  auto pDoc = std::make_unique<CPDF_TestDocPagesWithoutKids>();
  EXPECT_TRUE(pDoc->GetPageDictionary(0));
  // Test GetPage does not fetch pages out of range
  for (int i = 1; i < 5; i++)
    EXPECT_FALSE(pDoc->GetPageDictionary(i));

  EXPECT_TRUE(pDoc->GetPageDictionary(0));
}
