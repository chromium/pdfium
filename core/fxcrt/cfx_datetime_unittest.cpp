// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/cfx_datetime.h"

#include "core/fxcrt/fx_extension.h"
#include "testing/gtest/include/gtest/gtest.h"

class FakeTimeTest : public ::testing::Test {
 public:
  void SetUp() override {
    // Arbitrary, picked descending digits, 2020-04-23 15:05:21.
    FXSYS_SetTimeFunction([]() -> time_t { return 1587654321; });
    FXSYS_SetLocaltimeFunction([](const time_t* t) { return gmtime(t); });
  }

  void TearDown() override {
    FXSYS_SetTimeFunction(nullptr);
    FXSYS_SetLocaltimeFunction(nullptr);
  }
};

TEST_F(FakeTimeTest, Now) {
  CFX_DateTime dt = CFX_DateTime::Now();
  EXPECT_EQ(2020, dt.GetYear());
  EXPECT_EQ(4, dt.GetMonth());
  EXPECT_EQ(23, dt.GetDay());
  EXPECT_EQ(15, dt.GetHour());
  EXPECT_EQ(5, dt.GetMinute());
  EXPECT_EQ(21, dt.GetSecond());
  EXPECT_EQ(0, dt.GetMillisecond());
}
