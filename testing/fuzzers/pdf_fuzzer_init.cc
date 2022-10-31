// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string.h>

#include "public/fpdfview.h"

// Initialize the library once for all runs of the fuzzer.
struct TestCase {
  TestCase() {
    memset(&config, '\0', sizeof(config));
    config.version = 2;
    config.m_pUserFontPaths = nullptr;
    config.m_pIsolate = nullptr;
    config.m_v8EmbedderSlot = 0;
    FPDF_InitLibraryWithConfig(&config);
  }
  FPDF_LIBRARY_CONFIG config;
};

// pdf_fuzzer_init.cc and pdfium_fuzzer_init_public.cc are mutually exclusive
// and should not be built together. They deliberately have the same global
// variable.
static TestCase* g_test_case = new TestCase();
