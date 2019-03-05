// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/xfa_js_embedder_test.h"

class CFXJSE_AppEmbedderTest : public XFAJSEmbedderTest {};

// Should not crash.
TEST_F(CFXJSE_AppEmbedderTest, BUG_1252) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  EXPECT_FALSE(Execute("app.activeDocs()"));
}
