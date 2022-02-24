// Copyright 2017 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/cfx_widetextbuf.h"
#include "core/fxcrt/fx_string.h"
#include "testing/fuzzers/pdfium_fuzzer_util.h"
#include "testing/fuzzers/xfa_process_state.h"
#include "xfa/fxfa/formcalc/cxfa_fmparser.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  auto* state = static_cast<XFAProcessState*>(FPDF_GetFuzzerPerProcessState());
  WideString input = WideString::FromUTF8(ByteStringView(data, size));
  CXFA_FMLexer lexer(input.AsStringView());
  CXFA_FMParser parser(state->GetHeap(), &lexer);
  parser.Parse();
  state->ForceGCAndPump();
  return 0;
}
