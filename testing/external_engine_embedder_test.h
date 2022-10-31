// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_EXTERNAL_ENGINE_EMBEDDER_TEST_H_
#define TESTING_EXTERNAL_ENGINE_EMBEDDER_TEST_H_

#include <memory>

#include "testing/js_embedder_test.h"
#include "v8/include/v8-context.h"
#include "v8/include/v8-local-handle.h"

class CFXJS_Engine;

// Test class that allows creating a FXJS javascript engine without
// first having to load a document and instantiate a form filler
// against it. Generally, most tests will want to do the latter.
class ExternalEngineEmbedderTest : public JSEmbedderTest {
 public:
  ExternalEngineEmbedderTest();
  ~ExternalEngineEmbedderTest() override;

  // EmbedderTest:
  void SetUp() override;
  void TearDown() override;

  CFXJS_Engine* engine() const { return m_Engine.get(); }
  v8::Local<v8::Context> GetV8Context();

 private:
  std::unique_ptr<CFXJS_Engine> m_Engine;
};

#endif  // TESTING_EXTERNAL_ENGINE_EMBEDDER_TEST_H_
