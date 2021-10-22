// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_PDF_TEST_ENVIRONMENT_H_
#define TESTING_PDF_TEST_ENVIRONMENT_H_

#include <string>

#include "build/build_config.h"
#include "testing/gtest/include/gtest/gtest.h"

class PDFTestEnvironment : public testing::Environment {
 public:
  PDFTestEnvironment();
  ~PDFTestEnvironment() override;

  // testing::Environment:
  void SetUp() override;
  void TearDown() override;

 private:
#if defined(OS_LINUX) || defined(OS_CHROMEOS)
  std::string font_path_;
  const char* font_paths_[2];
#endif
};

#endif  // TESTING_PDF_TEST_ENVIRONMENT_H_
