// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/js_embedder_test.h"

#include "fxjs/cfxjs_engine.h"

JSEmbedderTest::JSEmbedderTest()
    : m_pArrayBufferAllocator(std::make_unique<CFX_V8ArrayBufferAllocator>()) {}

JSEmbedderTest::~JSEmbedderTest() = default;

void JSEmbedderTest::SetUp() {
  v8::Isolate::CreateParams params;
  params.array_buffer_allocator = m_pArrayBufferAllocator.get();
  m_pIsolate.reset(v8::Isolate::New(params));

  EmbedderTest::SetExternalIsolate(m_pIsolate.get());
  EmbedderTest::SetUp();
}

void JSEmbedderTest::TearDown() {
  EmbedderTest::TearDown();
  EmbedderTest::SetExternalIsolate(nullptr);
  m_pIsolate.reset();
}
