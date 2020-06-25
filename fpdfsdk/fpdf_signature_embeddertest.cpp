// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_signature.h"
#include "testing/embedder_test.h"

class FPDFSignatureEmbedderTest : public EmbedderTest {};

TEST_F(FPDFSignatureEmbedderTest, GetSignatureCount) {
  EXPECT_TRUE(OpenDocument("two_signatures.pdf"));
  EXPECT_EQ(2, FPDF_GetSignatureCount(document()));
}

TEST_F(FPDFSignatureEmbedderTest, GetSignatureCountZero) {
  EXPECT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_EQ(0, FPDF_GetSignatureCount(document()));

  // Provide no document.
  EXPECT_EQ(-1, FPDF_GetSignatureCount(nullptr));
}
