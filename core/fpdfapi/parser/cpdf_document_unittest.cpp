// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_document.h"

#include <memory>

#include "core/fpdfapi/cpdf_modulemgr.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fxcrt/fx_memory.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

CPDF_Dictionary* CreatePageTreeNode(CPDF_Array* kids,
                                    CPDF_Document* pDoc,
                                    int count) {
  CPDF_Dictionary* pageNode = new CPDF_Dictionary();
  pageNode->SetStringFor("Type", "Pages");
  pageNode->SetReferenceFor("Kids", pDoc, pDoc->AddIndirectObject(kids));
  pageNode->SetIntegerFor("Count", count);
  uint32_t pageNodeRef = pDoc->AddIndirectObject(pageNode);
  for (size_t i = 0; i < kids->GetCount(); i++)
    kids->GetDictAt(i)->SetReferenceFor("Parent", pDoc, pageNodeRef);
  return pageNode;
}

CPDF_Dictionary* CreateNumberedPage(size_t number) {
  CPDF_Dictionary* page = new CPDF_Dictionary();
  page->SetStringFor("Type", "Page");
  page->SetIntegerFor("PageNumbering", number);
  return page;
}

}  // namespace

class CPDF_TestDocumentForPages : public CPDF_Document {
 public:
  CPDF_TestDocumentForPages() : CPDF_Document(nullptr) {
    CPDF_ModuleMgr* module_mgr = CPDF_ModuleMgr::Get();
    module_mgr->InitPageModule();
    // Set up test
    CPDF_Array* zeroToTwo = new CPDF_Array();
    zeroToTwo->AddReference(this, AddIndirectObject(CreateNumberedPage(0)));
    zeroToTwo->AddReference(this, AddIndirectObject(CreateNumberedPage(1)));
    zeroToTwo->AddReference(this, AddIndirectObject(CreateNumberedPage(2)));
    CPDF_Dictionary* branch1 = CreatePageTreeNode(zeroToTwo, this, 3);

    CPDF_Array* zeroToThree = new CPDF_Array();
    zeroToThree->AddReference(this, branch1->GetObjNum());
    zeroToThree->AddReference(this, AddIndirectObject(CreateNumberedPage(3)));
    CPDF_Dictionary* branch2 = CreatePageTreeNode(zeroToThree, this, 4);

    CPDF_Array* fourFive = new CPDF_Array();
    fourFive->AddReference(this, AddIndirectObject(CreateNumberedPage(4)));
    fourFive->AddReference(this, AddIndirectObject(CreateNumberedPage(5)));
    CPDF_Dictionary* branch3 = CreatePageTreeNode(fourFive, this, 2);

    CPDF_Array* justSix = new CPDF_Array();
    justSix->AddReference(this, AddIndirectObject(CreateNumberedPage(6)));
    CPDF_Dictionary* branch4 = CreatePageTreeNode(justSix, this, 1);

    CPDF_Array* allPages = new CPDF_Array();
    allPages->AddReference(this, branch2->GetObjNum());
    allPages->AddReference(this, branch3->GetObjNum());
    allPages->AddReference(this, branch4->GetObjNum());
    CPDF_Dictionary* pagesDict = CreatePageTreeNode(allPages, this, 7);

    m_pOwnedRootDict.reset(new CPDF_Dictionary());
    m_pOwnedRootDict->SetReferenceFor("Pages", this,
                                      AddIndirectObject(pagesDict));
    m_pRootDict = m_pOwnedRootDict.get();
    m_PageList.SetSize(7);
  }

 private:
  std::unique_ptr<CPDF_Dictionary, ReleaseDeleter<CPDF_Dictionary>>
      m_pOwnedRootDict;
};

TEST(cpdf_document, GetPages) {
  std::unique_ptr<CPDF_TestDocumentForPages> document =
      pdfium::MakeUnique<CPDF_TestDocumentForPages>();
  for (int i = 0; i < 7; i++) {
    CPDF_Dictionary* page = document->GetPage(i);
    ASSERT_TRUE(page);
    ASSERT_TRUE(page->GetObjectFor("PageNumbering"));
    EXPECT_EQ(i, page->GetIntegerFor("PageNumbering"));
  }
  CPDF_Dictionary* page = document->GetPage(7);
  EXPECT_FALSE(page);
}

TEST(cpdf_document, GetPagesReverseOrder) {
  std::unique_ptr<CPDF_TestDocumentForPages> document =
      pdfium::MakeUnique<CPDF_TestDocumentForPages>();
  for (int i = 6; i >= 0; i--) {
    CPDF_Dictionary* page = document->GetPage(i);
    ASSERT_TRUE(page);
    ASSERT_TRUE(page->GetObjectFor("PageNumbering"));
    EXPECT_EQ(i, page->GetIntegerFor("PageNumbering"));
  }
  CPDF_Dictionary* page = document->GetPage(7);
  EXPECT_FALSE(page);
}
