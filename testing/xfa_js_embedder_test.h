// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_XFA_JS_EMBEDDER_TEST_H_
#define TESTING_XFA_JS_EMBEDDER_TEST_H_

#include <string>

#include "core/fxcrt/string_view_template.h"
#include "testing/js_embedder_test.h"
#include "v8/include/v8-local-handle.h"
#include "v8/include/v8-persistent-handle.h"
#include "v8/include/v8-value.h"

class CFXJSE_Engine;
class CXFA_Document;

class XFAJSEmbedderTest : public JSEmbedderTest {
 public:
  XFAJSEmbedderTest();
  ~XFAJSEmbedderTest() override;

  // EmbedderTest:
  void SetUp() override;
  void TearDown() override;
  bool OpenDocumentWithOptions(const std::string& filename,
                               const char* password,
                               LinearizeOption linearize_option,
                               JavaScriptOption javascript_option) override;

  CXFA_Document* GetXFADocument() const;
  CFXJSE_Engine* GetScriptContext() const { return script_context_; }
  v8::Local<v8::Value> GetValue() const;

  bool Execute(ByteStringView input);
  bool ExecuteSilenceFailure(ByteStringView input);

 private:
  bool ExecuteHelper(ByteStringView input);

  v8::Global<v8::Value> value_;
  CFXJSE_Engine* script_context_ = nullptr;
};

#endif  // TESTING_XFA_JS_EMBEDDER_TEST_H_
