// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/parser/cxfa_timezoneprovider.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/scoped_set_tz.h"

TEST(CXFA_TimeZoneProviderTest, HourOffsets) {
  {
    ScopedSetTZ scoped_set_tz("UTC");
    EXPECT_EQ(0, CXFA_TimeZoneProvider().GetTimeZoneInMinutes());
  }
  {
    ScopedSetTZ scoped_set_tz("UTC+1");
    EXPECT_EQ(-60, CXFA_TimeZoneProvider().GetTimeZoneInMinutes());
  }
  {
    ScopedSetTZ scoped_set_tz("UTC-1");
    EXPECT_EQ(60, CXFA_TimeZoneProvider().GetTimeZoneInMinutes());
  }
  {
    ScopedSetTZ scoped_set_tz("UTC+14");
    EXPECT_EQ(-840, CXFA_TimeZoneProvider().GetTimeZoneInMinutes());
  }
  {
    ScopedSetTZ scoped_set_tz("UTC-14");
    EXPECT_EQ(840, CXFA_TimeZoneProvider().GetTimeZoneInMinutes());
  }
}

TEST(CXFA_TimeZoneProviderTest, HalfHourOffsets) {
  {
    ScopedSetTZ scoped_set_tz("UTC+0:30");
    EXPECT_EQ(-30, CXFA_TimeZoneProvider().GetTimeZoneInMinutes());
  }
  {
    ScopedSetTZ scoped_set_tz("UTC-0:30");
    EXPECT_EQ(30, CXFA_TimeZoneProvider().GetTimeZoneInMinutes());
  }
  {
    ScopedSetTZ scoped_set_tz("UTC+1:30");
    EXPECT_EQ(-90, CXFA_TimeZoneProvider().GetTimeZoneInMinutes());
  }
  {
    ScopedSetTZ scoped_set_tz("UTC-1:30");
    EXPECT_EQ(90, CXFA_TimeZoneProvider().GetTimeZoneInMinutes());
  }
  {
    ScopedSetTZ scoped_set_tz("UTC+9:30");
    EXPECT_EQ(-570, CXFA_TimeZoneProvider().GetTimeZoneInMinutes());
  }
  {
    ScopedSetTZ scoped_set_tz("UTC-9:30");
    EXPECT_EQ(570, CXFA_TimeZoneProvider().GetTimeZoneInMinutes());
  }
}
