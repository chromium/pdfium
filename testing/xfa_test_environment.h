// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_XFA_TEST_ENVIRONMENT_H_
#define TESTING_XFA_TEST_ENVIRONMENT_H_

#include "testing/gtest/include/gtest/gtest.h"

#ifndef PDF_ENABLE_XFA
#error "XFA must be enabled"
#endif

class XFATestEnvironment : public testing::Environment {
 public:
  XFATestEnvironment();
  ~XFATestEnvironment();

  // testing::TestEnvironment:
  void SetUp() override;
  void TearDown() override;
};

#endif  // TESTING_XFA_TEST_ENVIRONMENT_H_
