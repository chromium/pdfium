// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/xfa_js_embedder_test.h"

#include <string>

#include "fpdfsdk/fpdfxfa/cpdfxfa_context.h"
#include "fpdfsdk/fsdk_define.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/parser/cxfa_scriptcontext.h"

XFAJSEmbedderTest::XFAJSEmbedderTest()
    : array_buffer_allocator_(pdfium::MakeUnique<FXJS_ArrayBufferAllocator>()) {
}

XFAJSEmbedderTest::~XFAJSEmbedderTest() {}

void XFAJSEmbedderTest::SetUp() {
  v8::Isolate::CreateParams params;
  params.array_buffer_allocator = array_buffer_allocator_.get();
  isolate_ = v8::Isolate::New(params);
  ASSERT_TRUE(isolate_ != nullptr);

  EmbedderTest::SetExternalIsolate(isolate_);
  EmbedderTest::SetUp();
}

void XFAJSEmbedderTest::TearDown() {
  value_ = nullptr;
  script_context_ = nullptr;

  EmbedderTest::TearDown();

  isolate_->Dispose();
  isolate_ = nullptr;
}

CXFA_Document* XFAJSEmbedderTest::GetXFADocument() {
  return UnderlyingFromFPDFDocument(document())->GetXFADoc()->GetXFADoc();
}

bool XFAJSEmbedderTest::OpenDocument(const std::string& filename,
                                     const char* password,
                                     bool must_linearize) {
  if (!EmbedderTest::OpenDocument(filename, password, must_linearize))
    return false;

  script_context_ = GetXFADocument()->GetScriptContext();
  return true;
}

bool XFAJSEmbedderTest::Execute(const CFX_ByteStringC& input) {
  if (ExecuteHelper(input)) {
    return true;
  }

  CFXJSE_Value msg(GetIsolate());
  value_->GetObjectPropertyByIdx(1, &msg);
  fprintf(stderr, "JS: %.*s\n", input.GetLength(), input.unterminated_c_str());
  // If the parsing of the input fails, then v8 will not run, so there will be
  // no value here to print.
  if (msg.IsString() && !msg.ToWideString().IsEmpty())
    fprintf(stderr, "JS ERROR: %ls\n", msg.ToWideString().c_str());
  return false;
}

bool XFAJSEmbedderTest::ExecuteSilenceFailure(const CFX_ByteStringC& input) {
  return ExecuteHelper(input);
}

bool XFAJSEmbedderTest::ExecuteHelper(const CFX_ByteStringC& input) {
  value_ = pdfium::MakeUnique<CFXJSE_Value>(GetIsolate());
  return script_context_->RunScript(XFA_SCRIPTLANGTYPE_Formcalc,
                                    CFX_WideString::FromUTF8(input).AsStringC(),
                                    value_.get(), GetXFADocument()->GetRoot());
}
