// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "build/build_config.h"
#include "core/fxcrt/fx_memory.h"
#include "testing/embedder_test_environment.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

#ifdef PDF_ENABLE_V8
#include "testing/v8_test_environment.h"
#endif  // PDF_ENABLE_V8

// Can't use gtest-provided main since we need to create our own
// testing environment which needs the executable path in order to
// find the external V8 binary data files.
int main(int argc, char** argv) {
  FX_InitializeMemoryAllocators();

#ifdef PDF_ENABLE_V8
  // The env will be deleted by gtest.
  AddGlobalTestEnvironment(new V8TestEnvironment(argv[0]));
#endif  // PDF_ENABLE_V8

  // The env will be deleted by gtest.
  AddGlobalTestEnvironment(new EmbedderTestEnvironment);

  testing::InitGoogleTest(&argc, argv);
  testing::InitGoogleMock(&argc, argv);

  // Anything remaining in argc/argv is an embedder_tests flag.
  EmbedderTestEnvironment::GetInstance()->AddFlags(argc, argv);

  return RUN_ALL_TESTS();
}
