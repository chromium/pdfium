// Copyright 2022 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_test_document.h"

#include <memory>

#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/render/cpdf_docrenderdata.h"

CPDF_TestDocument::CPDF_TestDocument()
    : CPDF_Document(std::make_unique<CPDF_DocRenderData>(),
                    std::make_unique<CPDF_DocPageData>()) {}

void CPDF_TestDocument::SetRoot(CPDF_Dictionary* root) {
  SetRootForTesting(root);
}
