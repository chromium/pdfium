// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_fontmapper.h"

#include "testing/gtest/include/gtest/gtest.h"

// Deliberately give this global variable external linkage.
char g_maybe_changes = '\xff';

TEST(CFX_FontMapper, MakeTag) {
  EXPECT_EQ(0x61626364u, CFX_FontMapper::MakeTag('a', 'b', 'c', 'd'));
  EXPECT_EQ(0x00000000u, CFX_FontMapper::MakeTag('\0', '\0', '\0', '\0'));
  EXPECT_EQ(0xfffe0a08u, CFX_FontMapper::MakeTag('\xff', '\xfe', '\n', '\b'));
  EXPECT_EQ(0xffffffffu,
            CFX_FontMapper::MakeTag('\xff', '\xff', '\xff', '\xff'));
  EXPECT_EQ(0xffffffffu,
            CFX_FontMapper::MakeTag(g_maybe_changes, '\xff', '\xff', '\xff'));
}
