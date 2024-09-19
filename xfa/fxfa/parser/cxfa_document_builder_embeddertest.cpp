// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class CXFASimpleParserEmbedderTest : public EmbedderTest {};

TEST_F(CXFASimpleParserEmbedderTest, Bug216) {
  ASSERT_TRUE(OpenDocument("bug_216.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  EXPECT_TRUE(page);
}

TEST_F(CXFASimpleParserEmbedderTest, Bug709793) {
  ASSERT_TRUE(OpenDocument("bug_709793.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  EXPECT_TRUE(page);
}
