// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/widestring.h"
#include "fxjs/fx_date_helpers.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size > 4 && size < 16384) {
    double ignore;
    size_t len1 = size / 2;

    // SAFETY: trusted arguments from fuzzer.
    auto span = UNSAFE_BUFFERS(pdfium::make_span(data, size));
    WideString input1 = WideString::FromUTF16LE(span.first(len1));
    WideString input2 = WideString::FromUTF16LE(span.subspan(len1));
    FX_ParseDateUsingFormat(input1, input2, &ignore);
  }
  return 0;
}
