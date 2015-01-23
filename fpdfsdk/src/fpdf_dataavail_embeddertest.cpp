// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "../../testing/embedder_test.h"
#include "../../fpdfsdk/include/fpdfview.h"
#include "../../fpdfsdk/include/fpdfdoc.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFDataAvailEmbeddertest : public EmbedderTest {
};

TEST_F(FPDFDataAvailEmbeddertest, TrailerUnterminated) {
  // Document doesn't even open under XFA but must not crash.
  EXPECT_FALSE(OpenDocument("testing/resources/trailer_unterminated.pdf"));
}

TEST_F(FPDFDataAvailEmbeddertest, TrailerAsHexstring) {
  // Document doesn't even open under XFA but must not crash.
  EXPECT_FALSE(OpenDocument("testing/resources/trailer_as_hexstring.pdf"));
}
