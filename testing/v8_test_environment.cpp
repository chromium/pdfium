// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/v8_test_environment.h"

#include <memory>
#include <string>

#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_system.h"
#include "testing/v8_initializer.h"
#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8-isolate.h"
#include "v8/include/v8-platform.h"
#include "v8/include/v8-snapshot.h"

namespace {

V8TestEnvironment* g_environment = nullptr;

}  // namespace

V8TestEnvironment::V8TestEnvironment(const char* exe_name)
    : exe_path_(exe_name),
      array_buffer_allocator_(std::make_unique<CFX_V8ArrayBufferAllocator>()) {
  DCHECK(!g_environment);
  g_environment = this;
}

V8TestEnvironment::~V8TestEnvironment() {
  DCHECK(g_environment);

#ifdef V8_USE_EXTERNAL_STARTUP_DATA
  if (startup_data_)
    free(const_cast<char*>(startup_data_->data));
#endif  // V8_USE_EXTERNAL_STARTUP_DATA

  g_environment = nullptr;
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
  if (startup_data_) {
    platform_ = InitializeV8ForPDFiumWithStartupData(exe_path_, std::string(),
                                                     std::string(), nullptr);
  } else {
    startup_data_ = std::make_unique<v8::StartupData>();
    platform_ = InitializeV8ForPDFiumWithStartupData(
        exe_path_, std::string(), std::string(), startup_data_.get());
  }
#else
  platform_ = InitializeV8ForPDFium(std::string(), exe_path_);
#endif  // V8_USE_EXTERNAL_STARTUP_DATA

  v8::Isolate::CreateParams params;
  params.array_buffer_allocator = array_buffer_allocator_.get();
  isolate_.reset(v8::Isolate::New(params));
}

void V8TestEnvironment::TearDown() {
  isolate_.reset();
  ShutdownV8ForPDFium();
}
