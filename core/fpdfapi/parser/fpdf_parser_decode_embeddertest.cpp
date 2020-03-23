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
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

using FPDFParserDecodeEmbedderTest = EmbedderTest;

TEST_F(FPDFParserDecodeEmbedderTest, Bug_552046) {
  // Tests specifying multiple image filters for a stream. Should not cause a
  // crash when rendered.
  EXPECT_TRUE(OpenDocument("bug_552046.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ScopedFPDFBitmap bitmap = RenderLoadedPage(page);
  CompareBitmap(bitmap.get(), 612, 792, "1940568c9ba33bac5d0b1ee9558c76b3");
  UnloadPage(page);
}

TEST_F(FPDFParserDecodeEmbedderTest, Bug_555784) {
  // Tests bad input to the run length decoder that caused a heap overflow.
  // Should not cause a crash when rendered.
  EXPECT_TRUE(OpenDocument("bug_555784.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ScopedFPDFBitmap bitmap = RenderLoadedPage(page);
  CompareBitmap(bitmap.get(), 612, 792, "1940568c9ba33bac5d0b1ee9558c76b3");
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
#if defined(OS_MACOSX)
  const char kExpectedMd5sum[] = "c38b75e16a13852aee3b97d77a0f0ee7";
#elif defined(OS_WIN)
  const char kExpectedMd5sum[] = "795b7ce1626931aa06af0fa23b7d80bb";
#else
  const char kExpectedMd5sum[] = "2baa4c0e1758deba1b9c908e1fbd04ed";
#endif
  CompareBitmap(bitmap.get(), 200, 200, kExpectedMd5sum);
  UnloadPage(page);
}
