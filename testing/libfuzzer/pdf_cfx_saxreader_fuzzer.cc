// Copyright 2016 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "core/fxcrt/cfx_memorystream.h"
#include "core/fxcrt/cfx_seekablestreamproxy.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/xml/cfx_saxreader.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  CFX_SAXReader reader;
  if (reader.StartParse(pdfium::MakeRetain<CFX_MemoryStream>(
                            const_cast<uint8_t*>(data), size, false),
                        0, -1, CFX_SaxParseMode_NotSkipSpace) < 0) {
    return 0;
  }

  while (1) {
    int32_t ret = reader.ContinueParse();
    if (ret < 0 || ret > 99)
      break;
  }

  return 0;
}
