// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstring>
#include <memory>
#include <string>

#include "build/build_config.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "public/cpp/fpdf_scopers.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_constants.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

using FPDFParserDecodeEmbedderTest = EmbedderTest;
using pdfium::kBlankPage612By792Checksum;

TEST_F(FPDFParserDecodeEmbedderTest, Bug_552046) {
  // Tests specifying multiple image filters for a stream. Should not cause a
  // crash when rendered.
  EXPECT_TRUE(OpenDocument("bug_552046.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ScopedFPDFBitmap bitmap = RenderLoadedPage(page);
  CompareBitmap(bitmap.get(), 612, 792, kBlankPage612By792Checksum);
  UnloadPage(page);
}

TEST_F(FPDFParserDecodeEmbedderTest, Bug_555784) {
  // Tests bad input to the run length decoder that caused a heap overflow.
  // Should not cause a crash when rendered.
  EXPECT_TRUE(OpenDocument("bug_555784.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ScopedFPDFBitmap bitmap = RenderLoadedPage(page);
  CompareBitmap(bitmap.get(), 612, 792, kBlankPage612By792Checksum);
  UnloadPage(page);
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_Bug_455199 DISABLED_Bug_455199
#else
#define MAYBE_Bug_455199 Bug_455199
#endif
TEST_F(FPDFParserDecodeEmbedderTest, MAYBE_Bug_455199) {
  // Tests object numbers with a value > 01000000.
  // Should open successfully.
  EXPECT_TRUE(OpenDocument("bug_455199.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ScopedFPDFBitmap bitmap = RenderLoadedPage(page);

  CompareBitmap(bitmap.get(), 200, 200, pdfium::kHelloWorldChecksum);
  UnloadPage(page);
}
