// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_V8_INITIALIZER_H_
#define TESTING_V8_INITIALIZER_H_

#include <memory>
#include <string>

#ifndef PDF_ENABLE_V8
#error "V8 must be enabled"
#endif

namespace v8 {
class Platform;
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
class StartupData;
#endif
}  // namespace v8

#ifdef V8_USE_EXTERNAL_STARTUP_DATA
// |snapshot_blob| is an optional out parameter.
std::unique_ptr<v8::Platform> InitializeV8ForPDFiumWithStartupData(
    const std::string& exe_path,
    const std::string& bin_dir,
    v8::StartupData* snapshot_blob);
#else
std::unique_ptr<v8::Platform> InitializeV8ForPDFium(
    const std::string& exe_path);
#endif

#endif  // TESTING_V8_INITIALIZER_H_
