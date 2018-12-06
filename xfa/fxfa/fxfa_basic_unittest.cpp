// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/fxfa_basic.h"

#include "core/fxcrt/bytestring.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(FXFABasic, AttrHashMatchesString) {
#undef ATTR____
#define ATTR____(a, b, c) EXPECT_EQ(a, FX_HashCode_GetAsIfW(b, false));
#include "xfa/fxfa/parser/attributes.inc"
#undef ATTR____
}

TEST(FXFABasic, AttrHashOrder) {
  uint32_t so_far = 0;
#undef ATTR____
#define ATTR____(a, b, c) \
  EXPECT_LT(so_far, a);   \
  so_far = a;
#include "xfa/fxfa/parser/attributes.inc"
#undef ATTR____
}

TEST(FXFABasic, ValueHashMatchesString) {
#undef VALUE____
#define VALUE____(a, b, c) EXPECT_EQ(a, FX_HashCode_GetAsIfW(b, false));
#include "xfa/fxfa/parser/attribute_values.inc"
#undef VALUE____
}

TEST(FXFABasic, ValueHashOrder) {
  uint32_t so_far = 0;
#undef VALUE____
#define VALUE____(a, b, c) \
  EXPECT_LT(so_far, a);    \
  so_far = a;
#include "xfa/fxfa/parser/attribute_values.inc"
#undef VALUE____
}

TEST(FXFABasic, ElementHashMatchesString) {
#undef ELEM____
#undef ELEM_HIDDEN____
#define ELEM____(a, b, c) EXPECT_EQ(a, FX_HashCode_GetAsIfW(b, false));
#define ELEM_HIDDEN____(a)
#include "xfa/fxfa/parser/elements.inc"
#undef ELEM____
#undef ELEM_HIDDEN____
}

TEST(FXFABasic, ElementHashOrder) {
  uint32_t so_far = 0;
#undef ELEM____
#undef ELEM_HIDDEN____
#define ELEM____(a, b, c) \
  EXPECT_LT(so_far, a);   \
  so_far = a;
#define ELEM_HIDDEN____(a)
#include "xfa/fxfa/parser/elements.inc"
#undef ELEM____
#undef ELEM_HIDDEN____
}
