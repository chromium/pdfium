// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/xfa_js_embedder_test.h"

#include <memory>
#include <string>

#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/fpdfxfa/cpdfxfa_context.h"
#include "fxjs/fxv8.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cfxjse_isolatetracker.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/check_op.h"
#include "v8/include/v8-container.h"
#include "v8/include/v8-local-handle.h"
#include "v8/include/v8-value.h"

XFAJSEmbedderTest::XFAJSEmbedderTest() = default;

XFAJSEmbedderTest::~XFAJSEmbedderTest() = default;

void XFAJSEmbedderTest::SetUp() {
  JSEmbedderTest::SetUp();
}

void XFAJSEmbedderTest::TearDown() {
  value_.Reset();
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

  CXFA_FFDoc* pFFDoc = pContext->GetXFADoc();
  if (!pFFDoc)
    return nullptr;

  return pFFDoc->GetXFADoc();
}

v8::Local<v8::Value> XFAJSEmbedderTest::GetValue() const {
  return v8::Local<v8::Value>::New(isolate(), value_);
}

bool XFAJSEmbedderTest::OpenDocumentWithOptions(
    const std::string& filename,
    const char* password,
    LinearizeOption linearize_option,
    JavaScriptOption javascript_option) {
  // JS required for XFA.
  DCHECK_EQ(javascript_option, JavaScriptOption::kEnableJavaScript);
  if (!EmbedderTest::OpenDocumentWithOptions(
          filename, password, linearize_option, javascript_option)) {
    return false;
  }
  CXFA_Document* pDoc = GetXFADocument();
  if (!pDoc)
    return false;

  script_context_ = pDoc->GetScriptContext();
  return true;
}

bool XFAJSEmbedderTest::Execute(ByteStringView input) {
  CFXJSE_ScopeUtil_IsolateHandleContext scope(
      script_context_->GetJseContextForTest());
  if (ExecuteHelper(input))
    return true;

  fprintf(stderr, "FormCalc: %.*s\n", static_cast<int>(input.GetLength()),
          input.unterminated_c_str());

  v8::Local<v8::Value> result = GetValue();
  if (!fxv8::IsArray(result))
    return false;

  v8::Local<v8::Value> msg = fxv8::ReentrantGetArrayElementHelper(
      isolate(), result.As<v8::Array>(), 1);
  if (!fxv8::IsString(msg))
    return false;

  WideString str = fxv8::ReentrantToWideStringHelper(isolate(), msg);
  if (!str.IsEmpty())
    fprintf(stderr, "JS ERROR: %ls\n", str.c_str());
  return false;
}

bool XFAJSEmbedderTest::ExecuteSilenceFailure(ByteStringView input) {
  CFXJSE_ScopeUtil_IsolateHandleContext scope(
      script_context_->GetJseContextForTest());
  return ExecuteHelper(input);
}

bool XFAJSEmbedderTest::ExecuteHelper(ByteStringView input) {
  auto value = std::make_unique<CFXJSE_Value>();
  bool ret = script_context_->RunScript(
      CXFA_Script::Type::Formcalc, WideString::FromUTF8(input).AsStringView(),
      value.get(), GetXFADocument()->GetRoot());
  value_.Reset(isolate(), value->GetValue(isolate()));
  return ret;
}
