// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include "public/fpdf_formfill.h"
#include "testing/fuzzers/pdfium_fuzzer_helper.h"
#include "testing/fuzzers/pdfium_xfa_lpm_fuzz_stub.h"

class PDFiumLpmFuzzStub : public PDFiumFuzzerHelper {
 public:
  PDFiumLpmFuzzStub() = default;
  ~PDFiumLpmFuzzStub() override = default;

  int GetFormCallbackVersion() const override { return 2; }
  // Allow fuzzer to fuzz XFA but don't require it to fuzz.
  bool OnFormFillEnvLoaded(FPDF_DOCUMENT doc) override {
    FPDF_LoadXFA(doc);
    return true;
  }
};

void PdfiumXFALPMFuzzStub(const char* pdf, size_t size) {
  PDFiumLpmFuzzStub fuzz_stub;
  fuzz_stub.RenderPdf(pdf, size);
}
