// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_FXV8_UNITTEST_H_
#define TESTING_FXV8_UNITTEST_H_

#include <memory>

#include "testing/gtest/include/gtest/gtest.h"

class CFX_V8;
class CFX_V8ArrayBufferAllocator;

namespace v8 {
class Isolate;
}  // namespace v8

class FXV8UnitTest : public ::testing::Test {
 public:
  struct V8IsolateDeleter {
    void operator()(v8::Isolate* ptr) const;
  };

  FXV8UnitTest();
  ~FXV8UnitTest() override;

  void SetUp() override;

  v8::Isolate* isolate() const { return isolate_.get(); }

 protected:
  std::unique_ptr<CFX_V8ArrayBufferAllocator> array_buffer_allocator_;
  std::unique_ptr<v8::Isolate, V8IsolateDeleter> isolate_;
};

#endif  // TESTING_FXV8_UNITTEST_H_
