// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include "testing/fuzzers/pdfium_fuzzer_helper.h"
#include "testing/fuzzers/pdfium_lpm_fuzz_stub.h"

class PDFiumLpmFuzzStub : public PDFiumFuzzerHelper {
 public:
  PDFiumLpmFuzzStub() = default;
  ~PDFiumLpmFuzzStub() override = default;

  int GetFormCallbackVersion() const override { return 1; }
};

void FuzzPdf(const char* pdf, size_t size) {
  PDFiumLpmFuzzStub fuzz_stub;
  fuzz_stub.RenderPdf(pdf, size);
}
