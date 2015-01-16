// Copyright (c) 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class BasicEmbeddertest : public EmbedderTest {
};

TEST_F(BasicEmbeddertest, GetPageCount) {
  EXPECT_TRUE(OpenDocument("testing/resources/about_blank.pdf"));
  EXPECT_EQ(1, GetPageCount());
}

TEST_F(BasicEmbeddertest, GetFirstPageNum) {
  EXPECT_TRUE(OpenDocument("testing/resources/about_blank.pdf"));
  EXPECT_EQ(0, GetFirstPageNum());
}
