// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_V8_TEST_ENVIRONMENT_H_
#define TESTING_V8_TEST_ENVIRONMENT_H_

#ifndef PDF_ENABLE_V8
#error "V8 must be enabled"
#endif  // PDF_ENABLE_V8

#include <memory>

#include "fxjs/cfx_v8.h"
#include "fxjs/cfx_v8_array_buffer_allocator.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace v8 {
class Isolate;
class Platform;
class StartupData;
}  // namespace v8

class TestLoader;

class V8TestEnvironment : public testing::Environment {
 public:
  explicit V8TestEnvironment(const char* exe_path);
  ~V8TestEnvironment() override;

  // Note: GetInstance() does not create one if it does not exist,
  // so the main program must explicitly add this enviroment.
  static V8TestEnvironment* GetInstance();
  static void PumpPlatformMessageLoop(v8::Isolate* pIsolate);

  // testing::Environment:
  void SetUp() override;
  void TearDown() override;

  v8::Platform* platform() const { return platform_.get(); }
  v8::Isolate* isolate() const { return isolate_.get(); }

 private:
  const char* const exe_path_;
  std::unique_ptr<v8::StartupData> startup_data_;
  std::unique_ptr<v8::Platform> platform_;
  std::unique_ptr<CFX_V8ArrayBufferAllocator> array_buffer_allocator_;
  std::unique_ptr<v8::Isolate, CFX_V8IsolateDeleter> isolate_;
};

#endif  // TESTING_V8_TEST_ENVIRONMENT_H_
