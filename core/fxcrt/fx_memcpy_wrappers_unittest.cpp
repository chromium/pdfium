// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_memcpy_wrappers.h"

#include "testing/gtest/include/gtest/gtest.h"

TEST(fxcrt, FXSYS_memset) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(nullptr, FXSYS_memset(nullptr, 0, 0));
}

TEST(fxcrt, FXSYS_wmemset) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(nullptr, FXSYS_wmemset(nullptr, 0, 0));
}

TEST(fxcrt, FXSYS_memcpy) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(nullptr, FXSYS_memcpy(nullptr, nullptr, 0));
}

TEST(fxcrt, FXSYS_wmemcpy) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(nullptr, FXSYS_wmemcpy(nullptr, nullptr, 0));
}

TEST(fxcrt, FXSYS_memmove) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(nullptr, FXSYS_memmove(nullptr, nullptr, 0));
}

TEST(fxcrt, FXSYS_wmemmove) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(nullptr, FXSYS_wmemmove(nullptr, nullptr, 0));
}

TEST(fxcrt, FXSYS_memcmp) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(0, FXSYS_memcmp(nullptr, nullptr, 0));
}

TEST(fxcrt, FXSYS_wmemcmp) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(0, FXSYS_wmemcmp(nullptr, nullptr, 0));
}

TEST(fxcrt, FXSYS_memchr) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(nullptr, FXSYS_memchr(nullptr, 0, 0));
}

TEST(fxcrt, FXSYS_wmemchr) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(nullptr, FXSYS_wmemchr(nullptr, 0, 0));
}
