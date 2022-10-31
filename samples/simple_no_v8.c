// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// No-frills example of how to initialize and call into a PDFium environment,
// from C. The PDFium API is compatible with C (the C++ internals are hidden
// beneath it).

#include <string.h>

#include "public/fpdf_edit.h"
#include "public/fpdf_formfill.h"
#include "public/fpdfview.h"

int main(int argc, const char* argv[]) {
  // The PDF library must be initialized before creating a document.
  FPDF_LIBRARY_CONFIG config;
  memset(&config, 0, sizeof(config));
  config.version = 3;
  FPDF_InitLibraryWithConfig(&config);

  // The document must be created before creating a form-fill environment.
  // Typically use FPDF_LoadDocument() for pre-existing documents. Here, we
  // create a new blank document for simplicity.
  FPDF_DOCUMENT doc = FPDF_CreateNewDocument();

  FPDF_FORMFILLINFO formfillinfo;
  memset(&formfillinfo, 0, sizeof(formfillinfo));
  formfillinfo.version = 1;

  FPDF_FORMHANDLE form_handle =
      FPDFDOC_InitFormFillEnvironment(doc, &formfillinfo);

  // Typically use FPDF_LoadPage() for pre-existing pages. Here, we
  // create a new blank page for simplicity.
  FPDF_PAGE page = FPDFPage_New(doc, 0, 640.0, 480.0);
  FORM_OnAfterLoadPage(page, form_handle);
  FORM_DoPageAAction(page, form_handle, FPDFPAGE_AACTION_OPEN);

  // Do actual work with the page here.

  FORM_DoPageAAction(page, form_handle, FPDFPAGE_AACTION_CLOSE);
  FORM_OnBeforeClosePage(page, form_handle);
  FPDF_ClosePage(page);

  FORM_DoDocumentAAction(form_handle, FPDFDOC_AACTION_WC);
  FPDFDOC_ExitFormFillEnvironment(form_handle);
  FPDF_CloseDocument(doc);
  FPDF_DestroyLibrary();

  return 0;
}
