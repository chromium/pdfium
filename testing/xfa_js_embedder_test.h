// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_XFA_JS_EMBEDDER_TEST_H_
#define TESTING_XFA_JS_EMBEDDER_TEST_H_

#include <memory>
#include <string>

#include "fxjs/cfxjse_value.h"
#include "fxjs/fxjs_v8.h"
#include "testing/embedder_test.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_object.h"

class CXFA_ScriptContext;

class XFAJSEmbedderTest : public EmbedderTest {
 public:
  XFAJSEmbedderTest();
  ~XFAJSEmbedderTest() override;

  void SetUp() override;
  void TearDown() override;

  bool OpenDocument(const std::string& filename,
                    const char* password = nullptr,
                    bool must_linearize = false) override;

  v8::Isolate* GetIsolate() const { return isolate_; }
  CXFA_Document* GetXFADocument();

  bool Execute(const CFX_ByteStringC& input);
  bool ExecuteSilenceFailure(const CFX_ByteStringC& input);

  CFXJSE_Value* GetValue() const { return value_.get(); }

 private:
  std::unique_ptr<FXJS_ArrayBufferAllocator> array_buffer_allocator_;
  std::unique_ptr<CFXJSE_Value> value_;
  v8::Isolate* isolate_;
  CXFA_ScriptContext* script_context_;

  bool ExecuteHelper(const CFX_ByteStringC& input);
};

#endif  // TESTING_XFA_JS_EMBEDDER_TEST_H_
