// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_EMBEDDER_TEST_ENVIRONMENT_H_
#define TESTING_EMBEDDER_TEST_ENVIRONMENT_H_

#include <string>

#include "public/fpdfview.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_fonts.h"

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

  void AddFlags(int argc, char** argv);

  bool write_pngs() const { return write_pngs_; }

 private:
  void AddFlag(const std::string& flag);

  FPDF_RENDERER_TYPE renderer_type_;
  bool write_pngs_ = false;
  TestFonts test_fonts_;
};

#endif  // TESTING_EMBEDDER_TEST_ENVIRONMENT_H_
