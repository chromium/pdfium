// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/edit/cpdf_npagetooneexporter.h"

#include "core/fxcrt/bytestring.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(NPageToOneExporter, GenerateSubPageContentStream) {
  // TODO(crbug.com/356075497): Content stream should not contain floats with
  // exponents.
  const char kExpected[] = "q\n0.5 0 0 0.5 1e-06 1e+12 cm\n/foo Do Q\n";

  CPDF_NPageToOneExporter::NupPageSettings settings;
  settings.sub_page_start_point = {0.000001f, 1000000000000.0f};
  settings.scale = 0.5f;
  EXPECT_EQ(kExpected,
            CPDF_NPageToOneExporter::GenerateSubPageContentStreamForTesting(
                "foo", settings));
}
