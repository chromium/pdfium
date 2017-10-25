// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/fm2js/cxfa_fmtojavascriptdepth.h"

namespace {

// Arbitarily picked by looking at how deep a translation got before hitting
// the getting fuzzer memory limits. Should be larger then |kMaxParseDepth| in
// cxfa_fmparser.cpp.
const unsigned int kMaxDepth = 5000;

}  // namespace

unsigned long CXFA_FMToJavaScriptDepth::depth_ = 0;
unsigned long CXFA_FMToJavaScriptDepth::max_depth_ = kMaxDepth;

void CXFA_FMToJavaScriptDepth::Reset() {
  depth_ = 0;
}
