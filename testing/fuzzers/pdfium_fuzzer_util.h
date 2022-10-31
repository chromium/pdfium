// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_FUZZERS_PDFIUM_FUZZER_UTIL_H_
#define TESTING_FUZZERS_PDFIUM_FUZZER_UTIL_H_

#include <stdint.h>

#include "public/fpdfview.h"

// Returns an integer from the first 4 bytes of |data|.
int GetInteger(const uint8_t* data);

// Plumb access to any context created by fuzzer initialization into
// the LLVMFuzzerTestOneInput() function, as that function does not
// allow for additional parameters, nor can it reach back up to the
// top-level fuzzer shim during a component build (see the comment
// in BUILD.gn about splitting fuzzers into _impl and _src targets).
extern "C" {
FPDF_EXPORT void FPDF_CALLCONV FPDF_SetFuzzerPerProcessState(void* state);
FPDF_EXPORT void* FPDF_CALLCONV FPDF_GetFuzzerPerProcessState();
}  // extern "C"

#endif  // TESTING_FUZZERS_PDFIUM_FUZZER_UTIL_H_
