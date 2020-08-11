// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/v8_test_environment.h"

#include <string>

#include "core/fxcrt/fx_system.h"
#include "testing/v8_initializer.h"
#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8-platform.h"
#include "v8/include/v8.h"

namespace {

V8TestEnvironment* g_environment = nullptr;

}  // namespace

V8TestEnvironment::V8TestEnvironment(const char* exe_name)
    : exe_path_(exe_name) {
  ASSERT(!g_environment);
  g_environment = this;
}

V8TestEnvironment::~V8TestEnvironment() {
  ASSERT(g_environment);
  g_environment = nullptr;

#ifdef V8_USE_EXTERNAL_STARTUP_DATA
  if (v8_snapshot_)
    free(const_cast<char*>(v8_snapshot_->data));
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
}

// static
V8TestEnvironment* V8TestEnvironment::GetInstance() {
  return g_environment;
}

// static
void V8TestEnvironment::PumpPlatformMessageLoop(v8::Isolate* isolate) {
  v8::Platform* platform = GetInstance()->platform();
  while (v8::platform::PumpMessageLoop(platform, isolate))
    continue;
}

void V8TestEnvironment::SetUp() {
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
  if (v8_snapshot_) {
    platform_ = InitializeV8ForPDFiumWithStartupData(exe_path_, std::string(),
                                                     std::string(), nullptr);
  } else {
    v8_snapshot_ = std::make_unique<v8::StartupData>();
    platform_ = InitializeV8ForPDFiumWithStartupData(
        exe_path_, std::string(), std::string(), v8_snapshot_.get());
  }
#else
  platform_ = InitializeV8ForPDFium(exe_path_);
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
}

void V8TestEnvironment::TearDown() {
  v8::V8::ShutdownPlatform();
}
