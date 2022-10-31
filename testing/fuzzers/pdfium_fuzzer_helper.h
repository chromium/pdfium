// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_FUZZERS_PDFIUM_FUZZER_HELPER_H_
#define TESTING_FUZZERS_PDFIUM_FUZZER_HELPER_H_

#include <stdint.h>

#include "public/fpdfview.h"

class PDFiumFuzzerHelper {
 public:
  void RenderPdf(const char* data, size_t len);

  virtual int GetFormCallbackVersion() const = 0;
  virtual bool OnFormFillEnvLoaded(FPDF_DOCUMENT doc);
  virtual void OnRenderFinished(FPDF_DOCUMENT doc) {}
  virtual void FormActionHandler(FPDF_FORMHANDLE form,
                                 FPDF_DOCUMENT doc,
                                 FPDF_PAGE page) {}

 protected:
  PDFiumFuzzerHelper();
  virtual ~PDFiumFuzzerHelper();

 private:
  bool RenderPage(FPDF_DOCUMENT doc,
                  FPDF_FORMHANDLE form,
                  int page_index,
                  int render_flags,
                  int form_flags);
};

#endif  // TESTING_FUZZERS_PDFIUM_FUZZER_HELPER_H_
