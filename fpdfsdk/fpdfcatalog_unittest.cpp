// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_catalog.h"

#include "core/fpdfapi/cpdf_modulemgr.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/cpdfxfa_context.h"
#endif  // PDF_ENABLE_XFA

class CPDF_TestDocument : public CPDF_Document {
 public:
  CPDF_TestDocument() : CPDF_Document(nullptr) {}

  void SetRoot(CPDF_Dictionary* root) {
    m_pRootDict = root;
    GetRoot();
  }
};

#ifdef PDF_ENABLE_XFA
class CPDF_TestXFAContext : public CPDFXFA_Context {
 public:
  CPDF_TestXFAContext()
      : CPDFXFA_Context(pdfium::MakeUnique<CPDF_TestDocument>()) {}

  void SetRoot(CPDF_Dictionary* root) {
    reinterpret_cast<CPDF_TestDocument*>(GetPDFDoc())->SetRoot(root);
  }

  CPDF_IndirectObjectHolder* GetHolder() { return GetPDFDoc(); }
};
using CPDF_TestPdfDocument = CPDF_TestXFAContext;
#else   // PDF_ENABLE_XFA
using CPDF_TestPdfDocument = CPDF_TestDocument;
#endif  // PDF_ENABLE_XFA

class PDFCatalogTest : public testing::Test {
 public:
  void SetUp() override {
    CPDF_ModuleMgr::Get()->Init();

    m_pDoc = pdfium::MakeUnique<CPDF_TestPdfDocument>();

    // Setup the root directory.
    m_pRootObj = pdfium::MakeUnique<CPDF_Dictionary>();
  }

  void TearDown() override {
    m_pDoc.reset();
    CPDF_ModuleMgr::Destroy();
  }

 protected:
  std::unique_ptr<CPDF_TestPdfDocument> m_pDoc;
  std::unique_ptr<CPDF_Dictionary> m_pRootObj;
};

TEST_F(PDFCatalogTest, IsTagged) {
  // Null doc
  EXPECT_FALSE(FPDFCatalog_IsTagged(nullptr));

  // No root
  m_pDoc->SetRoot(nullptr);
  EXPECT_FALSE(FPDFCatalog_IsTagged(m_pDoc.get()));

  // Empty root
  m_pDoc->SetRoot(m_pRootObj.get());
  EXPECT_FALSE(FPDFCatalog_IsTagged(m_pDoc.get()));

  // Root with other key
  m_pRootObj->SetNewFor<CPDF_String>("OTHER_KEY", "other value", false);
  EXPECT_FALSE(FPDFCatalog_IsTagged(m_pDoc.get()));

  // Root with empty MarkInfo
  CPDF_Dictionary* markInfoDict =
      m_pRootObj->SetNewFor<CPDF_Dictionary>("MarkInfo");
  EXPECT_FALSE(FPDFCatalog_IsTagged(m_pDoc.get()));

  // MarkInfo present but Marked is 0
  markInfoDict->SetNewFor<CPDF_Number>("Marked", 0);
  EXPECT_FALSE(FPDFCatalog_IsTagged(m_pDoc.get()));

  // MarkInfo present and Marked is 1, PDF is considered tagged.
  markInfoDict->SetNewFor<CPDF_Number>("Marked", 1);
  EXPECT_TRUE(FPDFCatalog_IsTagged(m_pDoc.get()));
}
