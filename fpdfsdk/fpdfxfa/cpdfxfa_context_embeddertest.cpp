// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fpdfsdk/fpdfxfa/cpdfxfa_context.h"

#include "fpdfsdk/cpdfsdk_helpers.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/xfa_js_embedder_test.h"

class CPDFXFAContextEmbedderTest : public XFAJSEmbedderTest {};

// Should not crash.
TEST_F(CPDFXFAContextEmbedderTest, HasHeap) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  CPDF_Document* pDocument = CPDFDocumentFromFPDFDocument(document());
  auto* pContext = static_cast<CPDFXFA_Context*>(pDocument->GetExtension());
  EXPECT_TRUE(pContext->GetGCHeap());
}
