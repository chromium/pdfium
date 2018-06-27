// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/autorestorer.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(fxcrt, AutoRestorer) {
  int x = 5;
  {
    AutoRestorer<int> restorer(&x);
    x = 6;
    EXPECT_EQ(6, x);
  }
  EXPECT_EQ(5, x);
  {
    AutoRestorer<int> restorer(&x);
    x = 6;
    EXPECT_EQ(6, x);
    restorer.AbandonRestoration();
  }
  EXPECT_EQ(6, x);
}
