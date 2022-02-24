// Copyright 2022 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FPDFAPI_PARSER_CPDF_TEST_DOCUMENT_H_
#define CORE_FPDFAPI_PARSER_CPDF_TEST_DOCUMENT_H_

#include "core/fpdfapi/parser/cpdf_document.h"

class CPDF_TestDocument : public CPDF_Document {
 public:
  CPDF_TestDocument();

  void SetRoot(CPDF_Dictionary* root);
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_TEST_DOCUMENT_H_
