// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/xfa/cjx_object.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/xfa_js_embedder_test.h"

class CJXObjectEmbedderTest : public XFAJSEmbedderTest {};

// Should not crash, but document is not valid.
TEST_F(CJXObjectEmbedderTest, Bug1327884) {
  ASSERT_FALSE(OpenDocument("bug_1327884.pdf"));
}

// Should not CHECK(), but document is uninteresting.
TEST_F(CJXObjectEmbedderTest, Bug1333298) {
  ASSERT_TRUE(OpenDocument("bug_1333298.pdf"));
}
