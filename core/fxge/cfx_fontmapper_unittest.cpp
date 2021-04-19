// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_fontmapper.h"

#include "testing/gtest/include/gtest/gtest.h"

// Deliberately give this global variable external linkage.
char g_maybe_changes = '\xff';

TEST(CFX_FontMapper, IsStandardFontName) {
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Courier"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Courier-Bold"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Courier-BoldOblique"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Courier-Oblique"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Helvetica"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Helvetica-Bold"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Helvetica-BoldOblique"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Helvetica-Oblique"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Times-Roman"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Times-Bold"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Times-BoldItalic"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Times-Italic"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("Symbol"));
  EXPECT_TRUE(CFX_FontMapper::IsStandardFontName("ZapfDingbats"));

  EXPECT_FALSE(CFX_FontMapper::IsStandardFontName("Courie"));
  EXPECT_FALSE(CFX_FontMapper::IsStandardFontName("Courier-"));
  EXPECT_FALSE(CFX_FontMapper::IsStandardFontName("Helvetica+Bold"));
  EXPECT_FALSE(CFX_FontMapper::IsStandardFontName("YapfDingbats"));
}

TEST(CFX_FontMapper, MakeTag) {
  EXPECT_EQ(0x61626364u, CFX_FontMapper::MakeTag('a', 'b', 'c', 'd'));
  EXPECT_EQ(0x00000000u, CFX_FontMapper::MakeTag('\0', '\0', '\0', '\0'));
  EXPECT_EQ(0xfffe0a08u, CFX_FontMapper::MakeTag('\xff', '\xfe', '\n', '\b'));
  EXPECT_EQ(0xffffffffu,
            CFX_FontMapper::MakeTag('\xff', '\xff', '\xff', '\xff'));
  EXPECT_EQ(0xffffffffu,
            CFX_FontMapper::MakeTag(g_maybe_changes, '\xff', '\xff', '\xff'));
}
