// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_XFA_JS_EMBEDDER_TEST_H_
#define TESTING_XFA_JS_EMBEDDER_TEST_H_

#include <memory>
#include <string>

#include "core/fxcrt/string_view_template.h"
#include "testing/js_embedder_test.h"

class CFXJSE_Engine;
class CFXJSE_Value;
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
  CFXJSE_Value* GetValue() const { return value_.get(); }

  bool Execute(ByteStringView input);
  bool ExecuteSilenceFailure(ByteStringView input);

 private:
  bool ExecuteHelper(ByteStringView input);

  std::unique_ptr<CFXJSE_Value> value_;
  CFXJSE_Engine* script_context_ = nullptr;
};

#endif  // TESTING_XFA_JS_EMBEDDER_TEST_H_
