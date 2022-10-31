// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// No-frills example of how to initialize and call into a PDFium environment
// including V8 support (but not XFA) from C++. Since the V8 API is in C++,
// C++ is required in this case (not just C).

#include <string.h>

#include "public/fpdf_edit.h"
#include "public/fpdf_formfill.h"
#include "public/fpdfview.h"
#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8-array-buffer.h"
#include "v8/include/v8-initialization.h"
#include "v8/include/v8-isolate.h"

int main(int argc, const char* argv[]) {
  // V8 must be initialized before the PDFium library if using V8.
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  v8::Platform* platform = v8::platform::NewDefaultPlatform().release();
  v8::V8::InitializePlatform(platform);
  v8::V8::Initialize();

  v8::Isolate::CreateParams params;
  params.array_buffer_allocator = static_cast<v8::ArrayBuffer::Allocator*>(
      FPDF_GetArrayBufferAllocatorSharedInstance());
  v8::Isolate* isolate = v8::Isolate::New(params);

  // The PDF library must be initialized before creating a document.
  FPDF_LIBRARY_CONFIG config;
  memset(&config, 0, sizeof(config));
  config.version = 3;
  config.m_pIsolate = isolate;
  config.m_pPlatform = platform;
  FPDF_InitLibraryWithConfig(&config);

  // The document must be created before creating a form-fill environment.
  // Typically use FPDF_LoadDocument() for pre-existing documents. Here, we
  // create a new blank document for simplicity.
  FPDF_DOCUMENT doc = FPDF_CreateNewDocument();

  IPDF_JSPLATFORM jsplatform;
  memset(&jsplatform, 0, sizeof(jsplatform));

  FPDF_FORMFILLINFO formfillinfo;
  memset(&formfillinfo, 0, sizeof(formfillinfo));
  formfillinfo.version = 1;
  formfillinfo.m_pJsPlatform = &jsplatform;

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

  isolate->Dispose();
  v8::V8::ShutdownPlatform();
  delete platform;

  return 0;
}
