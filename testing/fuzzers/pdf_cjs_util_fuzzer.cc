// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/widestring.h"
#include "fxjs/cjs_util.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size > 2) {
    WideString input = WideString::FromUTF16LE({data, size});
    CJS_Util::ParseDataType(&input);
  }
  if (size > 4) {
    size_t len1 = size / 2;
    size_t len2 = size - len1;
    WideString input1 = WideString::FromUTF16LE({data, len1});
    WideString input2 = WideString::FromUTF16LE({data + len1, len2});
    CJS_Util::StringPrintx(input1, input2);
  }
  return 0;
}
