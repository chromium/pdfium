// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>

#include "core/fxcrt/fx_memory.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

#ifdef PDF_ENABLE_V8
#include "testing/v8_initializer.h"
#include "v8/include/v8-platform.h"
#include "v8/include/v8.h"
#endif  // PDF_ENABLE_V8

#ifdef PDF_ENABLE_XFA
#include "testing/xfa_unit_test_support.h"
#endif  // PDF_ENABLE_XFA

// Can't use gtest-provided main since we need to initialize partition
// alloc before invoking any test.
int main(int argc, char** argv) {
  FXMEM_InitializePartitionAlloc();

#ifdef PDF_ENABLE_V8
  std::unique_ptr<v8::Platform> platform;
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
  static v8::StartupData* snapshot = new v8::StartupData;
  platform =
      InitializeV8ForPDFiumWithStartupData(argv[0], std::string(), snapshot);
#else  // V8_USE_EXTERNAL_STARTUP_DATA
  platform = InitializeV8ForPDFium(argv[0]);
#endif  // V8_USE_EXTERNAL_STARTUP_DATA
#endif  // PDF_ENABLE_V8

  InitializePDFTestEnvironment();
#ifdef PDF_ENABLE_XFA
  InitializeXFATestEnvironment();
#endif  // PDF_ENABLE_XFA

  testing::InitGoogleTest(&argc, argv);
  testing::InitGoogleMock(&argc, argv);

  int ret_val = RUN_ALL_TESTS();

  DestroyPDFTestEnvironment();
  // NOTE: XFA test environment, if present, destroyed by gtest.

#ifdef PDF_ENABLE_V8
  v8::V8::ShutdownPlatform();
#endif  // PDF_ENABLE_V8

  return ret_val;
}
