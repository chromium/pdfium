// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/xfa_js_embedder_test.h"

#include <string>

#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/fpdfxfa/cpdfxfa_context.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "testing/gtest/include/gtest/gtest.h"

XFAJSEmbedderTest::XFAJSEmbedderTest() = default;

XFAJSEmbedderTest::~XFAJSEmbedderTest() = default;

void XFAJSEmbedderTest::SetUp() {
  JSEmbedderTest::SetUp();
}

void XFAJSEmbedderTest::TearDown() {
  value_.reset();
  script_context_ = nullptr;

  JSEmbedderTest::TearDown();
}

CXFA_Document* XFAJSEmbedderTest::GetXFADocument() const {
  auto* pDoc = CPDFDocumentFromFPDFDocument(document());
  if (!pDoc)
    return nullptr;

  auto* pContext = static_cast<CPDFXFA_Context*>(pDoc->GetExtension());
  if (!pContext)
    return nullptr;

  return pContext->GetXFADoc()->GetXFADoc();
}

bool XFAJSEmbedderTest::OpenDocumentWithOptions(
    const std::string& filename,
    const char* password,
    LinearizeOption linearize_option,
    JavaScriptOption javascript_option) {
  // JS required for XFA.
  ASSERT(javascript_option == JavaScriptOption::kEnableJavaScript);
  if (!EmbedderTest::OpenDocumentWithOptions(
          filename, password, linearize_option, javascript_option)) {
    return false;
  }
  script_context_ = GetXFADocument()->GetScriptContext();
  return true;
}

bool XFAJSEmbedderTest::Execute(ByteStringView input) {
  if (ExecuteHelper(input))
    return true;

  CFXJSE_Value msg(isolate());
  value_->GetObjectPropertyByIdx(1, &msg);
  fprintf(stderr, "FormCalc: %.*s\n", static_cast<int>(input.GetLength()),
          input.unterminated_c_str());
  // If the parsing of the input fails, then v8 will not run, so there will be
  // no value here to print.
  if (msg.IsString() && !msg.ToWideString().IsEmpty())
    fprintf(stderr, "JS ERROR: %ls\n", msg.ToWideString().c_str());
  return false;
}

bool XFAJSEmbedderTest::ExecuteSilenceFailure(ByteStringView input) {
  return ExecuteHelper(input);
}

bool XFAJSEmbedderTest::ExecuteHelper(ByteStringView input) {
  value_ = std::make_unique<CFXJSE_Value>(isolate());
  return script_context_->RunScript(CXFA_Script::Type::Formcalc,
                                    WideString::FromUTF8(input).AsStringView(),
                                    value_.get(), GetXFADocument()->GetRoot());
}
