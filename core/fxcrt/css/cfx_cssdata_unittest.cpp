// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/css/cfx_cssdata.h"

#include "core/fxcrt/bytestring.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(CSSDataTest, PropertyHashes) {
  uint32_t max_hash = 0;
#undef CSS_PROP____
#define CSS_PROP____(a, b, c, d)                                       \
  {                                                                    \
    EXPECT_EQ(FX_HashCode_GetAsIfW(b), static_cast<uint32_t>(c)) << b; \
    EXPECT_GT(static_cast<uint32_t>(c), max_hash) << b;                \
    max_hash = c;                                                      \
  }
#include "core/fxcrt/css/properties.inc"
#undef CSS_PROP____
}

TEST(CSSDataTest, PropertyValueHashes) {
  uint32_t max_hash = 0;
#undef CSS_PROP_VALUE____
#define CSS_PROP_VALUE____(a, b, c)                                    \
  {                                                                    \
    EXPECT_EQ(FX_HashCode_GetAsIfW(b), static_cast<uint32_t>(c)) << b; \
    EXPECT_GT(static_cast<uint32_t>(c), max_hash) << b;                \
    max_hash = c;                                                      \
  }
#include "core/fxcrt/css/property_values.inc"
#undef CSS_PROP_VALUE____
}
