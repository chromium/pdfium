// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/cfx_datetime.h"

#include "core/fxcrt/fake_time_test.h"

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
