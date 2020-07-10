// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_JS_EMBEDDER_TEST_H_
#define TESTING_JS_EMBEDDER_TEST_H_

#include <memory>

#include "fxjs/cfx_v8.h"
#include "testing/embedder_test.h"
#include "v8/include/v8.h"

class JSEmbedderTest : public EmbedderTest {
 public:
  JSEmbedderTest();
  ~JSEmbedderTest() override;

  // EmbedderTest:
  void SetUp() override;
  void TearDown() override;

  v8::Isolate* isolate() const { return m_pIsolate.get(); }

 private:
  std::unique_ptr<CFX_V8ArrayBufferAllocator> m_pArrayBufferAllocator;
  std::unique_ptr<v8::Isolate, CFX_V8IsolateDeleter> m_pIsolate;
};

#endif  // TESTING_JS_EMBEDDER_TEST_H_
