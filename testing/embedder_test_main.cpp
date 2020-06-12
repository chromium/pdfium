// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_memory.h"
#include "testing/embedder_test.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

// Can't use gtest-provided main since we need to create our own
// testing environment which needs the executable path in order to
// find the external V8 binary data files.
int main(int argc, char** argv) {
  FXMEM_InitializePartitionAlloc();

  // The env will be deleted by gtest.
  AddGlobalTestEnvironment(new EmbedderTestEnvironment(argv[0]));

  testing::InitGoogleTest(&argc, argv);
  testing::InitGoogleMock(&argc, argv);

  return RUN_ALL_TESTS();
}
