// Copyright 2019 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/fuzzers/pdf_fuzzer_init_public.h"

#include <string.h>  // For memset()

#include <string>

#include "testing/fuzzers/pdfium_fuzzer_util.h"

#ifdef PDF_ENABLE_V8
#include "testing/free_deleter.h"
#include "testing/v8_initializer.h"
#ifdef PDF_ENABLE_XFA
#include "testing/fuzzers/xfa_process_state.h"
#endif  // PDF_ENABLE_XFA
#endif  // PDF_ENABLE_V8

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__Fuchsia__)
#include <limits.h>
#include <unistd.h>
#else  // Linux
#include <linux/limits.h>
#include <unistd.h>
#endif  // _WIN32

namespace {

// pdf_fuzzer_init.cc and pdf_fuzzer_init_public.cc are mutually exclusive
// and should not be built together.
PDFFuzzerInitPublic* g_instance = new PDFFuzzerInitPublic();

#ifdef PDF_ENABLE_V8
std::string ProgramPath() {
  std::string result;
#ifdef _WIN32
  char path[MAX_PATH];
  DWORD len = GetModuleFileNameA(nullptr, path, MAX_PATH);
  if (len != 0)
    result = std::string(path, len);
#elif defined(__APPLE__)
  char path[PATH_MAX];
  unsigned int len = PATH_MAX;
  if (!_NSGetExecutablePath(path, &len)) {
    std::unique_ptr<char, pdfium::FreeDeleter> resolved_path(
        realpath(path, nullptr));
    if (resolved_path.get())
      result = std::string(resolved_path.get());
  }
#else  // Linux
  char path[PATH_MAX];
  ssize_t len = readlink("/proc/self/exe", path, PATH_MAX);
  if (len > 0)
    result = std::string(path, len);
#endif
  return result;
}
#endif  // PDF_ENABLE_V8

}  // namespace

PDFFuzzerInitPublic::PDFFuzzerInitPublic() {
#ifdef PDF_ENABLE_V8
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
  platform_ = InitializeV8ForPDFiumWithStartupData(
      ProgramPath(), std::string(), std::string(), &snapshot_blob_);
#else   // V8_USE_EXTERNAL_STARTUP_DATA
  platform_ = InitializeV8ForPDFium(ProgramPath(), std::string());
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
#ifdef PDF_ENABLE_XFA
  allocator_.reset(v8::ArrayBuffer::Allocator::NewDefaultAllocator());
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = allocator_.get();
  isolate_.reset(v8::Isolate::New(create_params));
#endif  // PDF_ENABLE_XFA
#endif  // PDF_ENABLE_V8
  memset(&config_, '\0', sizeof(config_));
  config_.version = 3;
  config_.m_pUserFontPaths = nullptr;
  config_.m_pPlatform = nullptr;
  config_.m_pIsolate = nullptr;
  config_.m_v8EmbedderSlot = 0;
#ifdef PDF_ENABLE_V8
  config_.m_pPlatform = platform_.get();
  config_.m_pIsolate = isolate_.get();
#endif  // PDF_ENABLE_V8
  FPDF_InitLibraryWithConfig(&config_);

  memset(&unsupport_info_, '\0', sizeof(unsupport_info_));
  unsupport_info_.version = 1;
  unsupport_info_.FSDK_UnSupport_Handler = [](UNSUPPORT_INFO*, int) {};
  FSDK_SetUnSpObjProcessHandler(&unsupport_info_);

#ifdef PDF_ENABLE_XFA
  xfa_process_state_ =
      std::make_unique<XFAProcessState>(platform_.get(), isolate_.get());
  FPDF_SetFuzzerPerProcessState(xfa_process_state_.get());
#endif
}

PDFFuzzerInitPublic::~PDFFuzzerInitPublic() {
  FPDF_SetFuzzerPerProcessState(nullptr);
}
