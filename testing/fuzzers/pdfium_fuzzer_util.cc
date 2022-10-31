// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/fuzzers/pdfium_fuzzer_util.h"

namespace {
void* g_fuzzer_init_per_process_state = nullptr;
}  // namespace

int GetInteger(const uint8_t* data) {
  return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}

FPDF_EXPORT void FPDF_CALLCONV FPDF_SetFuzzerPerProcessState(void* state) {
  g_fuzzer_init_per_process_state = state;
}

FPDF_EXPORT void* FPDF_CALLCONV FPDF_GetFuzzerPerProcessState() {
  return g_fuzzer_init_per_process_state;
}
