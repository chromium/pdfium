// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "testing/gtest/include/gtest/gtest.h"

class FPDFParserEmbeddertest : public EmbedderTest {
};

Test_F(FPDFParserEmbeddertest, LoadError_454695) {
    EXPECT_TRUE(OpenDocument("testing/resources/bug_454695.pdf")));
}

