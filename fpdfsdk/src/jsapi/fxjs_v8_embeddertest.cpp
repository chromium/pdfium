// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "../../../core/include/fpdfapi/fpdf_parser.h"
#include "../../../testing/embedder_test.h"
#include "../../include/fsdk_mgr.h"
#include "../../include/javascript/JS_Runtime.h"
#include "../../include/jsapi/fxjs_v8.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

const wchar_t kScript[] = L"fred = 7";

}  // namespace

class FXJSV8Embeddertest : public EmbedderTest {
 public:
  FXJSV8Embeddertest() : m_pIsolate(nullptr) {}
  ~FXJSV8Embeddertest() override {}

  void SetUp() override {
    EmbedderTest::SetUp();
    m_pAllocator.reset(new FXJS_ArrayBufferAllocator());

    v8::Isolate::CreateParams params;
    params.array_buffer_allocator = m_pAllocator.get();
    m_pIsolate = v8::Isolate::New(params);

    v8::Isolate::Scope isolate_scope(m_pIsolate);
    v8::HandleScope handle_scope(m_pIsolate);
    FXJS_Initialize(0);
    FXJS_PerIsolateData::SetUp(m_pIsolate);
    FXJS_InitializeRuntime(m_pIsolate, nullptr, nullptr, m_pPersistentContext);
  }

  void TearDown() override {
    FXJS_ReleaseRuntime(m_pIsolate, m_pPersistentContext);
    FXJS_Release();
    EmbedderTest::TearDown();
  }

  v8::Isolate* isolate() const { return m_pIsolate; }
  v8::Local<v8::Context> GetV8Context() {
    return v8::Local<v8::Context>::New(m_pIsolate, m_pPersistentContext);
  }

 private:
  v8::Isolate* m_pIsolate;
  v8::Global<v8::Context> m_pPersistentContext;
  nonstd::unique_ptr<v8::ArrayBuffer::Allocator> m_pAllocator;
};

TEST_F(FXJSV8Embeddertest, Getters) {
  v8::Isolate::Scope isolate_scope(isolate());
  v8::HandleScope handle_scope(isolate());
  v8::Context::Scope context_scope(GetV8Context());

  FXJSErr error;
  CFX_WideString wsInfo;
  CFX_WideString wsScript(kScript);
  int sts = FXJS_Execute(isolate(), nullptr, kScript, wcslen(kScript), &error);
  EXPECT_EQ(0, sts);

  v8::Local<v8::Object> This = FXJS_GetThisObj(isolate());
  v8::Local<v8::Value> fred = FXJS_GetObjectElement(isolate(), This, L"fred");
  EXPECT_TRUE(fred->IsNumber());
}
