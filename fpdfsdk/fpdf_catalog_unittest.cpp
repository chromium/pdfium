// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_catalog.h"

#include <memory>

#include "core/fpdfapi/page/test_with_page_module.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/cpdf_test_document.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "public/cpp/fpdf_scopers.h"
#include "testing/gtest/include/gtest/gtest.h"

class PDFCatalogTest : public TestWithPageModule {
 public:
  void SetUp() override {
    TestWithPageModule::SetUp();
    auto pTestDoc = std::make_unique<CPDF_TestDocument>();
    m_pDoc.reset(FPDFDocumentFromCPDFDocument(pTestDoc.release()));
    m_pRootObj = pdfium::MakeRetain<CPDF_Dictionary>();
  }

  void TearDown() override {
    m_pDoc.reset();
    TestWithPageModule::TearDown();
  }

 protected:
  ScopedFPDFDocument m_pDoc;
  RetainPtr<CPDF_Dictionary> m_pRootObj;
};

TEST_F(PDFCatalogTest, IsTagged) {
  // Null doc
  EXPECT_FALSE(FPDFCatalog_IsTagged(nullptr));

  CPDF_TestDocument* pTestDoc = static_cast<CPDF_TestDocument*>(
      CPDFDocumentFromFPDFDocument(m_pDoc.get()));

  // No root
  pTestDoc->SetRoot(nullptr);
  EXPECT_FALSE(FPDFCatalog_IsTagged(m_pDoc.get()));

  // Empty root
  pTestDoc->SetRoot(m_pRootObj);
  EXPECT_FALSE(FPDFCatalog_IsTagged(m_pDoc.get()));

  // Root with other key
  m_pRootObj->SetNewFor<CPDF_String>("OTHER_KEY", "other value");
  EXPECT_FALSE(FPDFCatalog_IsTagged(m_pDoc.get()));

  // Root with empty MarkInfo
  auto markInfoDict = m_pRootObj->SetNewFor<CPDF_Dictionary>("MarkInfo");
  EXPECT_FALSE(FPDFCatalog_IsTagged(m_pDoc.get()));

  // MarkInfo present but Marked is 0
  markInfoDict->SetNewFor<CPDF_Number>("Marked", 0);
  EXPECT_FALSE(FPDFCatalog_IsTagged(m_pDoc.get()));

  // MarkInfo present and Marked is 1, PDF is considered tagged.
  markInfoDict->SetNewFor<CPDF_Number>("Marked", 1);
  EXPECT_TRUE(FPDFCatalog_IsTagged(m_pDoc.get()));
}
