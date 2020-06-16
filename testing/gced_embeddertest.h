// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_GCED_EMBEDDER_TEST_H_
#define TESTING_GCED_EMBEDDER_TEST_H_

#include <memory>

#include "testing/embedder_test.h"

namespace v8 {
class Isolate;
}  // namespace v8

class GCedEmbedderTest : public EmbedderTest {
 public:
  void SetUp() override;
  void TearDown() override;
  void PumpPlatformMessageLoop();

 private:
  struct IsolateDeleter {
    void operator()(v8::Isolate* ptr);
  };

  std::unique_ptr<v8::Isolate, IsolateDeleter> isolate_;
};

#endif  // TESTING_GCED_EMBEDDER_TEST_H_
