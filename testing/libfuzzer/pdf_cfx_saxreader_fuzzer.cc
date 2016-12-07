// Copyright 2016 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "xfa/fde/xml/cfx_saxreader.h"
#include "xfa/fgas/crt/fgas_stream.h"
#include "xfa/fxfa/parser/cxfa_widetextread.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  CFX_WideString input = CFX_WideString::FromUTF8(
      CFX_ByteStringC(data, static_cast<FX_STRSIZE>(size)));
  auto stream = pdfium::MakeRetain<CXFA_WideTextRead>(input);
  if (!stream)
    return 0;

  CFX_RetainPtr<IFX_SeekableReadStream> fileRead =
      stream->MakeSeekableReadStream();
  if (!fileRead)
    return 0;

  CFX_SAXReader reader;
  if (reader.StartParse(fileRead, 0, -1, CFX_SaxParseMode_NotSkipSpace) < 0)
    return 0;

  while (1) {
    int32_t ret = reader.ContinueParse(nullptr);
    if (ret < 0 || ret > 99)
      break;
  }

  return 0;
}
