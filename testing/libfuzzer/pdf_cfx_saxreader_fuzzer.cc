// Copyright 2016 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/xml/cfx_saxreader.h"
#include "xfa/fgas/crt/ifgas_stream.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  CFX_SAXReader reader;
  if (reader.StartParse(
          IFX_MemoryStream::Create(const_cast<uint8_t*>(data), size), 0, -1,
          CFX_SaxParseMode_NotSkipSpace) < 0) {
    return 0;
  }

  while (1) {
    int32_t ret = reader.ContinueParse(nullptr);
    if (ret < 0 || ret > 99)
      break;
  }

  return 0;
}
