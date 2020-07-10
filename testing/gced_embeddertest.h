// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_GCED_EMBEDDER_TEST_H_
#define TESTING_GCED_EMBEDDER_TEST_H_

#include <memory>

#include "testing/js_embedder_test.h"

class GCedEmbedderTest : public JSEmbedderTest {
 public:
  void PumpPlatformMessageLoop();
};

#endif  // TESTING_GCED_EMBEDDER_TEST_H_
