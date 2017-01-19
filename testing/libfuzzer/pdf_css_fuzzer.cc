// Copyright 2016 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_string.h"
#include "xfa/fde/css/cfde_csssyntaxparser.h"
#include "xfa/fde/css/fde_css.h"
#include "xfa/fgas/crt/fgas_stream.h"
#include "xfa/fxfa/parser/cxfa_widetextread.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  CFX_WideString input = CFX_WideString::FromUTF8(
      CFX_ByteStringC(data, static_cast<FX_STRSIZE>(size)));

  CFDE_CSSSyntaxParser parser;
  parser.Init(input.c_str(), size);

  FDE_CSSSyntaxStatus status;
  do {
    status = parser.DoSyntaxParse();
  } while (status != FDE_CSSSyntaxStatus::Error &&
           status != FDE_CSSSyntaxStatus::EOS);
  return 0;
}
