// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/js_embedder_test.h"

#include "fxjs/cfxjs_engine.h"
#include "third_party/base/ptr_util.h"

JSEmbedderTest::JSEmbedderTest()
    : m_pArrayBufferAllocator(
          pdfium::MakeUnique<CFX_V8ArrayBufferAllocator>()) {}

JSEmbedderTest::~JSEmbedderTest() {}

void JSEmbedderTest::SetUp() {
  v8::Isolate::CreateParams params;
  params.array_buffer_allocator = m_pArrayBufferAllocator.get();
  m_pIsolate.reset(v8::Isolate::New(params));

  EmbedderTest::SetExternalIsolate(isolate());
  EmbedderTest::SetUp();

  v8::Isolate::Scope isolate_scope(isolate());
  v8::HandleScope handle_scope(isolate());
  FXJS_PerIsolateData::SetUp(isolate());
  m_Engine = pdfium::MakeUnique<CFXJS_Engine>(isolate());
  m_Engine->InitializeEngine();
}

void JSEmbedderTest::TearDown() {
  m_Engine->ReleaseEngine();
  m_Engine.reset();
  EmbedderTest::TearDown();
  m_pIsolate.reset();
}

v8::Local<v8::Context> JSEmbedderTest::GetV8Context() {
  return m_Engine->GetV8Context();
}
