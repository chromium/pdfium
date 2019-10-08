// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/v8_initializer.h"

#include <cstring>

#include "public/fpdfview.h"
#include "testing/utils/file_util.h"
#include "testing/utils/path_service.h"
#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8.h"

namespace {

#ifdef V8_USE_EXTERNAL_STARTUP_DATA
// Returns the full path for an external V8 data file based on either
// the currect exectuable path or an explicit override.
std::string GetFullPathForSnapshotFile(const std::string& exe_path,
                                       const std::string& bin_dir,
                                       const std::string& filename) {
  std::string result;
  if (!bin_dir.empty()) {
    result = bin_dir;
    if (*bin_dir.rbegin() != PATH_SEPARATOR) {
      result += PATH_SEPARATOR;
    }
  } else if (!exe_path.empty()) {
    size_t last_separator = exe_path.rfind(PATH_SEPARATOR);
    if (last_separator != std::string::npos) {
      result = exe_path.substr(0, last_separator + 1);
    }
  }
  result += filename;
  return result;
}

bool GetExternalData(const std::string& exe_path,
                     const std::string& bin_dir,
                     const std::string& filename,
                     v8::StartupData* result_data) {
  std::string full_path =
      GetFullPathForSnapshotFile(exe_path, bin_dir, filename);
  size_t data_length = 0;
  std::unique_ptr<char, pdfium::FreeDeleter> data_buffer =
      GetFileContents(full_path.c_str(), &data_length);
  if (!data_buffer)
    return false;

  result_data->data = data_buffer.release();
  result_data->raw_size = data_length;
  return true;
}
#endif  // V8_USE_EXTERNAL_STARTUP_DATA

std::unique_ptr<v8::Platform> InitializeV8Common(const std::string& exe_path) {
  v8::V8::InitializeICUDefaultLocation(exe_path.c_str());

  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());

  const char* recommended_v8_flags = FPDF_GetRecommendedV8Flags();
  v8::V8::SetFlagsFromString(recommended_v8_flags);

  // By enabling predictable mode, V8 won't post any background tasks.
  // By enabling GC, it makes it easier to chase use-after-free.
  static const char kAdditionalV8Flags[] = "--predictable --expose-gc";
  v8::V8::SetFlagsFromString(kAdditionalV8Flags);

  v8::V8::Initialize();
  return platform;
}

}  // namespace

#ifdef V8_USE_EXTERNAL_STARTUP_DATA
std::unique_ptr<v8::Platform> InitializeV8ForPDFiumWithStartupData(
    const std::string& exe_path,
    const std::string& bin_dir,
    v8::StartupData* snapshot_blob) {
  std::unique_ptr<v8::Platform> platform = InitializeV8Common(exe_path);
  if (snapshot_blob) {
    if (!GetExternalData(exe_path, bin_dir, "snapshot_blob.bin", snapshot_blob))
      return nullptr;
    v8::V8::SetSnapshotDataBlob(snapshot_blob);
  }
  return platform;
}
#else   // V8_USE_EXTERNAL_STARTUP_DATA
std::unique_ptr<v8::Platform> InitializeV8ForPDFium(
    const std::string& exe_path) {
  return InitializeV8Common(exe_path);
}
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
