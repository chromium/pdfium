// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"

#include "../../include/fxcrt/fx_basic.h"

TEST(fxcrt, ByteStringCNull) {
  CFX_ByteStringC null_string;
  EXPECT_EQ(null_string.GetPtr(), nullptr);
  EXPECT_EQ(null_string.GetLength(), 0);
  EXPECT_TRUE(null_string.IsEmpty());

  CFX_ByteStringC another_null_string;
  EXPECT_TRUE(null_string == another_null_string);

  CFX_ByteString copied_null_string(null_string);
  EXPECT_EQ(null_string.GetPtr(), nullptr);
  EXPECT_EQ(null_string.GetLength(), 0);
  EXPECT_TRUE(null_string.IsEmpty());
  EXPECT_TRUE(null_string == another_null_string);

  CFX_ByteStringC empty_string("");
  EXPECT_EQ(null_string.GetPtr(), nullptr);
  EXPECT_EQ(null_string.GetLength(), 0);
  EXPECT_TRUE(null_string.IsEmpty());
  EXPECT_TRUE(null_string == empty_string);

  CFX_ByteStringC non_null_string("a");
  EXPECT_FALSE(null_string == non_null_string);

  // TODO(tsepez): fix assignment of a null ptr to a CFX_ByteStringC.
}

TEST(fxcrt, ByteStringCGetID) {
  CFX_ByteStringC null_string;
  EXPECT_EQ(null_string.GetID(), 0u);

  CFX_ByteStringC empty_string("");
  EXPECT_EQ(empty_string.GetID(), 0u);
}
