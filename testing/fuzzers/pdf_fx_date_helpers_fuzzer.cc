// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/widestring.h"
#include "fxjs/fx_date_helpers.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size > 4 && size < 16384) {
    double ignore;
    size_t len1 = size / 2;
    size_t len2 = size - len1;
    WideString input1 = WideString::FromUTF16LE({data, len1});
    WideString input2 = WideString::FromUTF16LE({data + len1, len2});
    FX_ParseDateUsingFormat(input1, input2, &ignore);
  }
  return 0;
}
