// Copyright 2019 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string.h>

#include <memory>

#include "public/fpdf_ext.h"

#ifdef PDF_ENABLE_V8
#include "testing/free_deleter.h"
#include "testing/v8_initializer.h"
#include "v8/include/v8-platform.h"
#include "v8/include/v8.h"
#endif

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else  // Linux
#include <linux/limits.h>
#include <unistd.h>
#endif  // _WIN32

namespace {

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

// Initialize the library once for all runs of the fuzzer.
struct TestCase {
  TestCase() {
#ifdef PDF_ENABLE_V8
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
    platform = InitializeV8ForPDFiumWithStartupData(
        ProgramPath(), std::string(), &snapshot_blob);
#else
    platform = InitializeV8ForPDFium(ProgramPath());
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
#endif  // PDF_ENABLE_V8

    memset(&config, '\0', sizeof(config));
    config.version = 2;
    config.m_pUserFontPaths = nullptr;
    config.m_pIsolate = nullptr;
    config.m_v8EmbedderSlot = 0;
    FPDF_InitLibraryWithConfig(&config);

    memset(&unsupport_info, '\0', sizeof(unsupport_info));
    unsupport_info.version = 1;
    unsupport_info.FSDK_UnSupport_Handler = [](UNSUPPORT_INFO*, int) {};
    FSDK_SetUnSpObjProcessHandler(&unsupport_info);
  }

#ifdef PDF_ENABLE_V8
  std::unique_ptr<v8::Platform> platform;
  v8::StartupData snapshot_blob;
#endif

  FPDF_LIBRARY_CONFIG config;
  UNSUPPORT_INFO unsupport_info;
};

// pdf_fuzzer_init.cc and pdfium_fuzzer_init_public.cc are mutually exclusive
// and should not be built together. They deliberately have the same global
// variable.
static TestCase* g_test_case = new TestCase();
