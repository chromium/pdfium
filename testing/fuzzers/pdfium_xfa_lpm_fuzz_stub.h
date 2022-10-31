// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_FUZZERS_PDFIUM_XFA_LPM_FUZZ_STUB_H_
#define TESTING_FUZZERS_PDFIUM_XFA_LPM_FUZZ_STUB_H_

#include "public/fpdfview.h"

// LPM defines LLVMFuzzerTestOneInput, this function should be used by the LPM
// harness to pass the deserialized proto to PDFium.
FPDF_EXPORT void PdfiumXFALPMFuzzStub(const char* pdf, size_t size);

#endif  // TESTING_FUZZERS_PDFIUM_XFA_LPM_FUZZ_STUB_H_
