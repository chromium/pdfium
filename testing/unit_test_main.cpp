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
#include "testing/v8_test_environment.h"
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
  // V8 test environment will be deleted by gtest.
  AddGlobalTestEnvironment(new V8TestEnvironment(argv[0]));
#endif  // PDF_ENABLE_V8

  InitializePDFTestEnvironment();

#ifdef PDF_ENABLE_XFA
  // XFA test environment will be deleted by gtest.
  InitializeXFATestEnvironment();
#endif  // PDF_ENABLE_XFA

  testing::InitGoogleTest(&argc, argv);
  testing::InitGoogleMock(&argc, argv);

  int ret_val = RUN_ALL_TESTS();

  DestroyPDFTestEnvironment();


  return ret_val;
}
