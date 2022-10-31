// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/js_embedder_test.h"

#include "testing/v8_test_environment.h"

JSEmbedderTest::JSEmbedderTest() = default;

JSEmbedderTest::~JSEmbedderTest() = default;

v8::Isolate* JSEmbedderTest::isolate() const {
  return V8TestEnvironment::GetInstance()->isolate();
}
