// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_EMBEDDER_TEST_ENVIRONMENT_H_
#define TESTING_EMBEDDER_TEST_ENVIRONMENT_H_

#include "public/fpdfview.h"
#include "testing/gtest/include/gtest/gtest.h"

class EmbedderTestEnvironment : public testing::Environment {
 public:
  EmbedderTestEnvironment();
  ~EmbedderTestEnvironment() override;

  // Note: GetInstance() does not create one if it does not exist,
  // so the main program must explicitly add this enviroment.
  static EmbedderTestEnvironment* GetInstance();

  // testing::Environment:
  void SetUp() override;
  void TearDown() override;
};

#endif  // TESTING_EMBEDDER_TEST_ENVIRONMENT_H_
