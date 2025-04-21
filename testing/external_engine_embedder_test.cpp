// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/external_engine_embedder_test.h"

#include <memory>

#include "fxjs/cfxjs_engine.h"
#include "testing/v8_test_environment.h"
#include "v8/include/v8-context.h"
#include "v8/include/v8-isolate.h"
#include "v8/include/v8-local-handle.h"

ExternalEngineEmbedderTest::ExternalEngineEmbedderTest() = default;

ExternalEngineEmbedderTest::~ExternalEngineEmbedderTest() = default;

void ExternalEngineEmbedderTest::SetUp() {
  EmbedderTest::SetUp();

  v8::Isolate::Scope isolate_scope(isolate());
  v8::HandleScope handle_scope(isolate());
  CFXJS_PerIsolateData::SetUp(isolate());
  engine_ = std::make_unique<CFXJS_Engine>(isolate());
  engine_->InitializeEngine();
}

void ExternalEngineEmbedderTest::TearDown() {
  engine_->ReleaseEngine();
  engine_.reset();
  JSEmbedderTest::TearDown();
}

v8::Local<v8::Context> ExternalEngineEmbedderTest::GetV8Context() {
  return engine_->GetV8Context();
}
