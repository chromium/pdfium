// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_catalog.h"

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "public/fpdf_edit.h"
#include "testing/embedder_test.h"

using FPDFCatalogTest = EmbedderTest;

TEST_F(FPDFCatalogTest, SetLanguageInvalidDocument) {
  // Document cannot be nullptr.
  EXPECT_FALSE(FPDFCatalog_SetLanguage(nullptr, "en-US"));

  ScopedFPDFDocument doc(FPDF_CreateNewDocument());
  CPDF_Document* cpdf_doc = CPDFDocumentFromFPDFDocument(doc.get());

  // Language cannot be null.
  ASSERT_TRUE(cpdf_doc->GetRoot());
  EXPECT_FALSE(FPDFCatalog_SetLanguage(doc.get(), nullptr));

  // Catalog cannot be nullptr.
  cpdf_doc->SetRootForTesting(nullptr);
  EXPECT_FALSE(FPDFCatalog_SetLanguage(doc.get(), "en-US"));
}

TEST_F(FPDFCatalogTest, SetLanguageNewDocument) {
  ScopedFPDFDocument doc(FPDF_CreateNewDocument());

  const CPDF_Dictionary* catalog =
      CPDFDocumentFromFPDFDocument(doc.get())->GetRoot();
  ASSERT_TRUE(catalog);

  // The new document shouldn't have any entry for /Lang.
  EXPECT_FALSE(catalog->GetStringFor("Lang"));

  // Add a new entry.
  EXPECT_TRUE(FPDFCatalog_SetLanguage(doc.get(), "en-US"));

  RetainPtr<const CPDF_String> result_language = catalog->GetStringFor("Lang");
  ASSERT_TRUE(result_language);
  EXPECT_EQ("en-US", result_language->GetString());
}

TEST_F(FPDFCatalogTest, SetLanguageExistingDocument) {
  ASSERT_TRUE(OpenDocument("tagged_table.pdf"));

  const CPDF_Dictionary* catalog =
      CPDFDocumentFromFPDFDocument(document())->GetRoot();
  ASSERT_TRUE(catalog);

  // The PDF already has an existing entry for /Lang.
  RetainPtr<const CPDF_String> result_language = catalog->GetStringFor("Lang");
  ASSERT_TRUE(result_language);
  EXPECT_EQ("en-US", result_language->GetString());

  // Replace the existing entry.
  EXPECT_TRUE(FPDFCatalog_SetLanguage(document(), "hu"));

  result_language = catalog->GetStringFor("Lang");
  ASSERT_TRUE(result_language);
  EXPECT_EQ("hu", result_language->GetString());
}
