// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/page/test_with_page_module.h"

#include "core/fpdfapi/page/cpdf_pagemodule.h"

void TestWithPageModule::SetUp() {
  pdfium::InitializePageModule();
}

void TestWithPageModule::TearDown() {
  pdfium::DestroyPageModule();
}
