// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_memcpy_wrappers.h"

#include "testing/gtest/include/gtest/gtest.h"

TEST(fxcrt, FXSYSmemset) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(nullptr, UNSAFE_BUFFERS(FXSYS_memset(nullptr, 0, 0)));
}

TEST(fxcrt, FXSYSwmemset) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(nullptr, UNSAFE_BUFFERS(FXSYS_wmemset(nullptr, 0, 0)));
}

TEST(fxcrt, FXSYSmemcpy) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(nullptr, UNSAFE_BUFFERS(FXSYS_memcpy(nullptr, nullptr, 0)));
}

TEST(fxcrt, FXSYSwmemcpy) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(nullptr, UNSAFE_BUFFERS(FXSYS_wmemcpy(nullptr, nullptr, 0)));
}

TEST(fxcrt, FXSYSmemmove) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(nullptr, UNSAFE_BUFFERS(FXSYS_memmove(nullptr, nullptr, 0)));
}

TEST(fxcrt, FXSYSwmemmove) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(nullptr, UNSAFE_BUFFERS(FXSYS_wmemmove(nullptr, nullptr, 0)));
}

TEST(fxcrt, FXSYSmemcmp) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(0, UNSAFE_BUFFERS(FXSYS_memcmp(nullptr, nullptr, 0)));
}

TEST(fxcrt, FXSYSwmemcmp) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(0, UNSAFE_BUFFERS(FXSYS_wmemcmp(nullptr, nullptr, 0)));
}

TEST(fxcrt, FXSYSmemchr) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(nullptr, UNSAFE_BUFFERS(FXSYS_memchr(nullptr, 0, 0)));
}

TEST(fxcrt, FXSYSwmemchr) {
  // Test passes if it does not trigger UBSAN warnings.
  EXPECT_EQ(nullptr, UNSAFE_BUFFERS(FXSYS_wmemchr(nullptr, 0, 0)));
}
