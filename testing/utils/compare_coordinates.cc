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
