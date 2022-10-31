// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include "public/fpdfview.h"

// This template is used in component builds to forward to the real fuzzers
// which are exported from the PDFium shared library.

// FUZZER_IMPL is a macro defined at build time that contains the name of the
// real fuzzer.
extern "C" FPDF_EXPORT int FUZZER_IMPL(const uint8_t* data, size_t size);

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  return FUZZER_IMPL(data, size);
}
