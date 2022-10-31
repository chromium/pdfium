// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_JS_EMBEDDER_TEST_H_
#define TESTING_JS_EMBEDDER_TEST_H_

#include "testing/embedder_test.h"

namespace v8 {
class Isolate;
}  // namespace v8

class JSEmbedderTest : public EmbedderTest {
 public:
  JSEmbedderTest();
  ~JSEmbedderTest() override;

  v8::Isolate* isolate() const;
};

#endif  // TESTING_JS_EMBEDDER_TEST_H_
