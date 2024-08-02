// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/xfa_js_embedder_test.h"

class CJXHostPseudoModelEmbedderTest : public XFAJSEmbedderTest {};

// Should not crash.
TEST_F(CJXHostPseudoModelEmbedderTest, Bug1256) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  EXPECT_TRUE(Execute("$host.openList(1)"));
}
