// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/pdf_test_environment.h"

#include "core/fxge/cfx_gemodule.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/utils/path_service.h"

PDFTestEnvironment::PDFTestEnvironment() = default;

PDFTestEnvironment::~PDFTestEnvironment() = default;

// testing::Environment:
void PDFTestEnvironment::SetUp() {
  ASSERT_TRUE(PathService::GetExecutableDir(&font_path_));
  font_path_.push_back(PATH_SEPARATOR);
  font_path_.append("test_fonts");
  font_paths_[0] = font_path_.c_str();
  font_paths_[1] = nullptr;
  CFX_GEModule::Create(font_paths_);
}

void PDFTestEnvironment::TearDown() {
  CFX_GEModule::Destroy();
}
