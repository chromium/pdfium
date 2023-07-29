// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_memory.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/pdf_test_environment.h"

#if defined(PDF_USE_PARTITION_ALLOC)
#include "testing/allocator_shim_config.h"
#endif

#ifdef PDF_ENABLE_V8
#include "testing/v8_test_environment.h"
#ifdef PDF_ENABLE_XFA
#include "testing/xfa_test_environment.h"
#endif  // PDF_ENABLE_XFA
#endif  // PDF_ENABLE_V8

// Can't use gtest-provided main since we need to initialize partition
// alloc before invoking any test, and add test environments.
int main(int argc, char** argv) {
#if defined(PDF_USE_PARTITION_ALLOC)
  pdfium::ConfigurePartitionAllocShimPartitionForTest();
#endif  // defined(PDF_USE_PARTITION_ALLOC)

  FX_InitializeMemoryAllocators();

  // PDF test environment will be deleted by gtest.
  AddGlobalTestEnvironment(new PDFTestEnvironment());

#ifdef PDF_ENABLE_V8
  // V8 test environment will be deleted by gtest.
  AddGlobalTestEnvironment(new V8TestEnvironment(argv[0]));
#ifdef PDF_ENABLE_XFA
  // XFA test environment will be deleted by gtest.
  AddGlobalTestEnvironment(new XFATestEnvironment());
#endif  // PDF_ENABLE_XFA
#endif  // PDF_ENABLE_V8

  testing::InitGoogleTest(&argc, argv);
  testing::InitGoogleMock(&argc, argv);

  return RUN_ALL_TESTS();
}
