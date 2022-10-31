// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fuzzer/FuzzedDataProvider.h>

#include <cctype>
#include <string>

#include "public/fpdf_formfill.h"
#include "testing/fuzzers/pdf_fuzzer_templates.h"
#include "testing/fuzzers/pdfium_fuzzer_helper.h"

class PDFiumXFAFuzzer : public PDFiumFuzzerHelper {
 public:
  PDFiumXFAFuzzer() = default;
  ~PDFiumXFAFuzzer() override = default;

  int GetFormCallbackVersion() const override { return 2; }

  // Return false if XFA doesn't load as otherwise we're duplicating the work
  // done by the non-xfa fuzzer.
  bool OnFormFillEnvLoaded(FPDF_DOCUMENT doc) override {
    int form_type = FPDF_GetFormType(doc);
    if (form_type != FORMTYPE_XFA_FULL && form_type != FORMTYPE_XFA_FOREGROUND)
      return false;
    return FPDF_LoadXFA(doc);
  }
};

bool IsValidForFuzzing(const uint8_t* data, size_t size) {
  if (size > 2048) {
    return false;
  }

  const char* ptr = reinterpret_cast<const char*>(data);
  bool is_open = false;
  size_t tag_size = 0;
  for (size_t i = 0; i < size; i++) {
    if (!std::isspace(ptr[i]) && !std::isprint(ptr[i])) {
      return false;
    }

    // We do not want any script tags. The reason is this fuzzer
    // should avoid exploring v8 code. Avoiding anything with "script"
    // is an over-approximation, in that some inputs may contain "script"
    // and still be a valid fuzz-case. However, this over-approximation is
    // used to enforce strict constraints and avoid cases where whitespace
    // may play a role, or other tags, e.g. "Javascript" will end up triggering
    // large explorations of v8 code. The alternative we considered were
    // "<script"
    if (i + 6 < size && memcmp(ptr + i, "script", 6) == 0) {
      return false;
    }

    if (ptr[i] == '<') {
      if (is_open) {
        return false;
      }
      is_open = true;
      tag_size = 0;
    } else if (ptr[i] == '>') {
      if (!is_open || tag_size == 0) {
        return false;
      }
      is_open = false;
    } else if (is_open) {
      tag_size++;
    }
  }
  // we must close the last bracket.
  return !is_open;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  // Filter the string to reduce the state space exploration.
  if (!IsValidForFuzzing(data, size)) {
    return 0;
  }
  std::string xfa_string = "<xdp xmlns=\"http://ns.adobe.com/xdp/\">";
  xfa_string += std::string(reinterpret_cast<const char*>(data), size);
  xfa_string += "</xdp>";

  // Add 1 for newline before endstream.
  std::string xfa_stream_len = std::to_string(xfa_string.size() + 1);

  // Compose the fuzzer
  std::string xfa_final_str = std::string(kSimplePdfTemplate);
  xfa_final_str.replace(xfa_final_str.find("$1"), 2, xfa_stream_len);
  xfa_final_str.replace(xfa_final_str.find("$2"), 2, xfa_string);

#ifdef PDFIUM_FUZZER_DUMP
  for (size_t i = 0; i < xfa_final_str.size(); i++) {
    putc(xfa_final_str[i], stdout);
  }
#endif

  PDFiumXFAFuzzer fuzzer;
  fuzzer.RenderPdf(xfa_final_str.c_str(), xfa_final_str.size());
  return 0;
}
