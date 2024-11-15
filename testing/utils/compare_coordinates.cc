// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/utils/compare_coordinates.h"

#include "public/fpdfview.h"
#include "testing/gtest/include/gtest/gtest.h"

void CompareFS_RECTF(const FS_RECTF& val1, const FS_RECTF& val2) {
  EXPECT_FLOAT_EQ(val1.left, val2.left);
  EXPECT_FLOAT_EQ(val1.top, val2.top);
  EXPECT_FLOAT_EQ(val1.right, val2.right);
  EXPECT_FLOAT_EQ(val1.top, val2.top);
}

void CompareFS_MATRIX(const FS_MATRIX& val1, const FS_MATRIX& val2) {
  EXPECT_FLOAT_EQ(val1.a, val2.a);
  EXPECT_FLOAT_EQ(val1.b, val2.b);
  EXPECT_FLOAT_EQ(val1.c, val2.c);
  EXPECT_FLOAT_EQ(val1.d, val2.d);
  EXPECT_FLOAT_EQ(val1.e, val2.e);
  EXPECT_FLOAT_EQ(val1.f, val2.f);
}
