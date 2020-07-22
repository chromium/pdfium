// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_V8_TEST_ENVIRONMENT_H_
#define TESTING_V8_TEST_ENVIRONMENT_H_

#include <memory>

#include "testing/gtest/include/gtest/gtest.h"

#ifndef PDF_ENABLE_V8
#error "V8 must be enabled"
#endif  // PDF_ENABLE_V8

namespace v8 {
class Isolate;
class Platform;
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
class StartupData;
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
}  // namespace v8

class TestLoader;

class V8TestEnvironment : public testing::Environment {
 public:
  explicit V8TestEnvironment(const char* exe_path);
  ~V8TestEnvironment() override;

  // Note: GetInstance() does not create one if it does not exist.
  static V8TestEnvironment* GetInstance();
  static void PumpPlatformMessageLoop(v8::Isolate* pIsolate);

  // testing::Environment:
  void SetUp() override;
  void TearDown() override;

  v8::Platform* platform() const { return platform_.get(); }

 private:
  const char* const exe_path_;
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
  std::unique_ptr<v8::StartupData> v8_snapshot_;
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
  std::unique_ptr<v8::Platform> platform_;
};

#endif  // TESTING_V8_TEST_ENVIRONMENT_H_
