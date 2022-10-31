// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/widestring.h"
#include "fxjs/fx_date_helpers.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  auto* short_data = reinterpret_cast<const unsigned short*>(data);
  size_t short_size = size / sizeof(unsigned short);
  if (short_size > 2 && short_size < 8192) {
    double ignore;
    size_t short_len1 = short_size / 2;
    size_t short_len2 = short_size - short_len1;
    WideString input1 = WideString::FromUTF16LE(short_data, short_len1);
    WideString input2 =
        WideString::FromUTF16LE(short_data + short_len1, short_len2);
    FX_ParseDateUsingFormat(input1, input2, &ignore);
  }
  return 0;
}
